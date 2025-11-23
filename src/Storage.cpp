/**
 * @file Storage.cpp
 * @brief Implémentation du stockage JSON sur carte SD
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
 * Gère l'enregistrement des données GPS sur carte SD au format JSON.
 * Les fichiers sont compatibles avec le système de replay du Display.
 * Chaque ligne du fichier contient un objet JSON avec timestamp,
 * type de message, et données du bateau.
 * 
 * Format JSON:
 * {"timestamp":1234567890,"type":1,"name":"AA:BB:CC:DD:EE:FF",
 *  "boatId":255,"sequenceNumber":42,"gpsTimestamp":1234567890,
 *  "latitude":43.123456,"longitude":2.654321,"speed":4.5,
 *  "heading":285.0,"satellites":8}
 * 
 * Rotation automatique des fichiers:
 * - Nouveau fichier toutes les MAX_RECORDS (1000 par défaut)
 * - Ou toutes les MAX_FILE_SIZE octets (1 MB par défaut)
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

Storage::Storage() 
    : sdAvailable(false), fileCreated(false), currentFileSize(0), recordCount(0) {
}

/**
 * @brief Initialise le stockage sur carte SD
 * @param enableSD Active l'écriture sur SD (false = désactivé)
 * @return true si initialisé avec succès ou SD désactivé
 * 
 * @details
 * Configure le SPI et monte la carte SD si enableSD=true.
 * Si la carte SD n'est pas disponible, retourne true quand même
 * (le système continue sans enregistrement).
 * 
 * Configuration SPI pour Atom GPS Base:
 * - SCK  : GPIO23
 * - MISO : GPIO33
 * - MOSI : GPIO19
 * - CS   : GPIO5
 * - Fréquence : 40 MHz
 * 
 * Note: L'AtomS3 Lite n'a pas de slot SD, enableSD doit être false.
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

bool Storage::isAvailable() {
    return sdAvailable;
}

String Storage::getCurrentFileName() {
    if (!fileCreated) {
        return "Waiting for GPS fix...";
    }
    return currentFileName;
}

void Storage::closeFile() {
    if (logFile) {
        logFile.close();
        Logger::info("✓ Storage file closed: " + currentFileName + 
                    " (" + String(recordCount) + " records, " + 
                    String(currentFileSize) + " bytes)");
    }
}

bool Storage::needsRotation() {
    return (currentFileSize >= MAX_FILE_SIZE) || (recordCount >= MAX_RECORDS_PER_FILE);
}

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
