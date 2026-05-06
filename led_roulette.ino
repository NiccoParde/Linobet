/*
 * ╔══════════════════════════════════════════════════════════╗
 * ║           ARDUINO 1 — ROULETTE LED                       ║
 * ╚══════════════════════════════════════════════════════════╝
 *
 *  D2-D13  → LED 1-12  (resistenza 220Ω in serie, poi GND)
 *  A0      → LED 13    (resistenza 220Ω in serie, poi GND)
 *  A2      → Buzzer polo+  (polo− a GND)
 *  A1      ← D5 Arduino 2  [START - riceve HIGH per avviare]
 *  A3 (TX) → A3 (RX) Arduino 2  [SoftwareSerial - RISULTATO]
 *  A4 (RX) ← A4 (TX) Arduino 2  [SoftwareSerial - non usato ora]
 *  GND     → GND Arduino 2 [OBBLIGATORIO - massa comune]
 *
 *  NOTA: il pin originale 1 era il TX seriale (problematico),
 *        sostituito con A0 per il 13° LED.
 */

#include <SoftwareSerial.h>

// RX=A4, TX=A3  →  A3 di Arduino 1 va collegato ad A3 di Arduino 2
SoftwareSerial mySerial(A4, A3);

const int numLed = 13;
// LED 1-12 su D2-D13, LED 13 su A0
int ledPins[numLed] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, A0};

const int buzzerPin = A2;
const int inputPin  = A1;   // Riceve HIGH da Arduino 2 → avvia spin

// ─── Setup ────────────────────────────────────────────────────
void setup() {
  pinMode(buzzerPin, OUTPUT);

  for (int i = 0; i < numLed; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  // INPUT semplice: Arduino 2 guida attivamente il pin (non pullup)
  pinMode(inputPin, INPUT);

  // Avvia comunicazione seriale software
  mySerial.begin(9600);

  // Seed casuale più robusto
  randomSeed(analogRead(A5) ^ (unsigned long)millis());

  // Sequenza LED di avvio
  for (int i = 0; i < numLed; i++) {
    digitalWrite(ledPins[i], HIGH);
    tone(buzzerPin, 600 + i * 60, 50);
    delay(70);
    digitalWrite(ledPins[i], LOW);
  }
  delay(300);
}

// ─── Loop principale ──────────────────────────────────────────
void loop() {
  // Rileva fronte HIGH su A1 (segnale START da Arduino 2)
  if (digitalRead(inputPin) == HIGH) {
    delay(50); // debounce
    if (digitalRead(inputPin) == HIGH) {
      spinRoulette();
      // Aspetta che il segnale torni LOW prima di accettare nuovi input
      unsigned long t = millis();
      while (digitalRead(inputPin) == HIGH && millis() - t < 5000UL) {
        delay(10);
      }
    }
  }
}

// ─── Funzione principale spin ─────────────────────────────────
void spinRoulette() {
  int posizione = 0;
  int giri      = random(30, 50); // 30-49 passi totali
  int delayTime = 50;             // ms iniziali tra un LED e l'altro

  for (int i = 0; i < giri; i++) {

    // Spegni tutti i LED
    for (int j = 0; j < numLed; j++) {
      digitalWrite(ledPins[j], LOW);
    }

    // Accendi il LED corrente
    digitalWrite(ledPins[posizione], HIGH);

    // Click buzzer con frequenza che scende al rallentare
    int freq = map(delayTime, 50, 350, 2000, 400);
    freq = constrain(freq, 400, 2000);
    tone(buzzerPin, freq, 20);

    delay(delayTime);

    // Avanza posizione
    posizione++;
    if (posizione >= numLed) posizione = 0;

    // Rallenta progressivamente, max 350ms
    delayTime += 6;
    if (delayTime > 350) delayTime = 350;
  }

  // ── LED vincente ──────────────────────────────────────────
  // "posizione" dopo il loop è il LED su cui si è fermata la roulette
  int vincitore = posizione;

  // Assicurati che solo il vincitore sia acceso
  for (int j = 0; j < numLed; j++) {
    digitalWrite(ledPins[j], LOW);
  }
  digitalWrite(ledPins[vincitore], HIGH);

  // Lampeggio finale sul LED vincente
  for (int k = 0; k < 10; k++) {
    digitalWrite(ledPins[vincitore], k % 2 == 0 ? LOW : HIGH);
    delay(180);
  }
  digitalWrite(ledPins[vincitore], HIGH);
  delay(1000);

  // ── Invia risultato ad Arduino 2 via SoftwareSerial ──────
  // Manda direttamente il numero del LED vincente (1-13) come byte
  delay(300);
  mySerial.write(vincitore + 1);

  // Pausa finale, poi spegni tutto
  delay(1000);
  for (int j = 0; j < numLed; j++) {
    digitalWrite(ledPins[j], LOW);
  }
}
