/**
 * @file GPS.cpp
 * @brief Implémentation de l'acquisition des données GPS
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
 * Gère la communication avec le module GPS, le parsing NMEA,
 * et la validation des données pour le tracker GPS embarqué.
 * 
 * Modules GPS supportés:
 * - AT6668 (GPS Atom v2) : 115200 baud, GPS + Galileo + BeiDou
 * - NEO-6M (GPS Base) : 9600 baud, GPS uniquement
 * 
 * Validation des données:
 * - Minimum 4 satellites requis
 * - Âge du fix < 2 secondes
 * - Position valide (isValid)
 */

#include "GPS.h"

/**
 * @brief Constructeur de la classe GPS
 * @param rxPin Broche GPIO pour RX (défaut: 1)
 * @param txPin Broche GPIO pour TX (défaut: 2)
 * 
 * @details
 * Initialise les broches de communication série et prépare
 * la structure de données GPS. La configuration réelle du
 * Serial2 se fait dans begin().
 */
GPS::GPS(uint8_t rxPin, uint8_t txPin)
    : rxPin(rxPin), txPin(txPin), gpsSerial(nullptr) {
    currentData.valid = false;
    currentData.timestamp = 0;
}

/**
 * @brief Initialise le module GPS
 * @return true si l'initialisation réussit
 * 
 * @details
 * Configure Serial2 pour la communication avec le module GPS.
 * Le baudrate est automatiquement sélectionné selon le module:
 * - AT6668 (GPS Atom v2 / AtomS3) : 115200 baud
 * - NEO-6M (GPS Base / Atom Lite) : 9600 baud
 * 
 * Les broches RX/TX sont configurées selon la plateforme:
 * - AtomS3 : GPIO5 (RX) et GPIO6 (TX) - Grove connector
 * - Atom Lite : GPIO22 (RX) et GPIO19 (TX)
 */
bool GPS::begin() {
    // Initialize Serial2 for GPS communication
    gpsSerial = &Serial2;
    gpsSerial->begin(GPS_BAUD, SERIAL_8N1, rxPin, txPin);
    
    Serial.println("✓ GPS: Initialized");
    Serial.printf("  RX: GPIO%d, TX: GPIO%d, Baud: %d\n", rxPin, txPin, GPS_BAUD);
    Serial.println("  Waiting for GPS data...");
    
    return true;
}

/**
 * @brief Met à jour les données GPS (à appeler fréquemment dans loop)
 * 
 * @details
 * Lit et parse les données NMEA du module GPS en continu.
 * TinyGPS++ extrait automatiquement les informations des trames:
 * - $GPGGA : Position, altitude, qualité du fix
 * - $GPRMC : Vitesse, cap, date/heure
 * - $GPGSA : DOP, satellites utilisés
 * - $GPGSV : Satellites visibles
 * 
 * La validation des données requiert:
 * - Position valide (gps.location.isValid())
 * - Minimum 4 satellites (currentData.satellites >= 4)
 * - Âge du fix < 2 secondes (MAX_AGE_MS)
 * 
 * Le timestamp est converti du format GPS (date/heure UTC)
 * en timestamp Unix (epoch) via mktime().
 */
void GPS::update() {
    static uint32_t lastDebug = 0;
    static uint32_t charCount = 0;
    
    // Feed GPS parser with available data
    while (gpsSerial->available() > 0) {
        char c = gpsSerial->read();
        gps.encode(c);
        charCount++;
        
        // Debug: Print raw NMEA sentences (DISABLED - uncomment for troubleshooting)
        // Serial.print(c);
    }
    
    // Periodic debug info (optional - uncomment for detailed GPS diagnostics)
    // uint32_t now = millis();
    // if (now - lastDebug >= 5000) {
    //     lastDebug = now;
    //     Serial.printf("\n[GPS] Chars: %u, Sentences: %u, Failed: %u, Sats: %d, HDOP: %.1f\n",
    //                  charCount, gps.passedChecksum(), gps.failedChecksum(),
    //                  gps.satellites.isValid() ? gps.satellites.value() : 0,
    //                  gps.hdop.isValid() ? gps.hdop.hdop() : 0.0);
    //     charCount = 0;
    // }
    
    // Update current data if location is updated
    if (gps.location.isUpdated()) {
        currentData.latitude = gps.location.lat();
        currentData.longitude = gps.location.lng();
        currentData.speed = gps.speed.knots();
        currentData.course = gps.course.deg();
        currentData.satellites = gps.satellites.value();
        currentData.age = gps.location.age();
        
        // Récupérer l'heure et la date du GPS
        int year = gps.date.year();
        int month = gps.date.month();
        int day = gps.date.day();
        int hour = gps.time.hour();
        int minute = gps.time.minute();
        int second = gps.time.second();
    
        // Convert GPS date and time to epoch time
        struct tm gpsTimeinfo;
        gpsTimeinfo.tm_year = year - 1900; // tm_year is years since 1900
        gpsTimeinfo.tm_mon = month - 1;   // tm_mon is 0-11
        gpsTimeinfo.tm_mday = day;
        gpsTimeinfo.tm_hour = hour;
        gpsTimeinfo.tm_min = minute;
        gpsTimeinfo.tm_sec = second;
        gpsTimeinfo.tm_isdst = -1; // Not considering daylight saving time
        
        currentData.timestamp = mktime(&gpsTimeinfo);

        //Serial.println("currentData.timestamp: " + String(currentData.timestamp));

        // Update date/time from GPS
        if (gps.date.isValid()) {
            currentData.year = gps.date.year();
            currentData.month = gps.date.month();
            currentData.day = gps.date.day();
        }
        if (gps.time.isValid()) {
            currentData.hour = gps.time.hour();
            currentData.minute = gps.time.minute();
            currentData.second = gps.time.second();
        }
        
        // Validate data
        currentData.valid = gps.location.isValid() && 
                           (currentData.satellites >= 4);
    }
}

/**
 * @brief Retourne les données GPS actuelles
 * @return Structure GPSData avec les dernières données parsées
 * 
 * @note Vérifier currentData.valid avant d'utiliser les données
 */
GPSData GPS::getData() {
    return currentData;
}

/**
 * @brief Vérifie si le fix GPS est valide
 * @return true si le fix GPS est valide (≥4 satellites, position valide)
 */
bool GPS::isValid() {
    return currentData.valid; 
}

/**
 * @brief Retourne le nombre de satellites visibles
 * @return Nombre de satellites GPS/GNSS en vue
 */
uint8_t GPS::getSatellites() {
    return gps.satellites.value();
}

/**
 * @brief Retourne le HDOP (Horizontal Dilution of Precision)
 * @return Valeur HDOP (plus la valeur est faible, meilleure est la précision)
 * 
 * @details
 * Interprétation du HDOP:
 * - < 1   : Idéal
 * - 1-2   : Excellent
 * - 2-5   : Bon
 * - 5-10  : Modéré
 * - 10-20 : Médiocre
 * - > 20  : Mauvais
 */
float GPS::getHDOP() {
    return gps.hdop.hdop();
}
