/**
 * @file GPS.h
 * @brief GPS data acquisition and validation
 * @author OpenSailingRC
 * @date 2025
 * 
 * This class handles GPS module communication, NMEA parsing,
 * and data validation for the boat GPS tracker.
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
    
    static const uint32_t GPS_BAUD = 9600;
    static const uint32_t MAX_AGE_MS = 2000;  ///< Maximum age for valid fix
};

#endif // GPS_H
