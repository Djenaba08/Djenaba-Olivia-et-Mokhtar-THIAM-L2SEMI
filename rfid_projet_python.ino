#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>


const int rs = A0, en = A1, d4 = A2, d5 = A3, d6 = A4, d7 = A5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define RST_PIN      9
#define SS_PIN      10
#define BUZZER_PIN   8
#define SERVO_PIN    3
#define LED_ROUGE    4
#define LED_VERTE    5
#define LED_ORANGE   6

MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo myServo;


byte uidCarte1[] = {0xC3, 0xAD, 0x21, 0x13};
byte uidCarte2[] = {0xC3, 0x1C, 0x6D, 0x93};
byte uidCarte3[] = {0x00, 0xE4, 0xDA, 0x11};


int compteurRefus = 0;
const String NOM_PROPRIETAIRE = "MATAR";
bool modeSilencieux = false;


struct Parametres {
  int tempoPorte = 3000;
  int tempoAlarme = 80;
  int nbRefusMax = 3;
  bool sonActif = true;
  int volumeBuzzer = 100;
} parametres;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  
  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_ROUGE, OUTPUT);
  pinMode(LED_VERTE, OUTPUT);
  pinMode(LED_ORANGE, OUTPUT);
  
  myServo.attach(SERVO_PIN);
  fermerPorte();

  sequenceStartup();
  afficherMessageCentre("Systeme MATAR", "Scanner carte");
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    return;

  afficherMessageCentre("Carte detectee", "Analyse en cours...");
  printUID(mfrc522.uid.uidByte, mfrc522.uid.size);

  if (compareUID(mfrc522.uid.uidByte, uidCarte1, 4) ||
      compareUID(mfrc522.uid.uidByte, uidCarte2, 4) ||
      compareUID(mfrc522.uid.uidByte, uidCarte3, 4)) {
    accesAutorise();
  } else {
    accesRefuse();
  }

  mfrc522.PICC_HaltA();
  delay(500);
}



void afficherMessageCentre(String ligne1, String ligne2) {
  lcd.clear();
  
  // Effet d'écriture pour la ligne 1
  if(ligne1 != "") {
    int startPos = (16 - ligne1.length()) / 2;
    for(int i=0; i<ligne1.length(); i++) {
      lcd.setCursor(startPos + i, 0);
      lcd.print(ligne1[i]);
      if(!modeSilencieux) tone(BUZZER_PIN, 3000 + i*50, 20);
      delay(30);
    }
  }
  

  if(ligne2 != "") {
    int startPos = (16 - ligne2.length()) / 2;
    String buffer = "                ";
    for(int i=0; i<=ligne2.length(); i++) {
      lcd.setCursor(startPos, 1);
      buffer = ligne2.substring(0, i) + "                ";
      lcd.print(buffer.substring(0, 16));
      delay(50);
    }
  }
  noTone(BUZZER_PIN);
}

void animationLCDAlarme() {
  for(int i=0; i<8; i++) {
    lcd.noDisplay();
    delay(50);
    lcd.display();
    lcd.scrollDisplayLeft();
    delay(50);
  }
  lcd.clear();
}

// =================================================================
// === FONCTIONS DE GESTION D'ACCÈS ===
// =================================================================

void accesAutorise() {
  compteurRefus = 0;
  afficherMessageCentre("Acces autorise", "Bienvenue " + NOM_PROPRIETAIRE);
  
  // Séquence sonore et visuelle
  bipSuccess();
  animationLED(LED_VERTE, 8, 50);
  
  // Ouverture dramatique
  animationOuverture();
  ouvrirPorte();
  delay(parametres.tempoPorte);
  
  // Fermeture avec effets
  animationFermeture();
  fermerPorte();
  
  afficherMessageCentre("Systeme pret", "Scanner carte");
}

void accesRefuse() {
  compteurRefus++;
  afficherMessageCentre("Acces refuse", "Carte invalide");
  

  bipError();
  animationLED(LED_ROUGE, 5, 100);

  if (compteurRefus >= parametres.nbRefusMax) {
    modeAlarme();
  } else {
  
    for(int i=0; i<3; i++) {
      afficherMessageCentre("Essais restants:", "    " + String(parametres.nbRefusMax - compteurRefus) + "    ");
      delay(300);
      afficherMessageCentre("Essais restants:", ">>> " + String(parametres.nbRefusMax - compteurRefus) + " <<<");
      delay(300);
    }
    afficherMessageCentre("Systeme pret", "Scanner carte");
  }
}

// =================================================================
// === FONCTIONS D'EFFETS SONORES ULTRA-AMÉLIORÉES ===
// =================================================================

void bipSuccess() {
  if(!parametres.sonActif || modeSilencieux) return;
  
  // Séquence ascendante futuriste
  for(int i=0; i<3; i++) {
    for(int freq=500; freq<1200; freq+=50) {
      tone(BUZZER_PIN, freq, 20);
      delay(15);
    }
  }
  // Finale triomphale
  tone(BUZZER_PIN, 1568, 200);
  delay(50);
  tone(BUZZER_PIN, 2093, 300);
  delay(350);
  noTone(BUZZER_PIN);
}

void bipError() {
  if(!parametres.sonActif || modeSilencieux) return;
  
  // Séquence descendante alarmante
  tone(BUZZER_PIN, 1200, 100);
  delay(120);
  tone(BUZZER_PIN, 600, 100);
  delay(120);
  tone(BUZZER_PIN, 300, 300);
  delay(350);
  
  // Vibration supplémentaire
  for(int i=0; i<2; i++) {
    tone(BUZZER_PIN, 200, 50);
    delay(70);
    tone(BUZZER_PIN, 150, 50);
    delay(70);
  }
  noTone(BUZZER_PIN);
}

void bipAlarme() {
  if(!parametres.sonActif || modeSilencieux) return;
  
  // Séquence d'alarme ultra-urgente
  for(int cycles=0; cycles<15; cycles++) {
    for(int freq=800; freq<2000; freq+=100) {
      tone(BUZZER_PIN, freq, 30);
      delay(20);
      digitalWrite(LED_ROUGE, !digitalRead(LED_ROUGE));
    }
    for(int freq=2000; freq>500; freq-=80) {
      tone(BUZZER_PIN, freq, 30);
      delay(15);
      digitalWrite(LED_ORANGE, !digitalRead(LED_ORANGE));
    }
  }
  noTone(BUZZER_PIN);
}

// =================================================================
// === ANIMATIONS DE PORTE CINÉMATIQUES ===
// =================================================================

void ouvrirPorte() {
  myServo.write(180);
  digitalWrite(LED_VERTE, HIGH);
}

void fermerPorte() {
  myServo.write(0);
  digitalWrite(LED_VERTE, LOW);
}

void animationOuverture() {
  // Suspense avant ouverture
  for(int i=0; i<3; i++) {
    digitalWrite(LED_VERTE, HIGH);
    if(!modeSilencieux) tone(BUZZER_PIN, 200, 50);
    delay(100);
    digitalWrite(LED_VERTE, LOW);
    delay(200);
  }
  
  // Ouverture avec sonorisation
  if(!modeSilencieux) tone(BUZZER_PIN, 400, 200);
  for(int pos=0; pos<=180; pos+=2) {
    myServo.write(pos);
    
    if(!modeSilencieux && pos%10 == 0) {
      tone(BUZZER_PIN, 300 + pos*5, 50);
    }
    
    digitalWrite(LED_VERTE, pos%20 < 10);
    delay(30 + pos);
  }
  
  // Finale
  for(int i=0; i<5; i++) {
    digitalWrite(LED_VERTE, HIGH);
    if(!modeSilencieux) tone(BUZZER_PIN, 1000 + i*100, 50);
    delay(50);
    digitalWrite(LED_VERTE, LOW);
    delay(50);
  }
  digitalWrite(LED_VERTE, HIGH);
  noTone(BUZZER_PIN);
}

void animationFermeture() {
  // Avertissement avant fermeture
  for(int i=0; i<3; i++) {
    digitalWrite(LED_VERTE, LOW);
    delay(200);
    digitalWrite(LED_VERTE, HIGH);
    if(!modeSilencieux) tone(BUZZER_PIN, 800 - i*100, 50);
    delay(200);
  }
  
  // Fermeture progressive
  for(int pos=180; pos>=0; pos-=2) {
    myServo.write(pos);
    digitalWrite(LED_VERTE, pos%20 < 10);
    delay(30 + (180-pos));
  }
  noTone(BUZZER_PIN);
}

// =================================================================
// === MODE ALARME ULTRA-IMPRESSIF ===
// =================================================================

void modeAlarme() {
  // Compte à rebours avant alarme
  for(int i=5; i>0; i--) {
    afficherMessageCentre("ALARME ACTIVEE", "Debut dans " + String(i));
    if(!modeSilencieux) tone(BUZZER_PIN, 800 + i*100, 200);
    digitalWrite(LED_ROUGE, HIGH);
    delay(300);
    digitalWrite(LED_ROUGE, LOW);
    delay(300);
  }
  
  // Phase d'alarme intensive
  for(int cycles=0; cycles<3; cycles++) {
    // Phase 1: Alarme stridente
    for(int i=0; i<10; i++) {
      lcd.noDisplay();
      if(!modeSilencieux) tone(BUZZER_PIN, 1500, 50);
      digitalWrite(LED_ROUGE, HIGH);
      delay(50);
      lcd.display();
      if(!modeSilencieux) tone(BUZZER_PIN, 800, 50);
      digitalWrite(LED_ROUGE, LOW);
      digitalWrite(LED_ORANGE, HIGH);
      delay(50);
      digitalWrite(LED_ORANGE, LOW);
    }
    
    // Phase 2: Affichage clignotant
    afficherMessageCentre("!!! INTRUSION !!!", "SECURITE ACTIVE");
    for(int i=0; i<8; i++) {
      if(!modeSilencieux) tone(BUZZER_PIN, 1000 + random(500), 30);
      digitalWrite(LED_ROUGE, i%2);
      digitalWrite(LED_ORANGE, !(i%2));
      delay(100 + random(100));
    }
  }
  
  for(int i=0; i<3; i++) {
    if(!modeSilencieux) tone(BUZZER_PIN, 800 - i*200, 200);
    digitalWrite(LED_ROUGE, HIGH);
    digitalWrite(LED_ORANGE, HIGH);
    delay(300 - i*80);
    digitalWrite(LED_ROUGE, LOW);
    digitalWrite(LED_ORANGE, LOW);
    delay(300 - i*80);
  }
  
  noTone(BUZZER_PIN);
  compteurRefus = 0;
  afficherMessageCentre("ALARME TERMINEE", "Systeme reactive");
  delay(2000);
  afficherMessageCentre("Systeme pret", "Scanner carte");
}

// =================================================================
// === FONCTIONS UTILITAIRES AVANCÉES ===
// =================================================================

void sequenceStartup() {
  // Animation LEDs futuriste
  for(int i=0; i<5; i++) {
    digitalWrite(LED_ROUGE, HIGH);
    delay(75);
    digitalWrite(LED_ORANGE, HIGH);
    delay(75);
    digitalWrite(LED_VERTE, HIGH);
    delay(75);
    digitalWrite(LED_ROUGE, LOW);
    delay(75);
    digitalWrite(LED_ORANGE, LOW);
    delay(75);
    digitalWrite(LED_VERTE, LOW);
    delay(75);
  }

  // Son de démarrage cinématique
  if(parametres.sonActif) {
    tone(BUZZER_PIN, 523, 150);  // Do
    delay(175);
    tone(BUZZER_PIN, 659, 150);  // Mi
    delay(175);
    tone(BUZZER_PIN, 784, 300);  // Sol
    delay(350);
    tone(BUZZER_PIN, 659, 150);  // Mi
    delay(175);
    tone(BUZZER_PIN, 784, 600);  // Sol
    delay(650);
    noTone(BUZZER_PIN);
  }

  // Affichage LCD animé
  lcd.clear();
  String message = "SYSTEME MATAR v2.0";
  for(int i=0; i<message.length(); i++) {
    if(i < 16) {
      lcd.setCursor(i, 0);
      lcd.print(message[i]);
    } else {
      lcd.scrollDisplayLeft();
      lcd.print(message[i]);
    }
    if(parametres.sonActif) tone(BUZZER_PIN, 2000 + i*30, 30);
    delay(100);
  }
  delay(500);
  lcd.clear();
  noTone(BUZZER_PIN);
}

void animationLED(int couleur, int repetitions, int vitesse) {
  for(int i=0; i<repetitions; i++) {
    digitalWrite(couleur, HIGH);
    delay(vitesse/2);
    digitalWrite(couleur, LOW);
    delay(vitesse/3);
    digitalWrite(couleur, HIGH);
    delay(vitesse/4);
    digitalWrite(couleur, LOW);
    delay(vitesse);
  }
}

bool compareUID(byte *uid, byte *refUID, byte size) {
  for(byte i=0; i<size; i++) {
    if(uid[i] != refUID[i]) return false;
  }
  return true;
}

void printUID(byte *uid, byte size) {
  Serial.print("UID:");
  for(byte i=0; i<size; i++) {
    Serial.print(uid[i] < 0x10 ? " 0" : " ");
    Serial.print(uid[i], HEX);
  }
  Serial.println();
}

void chargerParametres() {
  // Implémentation EEPROM à ajouter
}

void sauvegarderParametres() {
  // Implémentation EEPROM à ajouter
}
