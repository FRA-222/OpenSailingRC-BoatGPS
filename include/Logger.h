/**
 * @file Logger.h
 * @brief Journalisation des messages applicatifs
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
 * Système de logging statique pour messages de debug et monitoring.
 * Utilise la sortie série (Serial) pour afficher les messages.
 * 
 * Niveaux de logging:
 * - INFO : Informations générales (démarrage, état)
 * - WARN : Avertissements (SD non disponible, etc.)
 * - ERROR : Erreurs critiques (init failed, etc.)
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

// Forward declaration
struct GPSData;

/**
 * @class Logger
 * @brief Static logging utility for application messages
 * 
 * Provides simple serial output logging for debugging and monitoring.
 * Use Storage class for GPS data persistence to SD card.
 */
class Logger {
public:
    /**
     * @brief Initialize logger
     * @return true if initialization successful
     */
    static bool begin();
    
    /**
     * @brief Log info message
     * @param message Message to log
     */
    static void info(const String& message);
    
    /**
     * @brief Log warning message
     * @param message Message to log
     */
    static void warning(const String& message);
    
    /**
     * @brief Log error message
     * @param message Message to log
     */
    static void error(const String& message);
    
    /**
     * @brief Log GPS data to serial (for debugging)
     * @param data GPS data structure
     * @param macAddress MAC address of the GPS device (6 bytes)
     */
    static void logGPSData(const GPSData& data, const uint8_t* macAddress);
};

#endif // LOGGER_H
