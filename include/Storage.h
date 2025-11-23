/**
 * @file Storage.h
 * @brief Stockage JSON sur carte SD pour données GPS
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
 * Gère l'enregistrement persistant des données GPS sur carte SD.
 * Les fichiers JSON générés sont compatibles avec le système de
 * replay du Display pour analyse post-navigation.
 * 
 * Configuration SD:
 * - Format: FAT32 recommandé
 * - Pins: SCK=23, MISO=33, MOSI=19, CS=5 (Atom GPS Base)
 * - Vitesse SPI: 40 MHz
 * 
 * Fonctionnalités:
 * - Écriture JSON ligne par ligne (streaming)
 * - Rotation automatique des fichiers
 * - Numérotation séquentielle (gps_001.json, gps_002.json...)
 * - Gestion gracieuse des erreurs (continue sans SD si absent)
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
     * @param sequenceNumber Sequence number for packet loss detection (default: 0)
     */
    void writeGPSData(const GPSData& data, const uint8_t* macAddress, uint32_t sequenceNumber = 0);
    
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
