#include <Arduino.h>

#include "boards.h"
#include <SparkFun_Ublox_Arduino_Library.h>
#include <MicroNMEA.h> //https://github.com/stevemarple/MicroNMEA
#include <RadioLib.h>

SFE_UBLOX_GPS myGPS;

char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

#ifdef RADIO_USING_SX1262
RADIO_TYPE      radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);
#else
RADIO_TYPE      radio = new Module(RADIO_CS_PIN, RADIO_DIO0_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);
#endif

// flag to indicate that a packet was received
bool            receivedFlag = false;
// disable interrupt when it's not needed
bool            enableInterrupt = true;

// this function is called when a complete packet
// is received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
void setFlag(void)
{
    // check if the interrupt is enabled
    if (!enableInterrupt) {
        return;
    }
    // we got a packet, set the flag
    receivedFlag = true;
}

char            buff[5][256];

long lastTime = 0; //Simple local timer. Limits amount if I2C traffic to Ublox module.

void setup() 
{
    initBoard();
    // When the power is turned on, a delay is required.
    delay(1500);

    Serial.println("Tom's GPS Tracker");

    // GPS
    if (myGPS.begin(Serial1) == false) {
        Serial.println(F("Ublox GPS not detected . Please check wiring. Freezing."));
        while (1);
    }

    myGPS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
    myGPS.saveConfiguration(); //Save the current settings to flash and BBR

    // LoRa
    Serial.print(F("[Radio] Initializing ... "));
#ifndef LoRa_frequency
    int state = radio.begin(868.0);
#else
    int state = radio.begin(LoRa_frequency);
#endif

    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("success!"));
    } else {
        Serial.print(F("failed, code "));
        Serial.println(state);
        while (true);
    }

    // set the function that will be called
    // when new packet is received
#if defined(RADIO_USING_SX1262)
    radio.setDio1Action(setFlag);
#elif defined(RADIO_USING_SX1268)
    radio.setDio1Action(setFlag);
#else
    radio.setDio0Action(setFlag, RISING);
#endif


}

void loop()
{
  if (millis() - lastTime > 3000)
    {
    lastTime = millis(); //Update the timer
    
    myGPS.checkUblox(); //See if new data is available. Process bytes as they come in.

    if (nmea.isValid() == true) {
        long latitude_mdeg = nmea.getLatitude();
        long longitude_mdeg = nmea.getLongitude();
        long numSatellites = nmea.getNumSatellites();
        long altitude;
        nmea.getAltitude(altitude);
        long year = nmea.getYear();
        long month = nmea.getMonth();
        long day = nmea.getDay();
        long hour = nmea.getHour();
        long minute = nmea.getMinute();
        long seconds = nmea.getSecond();

        Serial.print("Num. satellites: ");
        Serial.println(numSatellites);
        Serial.print("Latitude (deg): ");
        Serial.println(latitude_mdeg / 1000000., 6);
        Serial.print("Longitude (deg): ");
        Serial.println(longitude_mdeg / 1000000., 6);
        Serial.print("Altitude (m): ");
        Serial.println(altitude / 1000., 1);
        Serial.print("Date/Time: ");
        Serial.print(day);
        Serial.print(".");
        Serial.print(month);
        Serial.print(".");
        Serial.print(year);
        Serial.print(" ");
        Serial.print(hour);
        Serial.print(":");
        Serial.print(minute);
        Serial.print(":");
        Serial.println(year);
        Serial.println("");
    } else {
        Serial.print("No Fix - ");
        Serial.print("Num. satellites: ");
        Serial.println(nmea.getNumSatellites());
    }
  }


}

// This function gets called from the SparkFun Ublox Arduino Library
// As each NMEA character comes in you can specify what to do with it
// Useful for passing to other libraries like tinyGPS, MicroNMEA, or even
// a buffer, radio, etc.
void SFE_UBLOX_GPS::processNMEA(char incoming)
{
    //Take the incoming char from the Ublox I2C port and pass it on to the MicroNMEA lib
    //for sentence cracking
    nmea.process(incoming);
}
