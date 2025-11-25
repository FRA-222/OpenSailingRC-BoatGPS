/**
 * Configuration du nom de bateau pour OpenSailingRC-BoatGPS
 * 
 * Instructions :
 * 1. Modifier la variable BOAT_NAME ci-dessous
 * 2. Flasher ce programme sur l'AtomS3
 * 3. Ouvrir le Serial Monitor (115200 baud)
 * 4. Reflasher le firmware BoatGPS v1.0.4
 * 
 * Le nom sera conservé en mémoire NVS.
 */

#include <Preferences.h>

// ============================================
// CONFIGURATION : Modifier le nom ici
// ============================================
const char* BOAT_NAME = "BOAT1";  // Max 17 caractères
// ============================================

Preferences preferences;

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n===========================================");
  Serial.println("Configuration du nom de bateau");
  Serial.println("===========================================\n");
  
  // Vérifier la longueur
  size_t len = strlen(BOAT_NAME);
  if (len > 17) {
    Serial.printf("ERREUR : Le nom est trop long (%d caractères, max 17)\n", len);
    Serial.printf("Nom : '%s'\n", BOAT_NAME);
    return;
  }
  
  // Ouvrir les préférences en écriture
  preferences.begin("boatgps", false);
  
  // Lire l'ancienne valeur
  String oldName = preferences.getString("boat_name", "");
  Serial.printf("Ancien nom : '%s'\n", oldName.c_str());
  
  // Écrire la nouvelle valeur
  preferences.putString("boat_name", BOAT_NAME);
  
  // Vérifier l'écriture
  String newName = preferences.getString("boat_name", "");
  preferences.end();
  
  Serial.printf("Nouveau nom : '%s'\n", newName.c_str());
  
  if (newName == String(BOAT_NAME)) {
    Serial.println("\n✅ Configuration réussie !");
    Serial.println("Vous pouvez maintenant reflasher le firmware BoatGPS.");
  } else {
    Serial.println("\n❌ Erreur lors de l'écriture !");
  }
  
  Serial.println("===========================================\n");
}

void loop() {
  delay(1000);
}
