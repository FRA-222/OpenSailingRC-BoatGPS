# Configuration des préférences M5Burner

## OpenSailingRC-BoatGPS

### Paramètre : Boat Name (Nom du bateau)

Ce firmware permet de définir un nom personnalisé pour votre bateau via M5Burner.

#### Configuration dans M5Burner

1. **Ouvrir M5Burner**
2. **Sélectionner le firmware** OpenSailingRC-BoatGPS
3. **Cliquer sur "Configuration"** ou l'icône d'engrenage
4. **Entrer un nom personnalisé** dans le champ "Boat Name"
   - Maximum 17 caractères
   - Exemples : `BOAT1`, `Starboard`, `Tribord`, `Skipper`
   - Laisser vide pour utiliser l'adresse MAC

5. **Flasher le firmware** avec la configuration

#### Comportement

- **Avec nom personnalisé** : Le champ `name` contiendra votre nom personnalisé (ex: "BOAT1")
- **Sans nom personnalisé** : Le champ `name` contiendra l'adresse MAC (ex: "AA:BB:CC:DD:EE:FF")

#### Avantages

✅ Identification facile des bateaux sur le Display  
✅ Noms lisibles dans les logs SD  
✅ Configuration persistante (stockée en mémoire flash)  
✅ Pas besoin de recompiler le firmware  

#### Vérification

Au démarrage, le GPS affiche dans le Serial Monitor :
```
2. Initializing ESP-NOW...
  No custom boat name - using MAC address
  Boat ID (MAC): AA:BB:CC:DD:EE:FF
```

Ou avec un nom personnalisé :
```
2. Initializing ESP-NOW...
  Custom boat name: BOAT1
  Boat ID (MAC): AA:BB:CC:DD:EE:FF
```

#### Notes techniques

- Les préférences sont stockées dans la partition NVS (Non-Volatile Storage)
- La clé de préférence est : `boat_name`
- Le namespace est : `boatgps`
- La longueur maximale est de 17 caractères (18 avec le null terminator)

#### Compatibilité

Compatible avec :
- Display OpenSailingRC v1.0.4+
- Toutes les versions précédentes (MAC address utilisée si pas de nom personnalisé)
