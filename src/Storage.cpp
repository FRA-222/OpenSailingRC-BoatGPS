/**
 * @file Storage.cpp
 * @brief SD card JSON storage implementation
 * @author OpenSailingRC Contributors
 * @date 2025
 * @version 1.0.3
 * 
 * @copyright Copyright (c) 2025 OpenSailingRC
 * @license GNU General Public License v3.0
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 * 
 * @details
 * Manages GPS data recording to SD card in JSON format.
 * Files are compatible with the Display replay system.
 * Each line in the file contains one JSON object with timestamp,
 * message type, and boat data.
 * 
 * JSON format:
 * {"timestamp":1234567890,"type":1,"name":"AA:BB:CC:DD:EE:FF",
 *  "sequenceNumber":42,"gpsTimestamp":1234567890,
 *  "latitude":43.123456,"longitude":2.654321,"speed":4.5,
 *  "heading":285.0,"satellites":8}
 * 
 * Automatic file rotation:
 * - New file every MAX_RECORDS (1000 by default)
 * - Or every MAX_FILE_SIZE bytes (1 MB by default)
 */

#include "Storage.h"
#include "Logger.h"

// SD card pins for Atom Lite + GPS Base
#define SPI_SCK  23
#define SPI_MISO 33
#define SPI_MOSI 19
#define SPI_CS   5  // CS pin for Atom Lite

// Static constants
const char* Storage::FILE_PREFIX = "/gps_";
const char* Storage::FILE_EXTENSION = ".json";

/**
 * @brief Constructor for Storage class
 * 
 * @details
 * Initializes all member variables to their default state.
 * SD card must be initialized by calling begin() before use.
 */
Storage::Storage() 
    : sdAvailable(false), fileCreated(false), currentFileSize(0), recordCount(0) {
}

/**
 * @brief Initialize SD card storage
 * @param enableSD Enable SD card writing (false = disabled)
 * @return true if successfully initialized or SD disabled
 * 
 * @details
 * Configures SPI and mounts SD card if enableSD=true.
 * If SD card is not available, returns true anyway
 * (system continues without recording).
 * 
 * SPI configuration for Atom GPS Base:
 * - SCK  : GPIO23
 * - MISO : GPIO33
 * - MOSI : GPIO19
 * - CS   : GPIO5
 * - Frequency : 40 MHz
 * 
 * Note: AtomS3 Lite has no SD slot, enableSD must be false.
 */
bool Storage::begin(bool enableSD) {
    Logger::info("Storage: Initializing...");
    
    if (!enableSD) {
        Logger::info("  SD card storage disabled");
        return true;
    }
    
    // M5Stack Atom GPS Base has built-in SD card slot
    // Different pin configurations exist depending on Base version
    // Standard Atom GPS Base: SCK=22, MISO=23, MOSI=19, CS=33
    Logger::info("  Initializing Atom GPS SD card...");
    
    // Initialize SPI with Atom GPS Base standard pins
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS);
    if (!SD.begin(SPI_CS, SPI, 40000000)) 
    {
        Logger::warning("SD card not available - storage disabled");
        Logger::info("  Check if SD card is properly inserted");
        Logger::info("  Format SD card as FAT32 if needed");
        sdAvailable = false;
        return true;  // Not a failure, just no SD
    }
    else
      Logger::info("SD card initialized OK");

    
    sdAvailable = true;
    Logger::info("✓ SD card mounted");
    Logger::info("  Waiting for first valid GPS fix to create log file...");
    
    return true;
}

/**
 * @brief Write GPS data to SD card in JSON format
 * @param data GPS data structure to record
 * @param macAddress Device MAC address (6 bytes)
 * @param sequenceNumber Packet sequence number for tracking
 * 
 * @details
 * Records GPS data as one JSON object per line.
 * Creates log file on first valid GPS fix.
 * Automatically rotates files when MAX_FILE_SIZE or MAX_RECORDS_PER_FILE is reached.
 * 
 * Filename format: /gps_MACADDRESS_YYYY-MM-DD_HH-MM-SS.json
 * Example: /gps_D0CF130FD9DC_2025-11-25_14-30-00.json
 */
void Storage::writeGPSData(const GPSData& data, const uint8_t* macAddress, uint32_t sequenceNumber) {
    if (!sdAvailable) {
        return;
    }
    
    // Create file on first valid GPS fix
    if (!fileCreated && data.valid) {
        if (createLogFile(macAddress, data)) {
            fileCreated = true;
            Logger::info("✓ Log file created: " + currentFileName);
        } else {
            return;
        }
    }
    
    // Skip if file not created yet (waiting for valid GPS)
    if (!fileCreated || !logFile) {
        return;
    }
    
    // Check if rotation needed
    if (needsRotation()) {
        rotateFile(data);
    }
    
    // Create JSON document
    JsonDocument doc;
    doc["timestamp"] = data.timestamp;
    doc["type"] = MESSAGE_TYPE;
    
    // Boat data nested object
    JsonObject boat = doc["boat"].to<JsonObject>();
    boat["messageType"] = MESSAGE_TYPE;
    boat["sequenceNumber"] = sequenceNumber;  // Add sequence number for packet loss tracking
    boat["gpsTimestamp"] = data.timestamp;
    boat["latitude"] = data.latitude;
    boat["longitude"] = data.longitude;
    boat["speed"] = data.speed;
    boat["heading"] = data.course;
    boat["satellites"] = data.satellites;
    
    // Serialize to file (one JSON object per line)
    serializeJson(doc, logFile);
    logFile.println();
    logFile.flush();  // Ensure data is written
    
    currentFileSize = logFile.size();
    recordCount++;
}

/**
 * @brief Check if SD card storage is available
 * @return true if SD card is mounted and working
 */
bool Storage::isAvailable() {
    return sdAvailable;
}

/**
 * @brief Get current log filename
 * @return Current filename or status message if file not yet created
 */
String Storage::getCurrentFileName() {
    if (!fileCreated) {
        return "Waiting for GPS fix...";
    }
    return currentFileName;
}

/**
 * @brief Close current log file
 * 
 * @details
 * Safely closes the current log file and logs statistics
 * (record count and file size).
 */
void Storage::closeFile() {
    if (logFile) {
        logFile.close();
        Logger::info("✓ Storage file closed: " + currentFileName + 
                    " (" + String(recordCount) + " records, " + 
                    String(currentFileSize) + " bytes)");
    }
}

/**
 * @brief Check if file rotation is needed
 * @return true if file size or record count exceeds limits
 * 
 * @details
 * File rotation triggers when either:
 * - File size >= MAX_FILE_SIZE (1 MB)
 * - Record count >= MAX_RECORDS_PER_FILE (1000)
 */
bool Storage::needsRotation() {
    return (currentFileSize >= MAX_FILE_SIZE) || (recordCount >= MAX_RECORDS_PER_FILE);
}

/**
 * @brief Create new log file with timestamp-based filename
 * @param macAddress Device MAC address for filename
 * @param data GPS data containing timestamp for filename
 * @return true if file created successfully
 * 
 * @details
 * Generates filename from MAC address and GPS timestamp.
 * Format: /gps_MACADDRESS_YYYY-MM-DD_HH-MM-SS.json
 * If file exists, appends suffix (_1, _2, etc.)
 * 
 * Closes any previously open file before creating new one.
 */
bool Storage::createLogFile(const uint8_t* macAddress, const GPSData& data) {
    // Close previous file if open
    if (logFile) {
        closeFile();
    }
    
    // Convert MAC address to string (without colons): D0CF130FD9DC
    char macStr[13];
    snprintf(macStr, sizeof(macStr), "%02X%02X%02X%02X%02X%02X",
             macAddress[0], macAddress[1], macAddress[2],
             macAddress[3], macAddress[4], macAddress[5]);
    macAddressStr = String(macStr);
    
    // Generate filename: /gps_MACADDRESS_YYYY-MM-DD_HH-MM-SS.json
    char filename[80];
    snprintf(filename, sizeof(filename), "%s%s_%04d-%02d-%02d_%02d-%02d-%02d%s", 
             FILE_PREFIX, macStr, 
             data.year, data.month, data.day,
             data.hour, data.minute, data.second,
             FILE_EXTENSION);
    currentFileName = String(filename);
    
    // Check if file already exists (shouldn't happen but safety check)
    int suffix = 1;
    while (SD.exists(filename) && suffix < 100) {
        snprintf(filename, sizeof(filename), "%s%s_%04d-%02d-%02d_%02d-%02d-%02d_%d%s", 
                 FILE_PREFIX, macStr,
                 data.year, data.month, data.day,
                 data.hour, data.minute, data.second,
                 suffix, FILE_EXTENSION);
        currentFileName = String(filename);
        suffix++;
    }
    
    // Open file for writing
    logFile = SD.open(filename, FILE_WRITE);
    if (!logFile) {
        Logger::error("Failed to create: " + String(filename));
        return false;
    }
    
    currentFileSize = 0;
    recordCount = 0;
    
    return true;
}

/**
 * @brief Rotate to new log file
 * @param data GPS data for new filename timestamp
 * 
 * @details
 * Closes current file and creates new one with updated timestamp.
 * Called automatically when file size or record count limits are reached.
 * Preserves MAC address from previous file.
 */
void Storage::rotateFile(const GPSData& data) {
    Logger::info("🔄 Rotating storage file...");
    closeFile();
    
    // Create new file with new timestamp
    uint8_t dummyMac[6];
    // Extract MAC from macAddressStr
    for (int i = 0; i < 6; i++) {
        char hexByte[3] = {macAddressStr[i*2], macAddressStr[i*2+1], '\0'};
        dummyMac[i] = (uint8_t)strtol(hexByte, NULL, 16);
    }
    
    createLogFile(dummyMac, data);
    Logger::info("✓ New log file: " + currentFileName);
}
