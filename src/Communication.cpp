/**
 * @file Communication.cpp
 * @brief ESP-NOW broadcast communication implementation
 * @author OpenSailingRC
 * @date 2025
 */

#include "Communication.h"

// Static member initialization
Communication* Communication::instance = nullptr;

Communication::Communication() {
    instance = this;
    memset(localMAC, 0, sizeof(localMAC));
}

bool Communication::begin() {
    // Configure WiFi in Station mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    
    // Set maximum TX power for best range
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    
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

bool Communication::broadcastGPSData(const GPSData& data) {
    // Prepare broadcast packet
    GPSBroadcastPacket packet;
    packet.messageType = 1;           // 1 = Boat GPS data
    
    // Format MAC address as string
    snprintf(packet.name, sizeof(packet.name), "%02X:%02X:%02X:%02X:%02X:%02X",
             localMAC[0], localMAC[1], localMAC[2],
             localMAC[3], localMAC[4], localMAC[5]);
    packet.boatId = (localMAC[5]);   // Simple boat ID from last byte of MAC
    packet.gpsTimestamp = data.timestamp;
    packet.latitude = data.latitude;
    packet.longitude = data.longitude;
    packet.speed = data.speed;
    packet.heading = data.course;
    packet.satellites = data.satellites;
    
    // Broadcast address
    uint8_t broadcastAddr[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    
    // Send via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddr, (uint8_t*)&packet, sizeof(packet));
    
    if (result == ESP_OK) {
        Serial.printf("→ Broadcast: %.6f,%.6f (%.1fkts, %d°, %d sats)\n",
                     packet.latitude,
                     packet.longitude,
                     packet.speed,
                     (int)packet.heading,
                     packet.satellites);
        return true;
    } else {
        Serial.printf("✗ Broadcast failed (error: %d)\n", result);
        return false;
    }
}

void Communication::getLocalMAC(uint8_t* mac) {
    memcpy(mac, localMAC, 6);
}

void Communication::onDataSent(const uint8_t* mac, esp_now_send_status_t status) {
    if (instance) {
        instance->handleSendCallback(status);
    }
}

void Communication::handleSendCallback(esp_now_send_status_t status) {
    // Optional: handle send status
    if (status != ESP_NOW_SEND_SUCCESS) {
        Serial.println("⚠️  ESP-NOW: Send callback reported failure");
    }
}
