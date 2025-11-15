/**
 * @file main.cpp
 * @brief Main program for OpenSailingRC-BoatGPS
 * @author OpenSailingRC
 * @date 2025
 * 
 * GPS tracker with ESP-NOW broadcast for sailing boats.
 * Transmits GPS position to all listening devices and logs to SD card.
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
// AtomS3 Lite configuration
const uint8_t GPS_RX_PIN = 5;              // GPIO5 for GPS RX
const uint8_t GPS_TX_PIN = 6;              // GPIO6 for GPS TX
const uint8_t LED_PIN = 35;                // RGB LED on GPIO35 (AtomS3)
#else
// Atom Lite configuration (ESP32 classic)
const uint8_t GPS_RX_PIN = 22;             // GPIO22 for GPS RX
const uint8_t GPS_TX_PIN = 19;             // GPIO19 for GPS TX
const uint8_t LED_PIN = 27;                // RGB LED on GPIO27 (Atom Lite)
#endif

const uint32_t BROADCAST_INTERVAL = 1000;  // Broadcast every 1 second
const uint32_t STATUS_INTERVAL = 5000;     // Status update every 5 seconds
const bool ENABLE_SD_STORAGE = true;       // Enable SD card on Atom GPS Base

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
void setStatusLED(uint32_t color) {
    // AtomS3 Lite has one RGB LED on the button (GPIO35)
    leds[0] = CRGB(color);
    FastLED.show();
}

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
    
    // Configure GPS for GPS + Galileo constellations
    Serial.println("  Configuring GPS + Galileo...");
    delay(100);  // Wait for GPS to be ready
    sendUBX(ubx_cfg_gnss_gps_galileo, sizeof(ubx_cfg_gnss_gps_galileo));
    delay(500);  // Wait for configuration to be applied
    Serial.println("  GPS constellation configuration sent");
    
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
    if (!storage.begin(ENABLE_SD_STORAGE)) {
        Serial.println("⚠️  Storage initialization warning (continuing anyway)");
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
            
            // Broadcast GPS data
            bool success = comm.broadcastGPSData(data);
            
            if (success) {
                validPacketCount++;
                
                // Log to serial
                Logger::logGPSData(data, mac);
                
                // Save to SD card
                storage.writeGPSData(data, mac);
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
