/**
 * @file main.cpp
 * @brief Programme principal pour OpenSailingRC-BoatGPS
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
 * Tracker GPS pour voiliers RC avec diffusion ESP-NOW.
 * Transmet la position GPS, la vitesse et le cap à tous les appareils
 * à l'écoute (Display) et enregistre les données sur carte SD.
 * 
 * Matériel supporté:
 * - M5Stack AtomS3 Lite + GPS Atom v2 (AT6668)
 * - M5Stack Atom Lite + GPS Base (NEO-6M)
 * 
 * Communication: ESP-NOW broadcast (FF:FF:FF:FF:FF:FF)
 * Portée: 100-200m en ligne de vue
 * Fréquence d'émission: 1 Hz (configurable)
 */

#include <M5Unified.h>
#include <FastLED.h>
#include "GPS.h"
#include "Communication.h"
#include "Logger.h"
#include "Storage.h"

// ============================================================================
// CONFIGURATION
// ============================================================================
// Board detection and pin configuration
#ifdef CONFIG_IDF_TARGET_ESP32S3
// AtomS3 Lite + GPS Atom v2 (AT6668) configuration
// Grove connector uses GPIO5 and GPIO6
const uint8_t GPS_RX_PIN = 5;              // GPIO5 for GPS RX (connected to GPS TX)
const uint8_t GPS_TX_PIN = 6;              // GPIO6 for GPS TX (connected to GPS RX)
const uint8_t LED_PIN = 35;                // RGB LED on GPIO35 (AtomS3)
#else
// Atom Lite + GPS Base configuration (ESP32 classic)
const uint8_t GPS_RX_PIN = 22;             // GPIO22 for GPS RX
const uint8_t GPS_TX_PIN = 19;             // GPIO19 for GPS TX
const uint8_t LED_PIN = 27;                // RGB LED on GPIO27 (Atom Lite)
#endif

const uint32_t BROADCAST_INTERVAL = 1000;  // Broadcast every 1 second
const uint32_t STATUS_INTERVAL = 5000;     // Status update every 5 seconds

// SD Storage configuration based on build flags
#ifdef DISABLE_SD_STORAGE
const bool ENABLE_SD_STORAGE = false;      // AtomS3 Lite: No SD card
#else
const bool ENABLE_SD_STORAGE = true;       // Atom GPS Base: SD card available
#endif

// LED Configuration
const uint8_t LED_COUNT = 1;               // One RGB LED
CRGB leds[LED_COUNT];

// ============================================================================
// UBX COMMANDS FOR GPS CONFIGURATION
// ============================================================================
// UBX command to enable GPS + Galileo constellations
const uint8_t ubx_cfg_gnss_gps_galileo[] = {
  0xB5, 0x62, 0x06, 0x3E, 0x28, 0x00,
  0x00, 0x00, 0x20, 0x05,

  // GPS (enabled)
  0x00, 0x00, 0x10, 0x00, 0x01, 0x01, 0x00, 0x01,

  // SBAS (disabled)
  0x01, 0x00, 0x08, 0x00, 0x01, 0x01, 0x00, 0x00,

  // Galileo (enabled)
  0x02, 0x00, 0x10, 0x00, 0x01, 0x01, 0x00, 0x01,

  // BeiDou (disabled)
  0x03, 0x00, 0x08, 0x00, 0x01, 0x01, 0x00, 0x00,

  // GLONASS (disabled)
  0x06, 0x00, 0x10, 0x00, 0x01, 0x01, 0x00, 0x00,

  // Checksum
  0x86, 0xA4
};

/**
 * @brief Send UBX command to GPS module
 * @param msg UBX command buffer
 * @param len Length of the command
 */
void sendUBX(const uint8_t *msg, uint8_t len) {
  for (uint8_t i = 0; i < len; i++) {
    Serial2.write(msg[i]);
  }
}

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================
GPS gps(GPS_RX_PIN, GPS_TX_PIN);
Communication comm;
Storage storage;

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================
uint32_t lastBroadcast = 0;
uint32_t lastStatus = 0;
uint32_t validPacketCount = 0;
uint32_t invalidPacketCount = 0;

// ============================================================================
// LED STATUS INDICATORS
// ============================================================================

/**
 * @brief Définit la couleur de la LED RGB de statut
 * @param color Couleur au format 0xRRGGBB
 * 
 * @details
 * La LED RGB est située sur le bouton de l'AtomS3 Lite (GPIO35).
 * Codes couleur utilisés:
 * - Bleu (0x0000FF)   : Initialisation en cours
 * - Jaune (0xFFFF00)  : Attente du fix GPS
 * - Vert (0x00FF00)   : Données GPS valides, transmission OK
 * - Rouge (0xFF0000)  : Erreur critique
 * - Éteint (0x000000) : Inactif
 */
void setStatusLED(uint32_t color) {
    leds[0] = CRGB(color);
    FastLED.show();
}

/**
 * @brief Fait clignoter la LED RGB
 * @param color Couleur du clignotement (format 0xRRGGBB)
 * @param times Nombre de clignotements (défaut: 1)
 * 
 * @details
 * Chaque clignotement dure 200ms (100ms allumé + 100ms éteint).
 * Utilisé principalement pour signaler les erreurs critiques.
 */
void blinkLED(uint32_t color, int times = 1) {
    for (int i = 0; i < times; i++) {
        setStatusLED(color);
        delay(100);
        setStatusLED(0x000000);
        delay(100);
    }
}

// ============================================================================
// SETUP
// ============================================================================

/**
 * @brief Initialisation du système
 * 
 * @details
 * Séquence d'initialisation:
 * 1. Serial (115200 baud) pour debugging
 * 2. M5Stack (configuration AtomS3 Lite)
 * 3. FastLED (LED RGB de statut)
 * 4. GPS (Serial2 sur GPIO5/6 ou GPIO22/19)
 * 5. ESP-NOW (communication broadcast)
 * 6. Logger (système de journalisation)
 * 7. Storage (carte SD si disponible)
 * 
 * En cas d'erreur critique (GPS ou ESP-NOW), le système
 * affiche une LED rouge clignotante et s'arrête.
 */
void setup() {
    // Initialize serial first for debugging
    Serial.begin(115200);
    
    // Wait for USB Serial to be ready (ESP32-S3 with USB CDC)
    unsigned long startMillis = millis();
    while (!Serial && (millis() - startMillis < 3000)) {
        delay(100);
    }
    delay(500);
    
    // Initialize M5Stack with custom config for AtomS3 Lite
    auto cfg = M5.config();
    cfg.clear_display = true;
    cfg.output_power = true;
    cfg.internal_imu = false;      // AtomS3 Lite doesn't have IMU
    cfg.internal_rtc = false;       // AtomS3 Lite doesn't have RTC
    cfg.internal_spk = false;       // AtomS3 Lite doesn't have speaker
    cfg.internal_mic = false;       // AtomS3 Lite doesn't have microphone
    
    M5.begin(cfg);
    
    // Initialize FastLED for AtomS3 Lite RGB LED
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_COUNT);
    FastLED.setBrightness(50);  // Set brightness to 50/255
    
    delay(500);
    
    Serial.println();
    Serial.println("==========================================");
    Serial.println("  OpenSailingRC - BoatGPS Tracker v1.0");
    Serial.println("==========================================");
    Serial.println();
    
    // Blue LED: Initialization
    setStatusLED(0x0000FF);
    
    // Initialize GPS
    Serial.println("1. Initializing GPS...");
    if (!gps.begin()) {
        Serial.println("✗ GPS initialization failed!");
        blinkLED(0xFF0000, 5);  // Red blink = error
        while(1) delay(1000);
    }
    
    // Note: UBX configuration is disabled for AT6668 GPS module
    // AT6668 uses CASIC protocol, not u-blox UBX protocol
    // The module will use its default configuration (GPS + BDS + GLONASS)
    
    // Initialize Communication
    Serial.println();
    Serial.println("2. Initializing ESP-NOW...");
    if (!comm.begin()) {
        Serial.println("✗ Communication initialization failed!");
        blinkLED(0xFF0000, 5);  // Red blink = error
        while(1) delay(1000);
    }
    
    // Get MAC address
    uint8_t mac[6];
    comm.getLocalMAC(mac);
    Serial.print("  Boat ID (MAC): ");
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", mac[i]);
        if (i < 5) Serial.print(":");
    }
    Serial.println();
    
    // Initialize Logger
    Serial.println();
    Serial.println("3. Initializing Logger...");
    Logger::begin();
    
    // Initialize Storage
    Serial.println();
    Serial.println("4. Initializing Storage...");
    if (ENABLE_SD_STORAGE) {
        if (!storage.begin(true)) {
            Serial.println("⚠️  SD card initialization warning (continuing anyway)");
        }
    } else {
        Serial.println("✓ SD storage disabled (AtomS3 Lite configuration)");
    }
    
    Serial.println();
    Serial.println("==========================================");
    Serial.println("  System Ready - Waiting for GPS fix...");
    Serial.println("==========================================");
    Serial.println();
    
    // Yellow LED: Waiting for GPS fix
    setStatusLED(0xFFFF00);
}

// ============================================================================
// LOOP
// ============================================================================

/**
 * @brief Boucle principale du programme
 * 
 * @details
 * Cycle de fonctionnement:
 * 1. Mise à jour M5Stack (gestion boutons)
 * 2. Mise à jour GPS (parsing NMEA continu)
 * 3. Vérification intervalle de broadcast (1 Hz par défaut)
 * 4. Si GPS valide:
 *    - Broadcast ESP-NOW avec retry (4 tentatives)
 *    - Log série avec numéro de séquence
 *    - Sauvegarde SD (si activée)
 *    - LED verte (transmission OK)
 * 5. Si GPS invalide:
 *    - LED jaune (attente fix)
 *    - Affichage statut (nombre de satellites, HDOP)
 * 6. Rapport de statut toutes les 5 secondes
 * 
 * LED de statut:
 * - Vert  : Données valides, transmission OK
 * - Jaune : Attente fix GPS (< 4 satellites)
 */
void loop() {
    uint32_t currentTime = millis();
    
    // Update M5Stack
    M5.update();
    
    // Update GPS data
    gps.update();
    
    // Get current GPS data
    GPSData data = gps.getData();
    
    // Check if it's time to broadcast
    if (currentTime - lastBroadcast >= BROADCAST_INTERVAL) {
        lastBroadcast = currentTime;
        
        // Only broadcast if GPS data is valid
        if (gps.isValid()) {
            // Green LED: Valid data
            setStatusLED(0x00FF00);
            
            // Get MAC address
            uint8_t mac[6];
            comm.getLocalMAC(mac);
            
            // Broadcast GPS data with 4 retries (5 total attempts)
            // This improves reliability in case of packet loss
            bool success = comm.broadcastGPSData(data, 4);
            
            if (success) {
                validPacketCount++;
                
                // Get sequence number after broadcast
                uint32_t seqNum = comm.getSequenceNumber();
                
                // Log to serial with sequence number
                Serial.printf("[SEQ #%lu] ", seqNum);
                Logger::logGPSData(data, mac);
                
                // Save to SD card with sequence number (if enabled)
                if (ENABLE_SD_STORAGE) {
                    storage.writeGPSData(data, mac, seqNum);
                }
            }
            
        } else {
            // Yellow LED: Waiting for valid fix
            setStatusLED(0xFFFF00);
            invalidPacketCount++;
            
            Serial.printf("⏳ Waiting for GPS fix... (sats: %d, HDOP: %.1f)\n",
                         gps.getSatellites(),
                         gps.getHDOP());
        }
    }
    
    // Status update
    if (currentTime - lastStatus >= STATUS_INTERVAL) {
        lastStatus = currentTime;
        
        Serial.println();
        Serial.println("--- Status Update ---");
        Serial.printf("Uptime: %lu s\n", currentTime / 1000);
        Serial.printf("GPS: %s (%d satellites, HDOP: %.1f)\n",
                     gps.isValid() ? "VALID" : "INVALID",
                     gps.getSatellites(),
                     gps.getHDOP());
        Serial.printf("Packets: %lu valid, %lu invalid\n",
                     validPacketCount, invalidPacketCount);
        
        if (storage.isAvailable()) {
            Serial.printf("SD Storage: %s\n", storage.getCurrentFileName().c_str());
        } else {
            Serial.println("SD Storage: Disabled");
        }
        
        if (gps.isValid()) {
            Serial.printf("Position: %.6f, %.6f\n", data.latitude, data.longitude);
            Serial.printf("Speed: %.1f kts, Course: %.0f°\n", data.speed, data.course);
        }
        
        Serial.println("--------------------");
        Serial.println();
    }
    
    // Small delay to prevent overwhelming the system
    delay(10);
}
