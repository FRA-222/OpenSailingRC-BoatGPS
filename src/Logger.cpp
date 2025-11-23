/**
 * @file Logger.cpp
 * @brief Implémentation de la journalisation des messages applicatifs
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
 * Système de journalisation simple via Serial pour debugging et monitoring.
 * Fournit des fonctions de logging typées (info, warning, error) et
 * une fonction spécialisée pour les données GPS.
 */

#include "Logger.h"
#include "GPS.h"

/**
 * @brief Initialise le système de logging
 * @return true toujours (pas d'erreur possible)
 * 
 * @details
 * Le logger utilise le Serial déjà initialisé dans setup().
 * Cette fonction affiche simplement un message de confirmation.
 */
bool Logger::begin() {
    Serial.println("✓ Logger: Initialized");
    return true;
}

/**
 * @brief Enregistre un message d'information
 * @param message Message à journaliser
 * 
 * @details
 * Préfixe le message avec [INFO] et l'affiche sur le Serial.
 * Utiliser pour les événements normaux du système.
 */
void Logger::info(const String& message) {
    Serial.println("[INFO] " + message);
}

/**
 * @brief Enregistre un message d'avertissement
 * @param message Message à journaliser
 * 
 * @details
 * Préfixe le message avec [WARN] et l'affiche sur le Serial.
 * Utiliser pour les situations anormales non critiques.
 */
void Logger::warning(const String& message) {
    Serial.println("[WARN] " + message);
}

/**
 * @brief Enregistre un message d'erreur
 * @param message Message à journaliser
 * 
 * @details
 * Préfixe le message avec [ERROR] et l'affiche sur le Serial.
 * Utiliser pour les erreurs critiques nécessitant attention.
 */
void Logger::error(const String& message) {
    Serial.println("[ERROR] " + message);
}

/**
 * @brief Enregistre les données GPS sur le Serial (debugging)
 * @param data Structure GPSData à journaliser
 * @param macAddress Adresse MAC du dispositif (6 octets)
 * 
 * @details
 * Affiche une ligne formatée avec toutes les informations GPS:
 * [timestamp] GPS: lat,lon | vitesse cap° | sats | MAC
 * 
 * Exemple:
 * [1234567890] GPS: 43.123456,2.654321 | 4.5kts 285° | 8 sats | MAC: AA:BB:CC:DD:EE:FF
 */
void Logger::logGPSData(const GPSData& data, const uint8_t* macAddress) {
    // Log to serial
    Serial.printf("[%lu] GPS: %.6f,%.6f | %.1fkts %d° | %d sats | MAC: ",
                 data.timestamp,
                 data.latitude,
                 data.longitude,
                 data.speed,
                 (int)data.course,
                 data.satellites);
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", macAddress[i]);
        if (i < 5) Serial.print(":");
    }
    Serial.println();
}
