# Projet Système d'authentification par badge RFID avec journalisation et interface web

## Description
- Arduino + MFRC522 scanne les badges RFID.
- Les accès sont envoyés au PC via port série.
- Python enregistre les accès dans une base de données SQLite.
- Interface web Flask pour visualiser les logs.

## Utilisation
1. Branchez l'Arduino.
2. Lancez `lancer_projet.bat`.
3. Passez un badge RFID sur le lecteur.
4. Consultez les accès sur votre navigateur (`http://127.0.0.1:5000/`).

## Matériel utilisé
- Arduino UNO
- Module RFID-RC522
- Servo moteur (pour ouverture de porte)
- LEDs verte et rouge
- Buzzer
- Ecran LCD I2C 16x2
