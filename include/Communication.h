/**
 * @file Communication.h
 * @brief Communication ESP-NOW en mode broadcast pour données GPS
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
 * Cette classe gère la communication ESP-NOW en mode broadcast pour
 * transmettre les données GPS à tous les appareils à l'écoute.
 * Chaque bateau est identifié par son adresse MAC unique.
 * 
 * Structure des paquets:
 * - Alignée avec struct_message_Boat du Display
 * - Contient position, vitesse, cap, nombre de satellites
 * - Timestamp rempli par le Display à la réception
 */

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include "GPS.h"

/**
 * @brief GPS broadcast packet structure (aligned with Display struct_message_Boat)
 */
struct GPSBroadcastPacket {
    int8_t messageType;      ///< 1 = Boat, 2 = Anemometer
     char name[18];     // Custom boat name or MAC address (max 17 chars + null terminator)
    uint32_t sequenceNumber; ///< Sequence number (incremental counter for packet loss detection)
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
     * @brief Broadcast GPS data with automatic retry
     * 
     * IMPORTANT: En mode broadcast (FF:FF:FF:FF:FF:FF), ESP-NOW ne fournit PAS
     * d'acquittement (ACK) de la part des récepteurs. Le callback onDataSent()
     * indique uniquement si le paquet a été transmis par la couche radio,
     * PAS s'il a été reçu par un ou plusieurs destinataires.
     * 
     * Cette fonction implémente des retries automatiques pour augmenter les
     * chances de réception, mais ne peut pas garantir qu'un récepteur spécifique
     * a reçu le message.
     * 
     * Pour avoir un vrai feedback de réception, il faudrait :
     * - Option A : Passer en mode unicast (envoyer à chaque bouée individuellement)
     * - Option B : Implémenter un protocole d'ACK applicatif (les bouées renvoient
     *              un petit paquet de confirmation)
     * 
     * @param data GPS data to broadcast
     * @param boatName Custom boat name or MAC address
     * @param retries Number of retry attempts if send fails (default: 2)
     * @return true if at least one broadcast attempt succeeded (transmission layer only)
     */
    bool broadcastGPSData(const GPSData& data, const String& boatName, uint8_t retries = 2);

    /**
     * @brief Get local MAC address
     * @param mac Output buffer for MAC address (6 bytes)
     */
    void getLocalMAC(uint8_t* mac);
    
    /**
     * @brief Get current sequence number
     * @return Current sequence counter value
     */
    uint32_t getSequenceNumber() const;

private:
    uint8_t localMAC[6];
    uint32_t sequenceCounter;        ///< Sequence counter for packet numbering
    
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
