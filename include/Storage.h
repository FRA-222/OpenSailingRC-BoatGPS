/**
 * @file Storage.h
 * @brief SD card JSON storage for GPS data
 * @author OpenSailingRC
 * @date 2025
 */

#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include <SD.h>
#include <ArduinoJson.h>
#include "GPS.h"

/**
 * @class Storage
 * @brief Manages JSON file storage on SD card
 * 
 * Stores GPS data in JSON format compatible with OpenSailingRC-Display replay files.
 * Each line in the file contains one JSON object with timestamp, type, and boat data.
 * Files are automatically rotated based on size and record count.
 */
class Storage {
public:
    /**
     * @brief Constructor
     */
    Storage();
    
    /**
     * @brief Initialize SD card storage
     * @param enableSD Enable SD card writing (false = disabled)
     * @return true if initialized successfully or SD disabled
     */
    bool begin(bool enableSD = true);
    
    /**
     * @brief Write GPS data to SD card in JSON format
     * @param data GPS data structure
     * @param macAddress MAC address of the GPS device (6 bytes)
     */
    void writeGPSData(const GPSData& data, const uint8_t* macAddress);
    
    /**
     * @brief Check if SD card is available
     * @return true if SD card is mounted and working
     */
    bool isAvailable();
    
    /**
     * @brief Get current log file name
     * @return Current file path (e.g., "/gps_001.json")
     */
    String getCurrentFileName();
    
    /**
     * @brief Close current log file
     */
    void closeFile();
    
private:
    File logFile;                                             ///< Current log file handle
    bool sdAvailable;                                         ///< SD card availability flag
    bool fileCreated;                                         ///< File created flag (waits for first valid GPS)
    uint32_t currentFileSize;                                 ///< Current file size in bytes
    uint32_t recordCount;                                     ///< Number of records in current file
    String currentFileName;                                   ///< Current file name
    String macAddressStr;                                     ///< MAC address string for filename
    
    static const uint32_t MAX_FILE_SIZE = 10 * 1024 * 1024;  ///< 10 MB max file size
    static const uint32_t MAX_RECORDS_PER_FILE = 10000;      ///< Max records per file
    static const char* FILE_PREFIX;                          ///< File name prefix ("/gps_")
    static const char* FILE_EXTENSION;                       ///< File extension (".json")
    static const uint8_t MESSAGE_TYPE = 1;                   ///< Type 1 = Boat data
    
    /**
     * @brief Create and open log file with MAC and timestamp
     * @param macAddress MAC address of device
     * @param data GPS data containing date/time information
     * @return true if file created successfully
     */
    bool createLogFile(const uint8_t* macAddress, const GPSData& data);
    
    /**
     * @brief Check if file rotation is needed
     * @return true if max size or max records reached
     */
    bool needsRotation();
    
    /**
     * @brief Rotate to new log file with new timestamp
     * @param data GPS data for new file timestamp
     */
    void rotateFile(const GPSData& data);
};

#endif // STORAGE_H
