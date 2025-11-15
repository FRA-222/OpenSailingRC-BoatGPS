/**
 * @file Logger.cpp
 * @brief Application message logging implementation
 * @author OpenSailingRC
 * @date 2025
 */

#include "Logger.h"
#include "GPS.h"

bool Logger::begin() {
    Serial.println("✓ Logger: Initialized");
    return true;
}

void Logger::info(const String& message) {
    Serial.println("[INFO] " + message);
}

void Logger::warning(const String& message) {
    Serial.println("[WARN] " + message);
}

void Logger::error(const String& message) {
    Serial.println("[ERROR] " + message);
}

void Logger::logGPSData(const GPSData& data, const uint8_t* macAddress) {
    // Log to serial
    Serial.printf("[%lu] GPS: %.6f,%.6f | %.1fkts %d° | %d sats | MAC: ",
                 data.timestamp,
                 data.latitude,
                 data.longitude,
                 data.speed,
                 (int)data.course,
                 data.satellites);
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", macAddress[i]);
        if (i < 5) Serial.print(":");
    }
    Serial.println();
}
