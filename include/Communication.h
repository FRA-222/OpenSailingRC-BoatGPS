/**
 * @file Communication.h
 * @brief ESP-NOW broadcast communication for GPS data
 * @author OpenSailingRC
 * @date 2025
 * 
 * This class handles ESP-NOW broadcast communication to transmit
 * GPS data to all listening devices. Each boat is identified by its MAC address.
 */

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "GPS.h"

/**
 * @brief GPS broadcast packet structure (aligned with Display struct_message_Boat)
 */
struct GPSBroadcastPacket {
    int8_t messageType;      ///< 1 = Boat, 2 = Anemometer
     char name[18];     // MAC address as string (format: "AA:BB:CC:DD:EE:FF")
    int boatId;          // Numeric boat ID
    uint32_t gpsTimestamp;   ///< GPS timestamp in milliseconds
    float latitude;          ///< Latitude in degrees
    float longitude;         ///< Longitude in degrees
    float speed;             ///< Speed in knots
    float heading;           ///< Heading in degrees (0=N, 90=E, 180=S, 270=W)
    uint8_t satellites;      ///< Number of visible satellites
};



/**
 * @brief Communication manager class
 */
class Communication {
public:
    /**
     * @brief Constructor
     */
    Communication();

    /**
     * @brief Initialize ESP-NOW communication
     * @return true if initialization succeeds
     */
    bool begin();

    /**
     * @brief Broadcast GPS data
     * @param data GPS data to broadcast
     * @return true if broadcast succeeds
     */
    bool broadcastGPSData(const GPSData& data);

    /**
     * @brief Get local MAC address
     * @param mac Output buffer for MAC address (6 bytes)
     */
    void getLocalMAC(uint8_t* mac);

private:
    uint8_t localMAC[6];
    
    static Communication* instance;  ///< Singleton instance for callbacks
    
    /**
     * @brief ESP-NOW send callback
     */
    static void onDataSent(const uint8_t* mac, esp_now_send_status_t status);
    
    /**
     * @brief Handle send callback
     */
    void handleSendCallback(esp_now_send_status_t status);
};

#endif // COMMUNICATION_H
