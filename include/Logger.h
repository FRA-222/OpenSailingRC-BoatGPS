/**
 * @file Logger.h
 * @brief Application message logging
 * @author OpenSailingRC
 * @date 2025
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

// Forward declaration
struct GPSData;

/**
 * @class Logger
 * @brief Static logging utility for application messages
 * 
 * Provides simple serial output logging for debugging and monitoring.
 * Use Storage class for GPS data persistence to SD card.
 */
class Logger {
public:
    /**
     * @brief Initialize logger
     * @return true if initialization successful
     */
    static bool begin();
    
    /**
     * @brief Log info message
     * @param message Message to log
     */
    static void info(const String& message);
    
    /**
     * @brief Log warning message
     * @param message Message to log
     */
    static void warning(const String& message);
    
    /**
     * @brief Log error message
     * @param message Message to log
     */
    static void error(const String& message);
    
    /**
     * @brief Log GPS data to serial (for debugging)
     * @param data GPS data structure
     * @param macAddress MAC address of the GPS device (6 bytes)
     */
    static void logGPSData(const GPSData& data, const uint8_t* macAddress);
};

#endif // LOGGER_H
