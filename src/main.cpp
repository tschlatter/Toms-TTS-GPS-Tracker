#include <Arduino.h>

#include "boards.h"
#include "SparkFun_Ublox_Arduino_Library.h"

SFE_UBLOX_GPS myGPS;

#include <MicroNMEA.h> //https://github.com/stevemarple/MicroNMEA

char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

long lastTime = 0; //Simple local timer. Limits amount if I2C traffic to Ublox module.

void setup() 
{
    initBoard();
    // When the power is turned on, a delay is required.
    delay(1500);

    Serial.println("Tom's GPS Tracker");

    if (myGPS.begin(Serial1) == false) {
        Serial.println(F("Ublox GPS not detected . Please check wiring. Freezing."));
        while (1);
    }

    myGPS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
    myGPS.saveConfiguration(); //Save the current settings to flash and BBR

}

void loop()
{
  if (millis() - lastTime > 1000)
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
