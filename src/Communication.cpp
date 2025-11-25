/**
 * @file Communication.cpp
 * @brief Implémentation de la communication ESP-NOW en mode broadcast
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
 * Gère la diffusion des données GPS via ESP-NOW en mode broadcast.
 * Chaque bateau est identifié par son adresse MAC unique.
 * 
 * Caractéristiques:
 * - Broadcast vers FF:FF:FF:FF:FF:FF (tous les appareils)
 * - Retry automatique en cas d'échec de transmission
 * - Numéro de séquence pour détection de perte de paquets
 * - Puissance TX maximale (21 dBm) pour portée optimale
 */

#include "Communication.h"

// Static member initialization
Communication* Communication::instance = nullptr;

/**
 * @brief Constructeur de la classe Communication
 * 
 * @details
 * Initialise le compteur de séquence à 0 et prépare
 * le buffer MAC. Le pointeur statique d'instance est
 * utilisé pour le callback ESP-NOW.
 */
Communication::Communication() : sequenceCounter(0) {
    instance = this;
    memset(localMAC, 0, sizeof(localMAC));
}

/**
 * @brief Initialise la communication ESP-NOW
 * @return true si l'initialisation réussit, false sinon
 * 
 * @details
 * Configuration:
 * - WiFi en mode Station (sans connexion)
 * - Puissance TX maximale (84 = 21 dBm)
 * - Canal WiFi 1 (modifiable pour éviter interférences)
 * - Adresse broadcast FF:FF:FF:FF:FF:FF
 * - Callback d'envoi enregistré
 * 
 * Note: En mode broadcast, ESP-NOW ne fournit PAS d'ACK
 * des récepteurs. Le callback indique uniquement si le
 * paquet a été transmis par la couche radio.
 */
bool Communication::begin() {
    // Configure WiFi in Station mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    
    // Set maximum TX power for best range (84 = 21 dBm = maximum power)
    esp_wifi_set_max_tx_power(84);
    
    // Set WiFi to channel 1 for consistency (can be changed to avoid interference)
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
    
    // Get local MAC address
    WiFi.macAddress(localMAC);
    Serial.print("✓ ESP-NOW: MAC Address: ");
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", localMAC[i]);
        if (i < 5) Serial.print(":");
    }
    Serial.println();
    
    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("✗ ESP-NOW: Initialization failed");
        return false;
    }
    
    Serial.println("✓ ESP-NOW: Initialized in broadcast mode");
    
    // Register send callback
    esp_now_register_send_cb(onDataSent);
    
    // Add broadcast peer
    esp_now_peer_info_t peerInfo = {};
    memset(&peerInfo, 0, sizeof(peerInfo));
    memset(peerInfo.peer_addr, 0xFF, 6);  // Broadcast address
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("✗ ESP-NOW: Failed to add broadcast peer");
        return false;
    }
    
    Serial.println("✓ ESP-NOW: Broadcast peer added");
    
    return true;
}

/**
 * @brief Diffuse les données GPS via ESP-NOW avec retry automatique
 * @param data Structure GPSData à diffuser
 * @param retries Nombre de tentatives supplémentaires en cas d'échec (défaut: 2)
 * @return true si au moins une tentative a réussi (couche transmission uniquement)
 * 
 * @details
 * Le paquet est diffusé vers l'adresse broadcast (FF:FF:FF:FF:FF:FF).
 * En cas d'échec de transmission, effectue jusqu'à 'retries' tentatives
 * supplémentaires avec un délai de 10ms entre chaque.
 * 
 * IMPORTANT: Le succès indique uniquement que le paquet a été transmis
 * par la couche radio, pas qu'il a été reçu par un Display spécifique.
 * 
 * Structure du paquet (alignée avec struct_message_Boat du Display):
 * - messageType : 1 (identifie les données bateau)
 * - name : Nom personnalisé du bateau ou adresse MAC ("BOAT1" ou "AA:BB:CC:DD:EE:FF")
 * - sequenceNumber : Compteur incrémental (détection perte)
 * - gpsTimestamp : Timestamp GPS (epoch Unix)
 * - latitude, longitude : Position en degrés
 * - speed : Vitesse en nœuds
 * - heading : Cap en degrés (0=Nord)
 * - satellites : Nombre de satellites
 */
bool Communication::broadcastGPSData(const GPSData& data, const String& boatName, uint8_t retries) {
    // Increment sequence counter
    sequenceCounter++;
    
    // Prepare broadcast packet
    GPSBroadcastPacket packet;
    packet.messageType = 1;           // 1 = Boat GPS data
    
    // Use custom boat name or MAC address
    strncpy(packet.name, boatName.c_str(), sizeof(packet.name) - 1);
    packet.name[sizeof(packet.name) - 1] = '\0'; // Ensure null termination
    packet.sequenceNumber = sequenceCounter;  // Add sequence number for packet loss detection
    packet.gpsTimestamp = data.timestamp;
    packet.latitude = data.latitude;
    packet.longitude = data.longitude;
    packet.speed = data.speed;
    packet.heading = data.course;
    packet.satellites = data.satellites;
    
    // Broadcast address
    uint8_t broadcastAddr[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    
    // Try to send with retries
    bool success = false;
    uint8_t attempt = 0;
    
    for (attempt = 0; attempt <= retries; attempt++) {
        // Send via ESP-NOW
        esp_err_t result = esp_now_send(broadcastAddr, (uint8_t*)&packet, sizeof(packet));
        
        if (result == ESP_OK) {
            success = true;
            if (attempt > 0) {
                Serial.printf("→ Broadcast #%lu: %.6f,%.6f (%.1fkts, %d°, %d sats) [retry %d]\n",
                             packet.sequenceNumber,
                             packet.latitude,
                             packet.longitude,
                             packet.speed,
                             (int)packet.heading,
                             packet.satellites,
                             attempt);
            } else {
                Serial.printf("→ Broadcast #%lu: %.6f,%.6f (%.1fkts, %d°, %d sats)\n",
                             packet.sequenceNumber,
                             packet.latitude,
                             packet.longitude,
                             packet.speed,
                             (int)packet.heading,
                             packet.satellites);
            }
            break;  // Success, exit loop
        } else {
            Serial.printf("✗ Broadcast attempt %d failed (error: %d)\n", attempt + 1, result);
            if (attempt < retries) {
                delay(10);  // Small delay before retry (10ms)
            }
        }
    }
    
    if (!success) {
        Serial.printf("✗ Broadcast failed after %d attempts\n", attempt);
    }
    
    return success;
}

/**
 * @brief Copie l'adresse MAC locale dans le buffer fourni
 * @param mac Buffer de sortie (6 octets)
 */
void Communication::getLocalMAC(uint8_t* mac) {
    memcpy(mac, localMAC, 6);
}

/**
 * @brief Retourne le numéro de séquence actuel
 * @return Compteur de séquence (nombre de paquets envoyés)
 */
uint32_t Communication::getSequenceNumber() const {
    return sequenceCounter;
}

/**
 * @brief Callback ESP-NOW appelé après tentative d'envoi
 * @param mac Adresse MAC du destinataire
 * @param status Statut de l'envoi (ESP_NOW_SEND_SUCCESS ou ESP_NOW_SEND_FAIL)
 * 
 * @details
 * Fonction statique servant de pont vers la méthode d'instance.
 * Note: En mode broadcast, ce callback indique uniquement si le paquet
 * a été transmis par la couche radio, pas s'il a été reçu.
 */
void Communication::onDataSent(const uint8_t* mac, esp_now_send_status_t status) {
    if (instance) {
        instance->handleSendCallback(status);
    }
}

/**
 * @brief Gère le callback d'envoi ESP-NOW
 * @param status Statut de l'envoi
 * 
 * @details
 * Affiche un avertissement si la transmission a échoué au niveau radio.
 * Cette fonction peut être étendue pour implémenter des statistiques
 * d'envoi ou des actions de récupération.
 */
void Communication::handleSendCallback(esp_now_send_status_t status) {
    // Optional: handle send status
    if (status != ESP_NOW_SEND_SUCCESS) {
        Serial.println("⚠️  ESP-NOW: Send callback reported failure");
    }
}
