/**
 * @file GPS.cpp
 * @brief GPS data acquisition implementation
 * @author OpenSailingRC
 * @date 2025
 */

#include "GPS.h"

GPS::GPS(uint8_t rxPin, uint8_t txPin)
    : rxPin(rxPin), txPin(txPin), gpsSerial(nullptr) {
    currentData.valid = false;
    currentData.timestamp = 0;
}

bool GPS::begin() {
    // Initialize Serial2 for GPS communication
    gpsSerial = &Serial2;
    gpsSerial->begin(GPS_BAUD, SERIAL_8N1, rxPin, txPin);
    
    Serial.println("✓ GPS: Initialized");
    Serial.printf("  RX: GPIO%d, TX: GPIO%d, Baud: %d\n", rxPin, txPin, GPS_BAUD);
    Serial.println("  Waiting for GPS data...");
    
    return true;
}

void GPS::update() {
    // Feed GPS parser with available data
    while (gpsSerial->available() > 0) {
        char c = gpsSerial->read();
        gps.encode(c);
        
        // Debug: Print raw NMEA sentences (uncomment for troubleshooting)
        // Serial.print(c);
    }
    
    // Update current data if location is updated
    if (gps.location.isUpdated()) {
        currentData.latitude = gps.location.lat();
        currentData.longitude = gps.location.lng();
        currentData.speed = gps.speed.knots();
        currentData.course = gps.course.deg();
        currentData.satellites = gps.satellites.value();
        currentData.age = gps.location.age();
        
        // Récupérer l'heure et la date du GPS
        int year = gps.date.year();
        int month = gps.date.month();
        int day = gps.date.day();
        int hour = gps.time.hour();
        int minute = gps.time.minute();
        int second = gps.time.second();
    
        // Convert GPS date and time to epoch time
        struct tm gpsTimeinfo;
        gpsTimeinfo.tm_year = year - 1900; // tm_year is years since 1900
        gpsTimeinfo.tm_mon = month - 1;   // tm_mon is 0-11
        gpsTimeinfo.tm_mday = day;
        gpsTimeinfo.tm_hour = hour;
        gpsTimeinfo.tm_min = minute;
        gpsTimeinfo.tm_sec = second;
        gpsTimeinfo.tm_isdst = -1; // Not considering daylight saving time
        
        currentData.timestamp = mktime(&gpsTimeinfo);

        //Serial.println("currentData.timestamp: " + String(currentData.timestamp));

        // Update date/time from GPS
        if (gps.date.isValid()) {
            currentData.year = gps.date.year();
            currentData.month = gps.date.month();
            currentData.day = gps.date.day();
        }
        if (gps.time.isValid()) {
            currentData.hour = gps.time.hour();
            currentData.minute = gps.time.minute();
            currentData.second = gps.time.second();
        }
        
        // Validate data
        currentData.valid = gps.location.isValid() && 
                           (currentData.satellites >= 4);
    }
}

GPSData GPS::getData() {
    return currentData;
}

bool GPS::isValid() {
    // Check if data is valid
    return currentData.valid; 
}

uint8_t GPS::getSatellites() {
    return gps.satellites.value();
}

float GPS::getHDOP() {
    return gps.hdop.hdop();
}
