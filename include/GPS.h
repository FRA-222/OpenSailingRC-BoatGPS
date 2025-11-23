/**
 * @file GPS.h
 * @brief Acquisition et validation des données GPS
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
 * Cette classe gère la communication avec le module GPS, le parsing
 * des trames NMEA via TinyGPS++, et la validation des données pour
 * le tracker GPS embarqué sur voilier RC.
 * 
 * Fonctionnalités:
 * - Support multi-modules (AT6668 / NEO-6M)
 * - Parsing NMEA automatique
 * - Validation du fix (≥4 satellites)
 * - Conversion en timestamp Unix
 */

#ifndef GPS_H
#define GPS_H

#include <Arduino.h>
#include <TinyGPSPlus.h>

/**
 * @brief GPS data structure
 */
struct GPSData {
    double latitude;         ///< Latitude in degrees
    double longitude;        ///< Longitude in degrees
    float speed;             ///< Speed in knots
    float course;            ///< Course in degrees
    uint32_t timestamp;      ///< Timestamp in milliseconds
    uint8_t satellites;      ///< Number of satellites
    bool valid;              ///< Data validity flag
    uint32_t age;            ///< Age of fix in milliseconds
    uint16_t year;           ///< GPS year
    uint8_t month;           ///< GPS month (1-12)
    uint8_t day;             ///< GPS day (1-31)
    uint8_t hour;            ///< GPS hour (0-23)
    uint8_t minute;          ///< GPS minute (0-59)
    uint8_t second;          ///< GPS second (0-59)
};

/**
 * @brief GPS manager class
 */
class GPS {
public:
    /**
     * @brief Constructor
     * @param rxPin GPIO pin for RX (default: GPIO1)
     * @param txPin GPIO pin for TX (default: GPIO2)
     */
    GPS(uint8_t rxPin = 1, uint8_t txPin = 2);

    /**
     * @brief Initialize GPS module
     * @return true if initialization succeeds
     */
    bool begin();

    /**
     * @brief Update GPS data (call frequently in loop)
     */
    void update();

    /**
     * @brief Get current GPS data
     * @return GPSData structure
     */
    GPSData getData();

    /**
     * @brief Check if GPS has valid fix
     * @return true if GPS fix is valid
     */
    bool isValid();

    /**
     * @brief Get number of satellites in view
     * @return Number of satellites
     */
    uint8_t getSatellites();

    /**
     * @brief Get HDOP (Horizontal Dilution of Precision)
     * @return HDOP value (lower is better)
     */
    float getHDOP();

private:
    TinyGPSPlus gps;
    HardwareSerial* gpsSerial;
    uint8_t rxPin;
    uint8_t txPin;
    GPSData currentData;
    
    // Baudrate depends on GPS module:
    // - Original GPS (NEO-6M): 9600 bps
    // - GPS Atom v2 (AT6668): 115200 bps
    #ifdef CONFIG_IDF_TARGET_ESP32S3
        static const uint32_t GPS_BAUD = 115200;  // AT6668 on AtomS3
    #else
        static const uint32_t GPS_BAUD = 9600;    // NEO-6M on Atom Lite
    #endif
    
    static const uint32_t MAX_AGE_MS = 2000;  ///< Maximum age for valid fix
};

#endif // GPS_H
