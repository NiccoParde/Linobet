/*
 * ================================================================
 *           ARDUINO 2 - PANNELLO LCD
 * ================================================================
 *
 *  LCD parallelo  RS=7, E=8, D4=9, D5=10, D6=11, D7=12
 *  Joystick       VRX=A0, VRY=A1, SW=D2, VCC=5V, GND
 *  Buzzer         polo+=D3, polo-=GND
 *  D5        -->  A1 Arduino 1   [START]
 *  A3 (RX)   <-- A3 (TX) Arduino 1   [SoftwareSerial RISULTATO]
 *  A4 (TX)   --> A4 (RX) Arduino 1   [SoftwareSerial - non usato ora]
 *  GND       --> GND Arduino 1        [OBBLIGATORIO - massa comune]
 *
 *  NOTA: non serve piu la resistenza pull-down su A2,
 *        SoftwareSerial gestisce tutto internamente.
 *
 *  REGOLE:
 *   Saldo iniziale : 1000 cr
 *   Puntata minima : 50 cr, passi da 50
 *   Vincita LED  1 : puntata x13
 *   Vincita LED 2-13: puntata x7
 */

#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

// SoftwareSerial: RX=A3, TX=A4
// Il pin A3 di Arduino 2 va collegato ad A3 di Arduino 1 (che e il TX)
SoftwareSerial mySerial(A3, A4);

// LCD parallelo (NON I2C)
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// Joystick
#define JOY_X  A0
#define JOY_Y  A1
#define JOY_SW 2

// Comunicazione con Arduino 1
const int OUTPUT_PIN = 5;   // D5 - START verso Arduino 1

// Buzzer locale
const int BUZZER = 3;

// Laser di vincita
const int LASER = 13;

// --- CARATTERI CUSTOM ---
byte frecciaSu[8] = {B00000,B00000,B00000,B00000,B00100,B01110,B11111,B00000};
byte frecciaGiu[8]= {B00000,B11111,B01110,B00100,B00000,B00000,B00000,B00000};
byte dollaro[8]   = {B00100,B01110,B01000,B01110,B00010,B01110,B00100,B00000};
byte check[8]     = {B00000,B00001,B00001,B10011,B11010,B01110,B00100,B00000};
byte crossX[8]    = {B00000,B10001,B01010,B00100,B01010,B10001,B00000,B00000};
byte selector[8]  = {B00000,B00000,B00000,B00110,B00110,B00000,B00000,B00000};

// --- STATO GIOCO ---
// stato 0: selezione LED
// stato 1: selezione puntata
// stato 2: schermata conferma
// stato 3: mostra risultato
int stato          = 0;
int sceltaConferma = 0;  // 0=SI  1=NO

int saldo   = 1000;
int numero  = 0;   // 0-12 -> rappresenta LED 1-13 (0=LED1=x13)
int puntata = 0;

unsigned long lastMove = 0;
const int delayInput   = 200;
bool lastButton        = HIGH;

// ================================================================
//  SETUP
// ================================================================
void setup() {
  lcd.begin(16, 2);
  lcd.createChar(0, frecciaSu);
  lcd.createChar(1, frecciaGiu);
  lcd.createChar(2, dollaro);
  lcd.createChar(3, check);
  lcd.createChar(4, crossX);
  lcd.createChar(5, selector);

  pinMode(JOY_SW,     INPUT_PULLUP);
  pinMode(OUTPUT_PIN, OUTPUT);
  pinMode(BUZZER,     OUTPUT);
  pinMode(LASER,      OUTPUT);
  digitalWrite(LASER, LOW);
  digitalWrite(OUTPUT_PIN, LOW);

  // Avvia SoftwareSerial allo stesso baud rate di Arduino 1
  mySerial.begin(9600);

  // Schermata di benvenuto
  lcd.setCursor(1, 0); lcd.print("** LINOBET **");
  lcd.setCursor(2, 1); lcd.print("  ROULETTE 1.0");
  tone(BUZZER,  880, 200); delay(220);
  tone(BUZZER, 1100, 200); delay(220);
  tone(BUZZER, 1320, 350); delay(600);
  noTone(BUZZER);
  delay(800);
  lcd.clear();

  disegnaBase();
  aggiornaNumero();
  aggiornaSaldo();
}

// ================================================================
//  LOOP
// ================================================================
void loop() {
  int  x      = analogRead(JOY_X);
  int  y      = analogRead(JOY_Y);
  bool button = digitalRead(JOY_SW);

  // ── Gestione click ──────────────────────────────────────────
  if (lastButton == HIGH && button == LOW) {

    if (stato == 2) {
      if (sceltaConferma == 0) {
        // SI -> avvia estrazione
        avviaEstrazione();
      } else {
        // NO -> torna a selezione LED
        stato = 0;
        lcd.clear();
        disegnaBase();
        aggiornaNumero();
        aggiornaSaldo();
        beepBreve();
      }

    } else if (stato == 3) {
      // Risultato mostrato -> nuovo giro
      controllaGameOver();
      puntata = 0;
      numero  = 0;
      stato   = 0;
      lcd.clear();
      disegnaBase();
      aggiornaNumero();
      aggiornaSaldo();

    } else {
      // stato 0->1 o 1->2
      if (stato == 1 && puntata == 0) {
        // Non si avanza senza puntata
        lcd.setCursor(0, 1);
        lcd.print("  Min. 50 cr!   ");
        tone(BUZZER, 400, 200);
        delay(900);
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(9, 1); lcd.print("LINOBET");
      } else {
        stato++;
        lcd.clear();
        if (stato == 1) {
          disegnaBase();
          aggiornaPuntata();
          aggiornaSaldo();
        }
        if (stato == 2) {
          sceltaConferma = 0;
          schermataConferma();
        }
        beepBreve();
      }
    }
  }
  lastButton = button;

  // ── Input joystick ──────────────────────────────────────────
  if (millis() - lastMove > delayInput) {

    // Selezione LED (stato 0)
    if (stato == 0) {
      if (y < 300 && numero < 12) {
        numero++;
        aggiornaNumero();
        lastMove = millis();
      }
      if (y > 700 && numero > 0) {
        numero--;
        aggiornaNumero();
        lastMove = millis();
      }
    }

    // Selezione puntata (stato 1)
    if (stato == 1) {
      if (y < 300 && puntata + 50 <= saldo) {
        puntata += 50;
        aggiornaPuntata();
        lastMove = millis();
      }
      if (y > 700 && puntata - 50 >= 0) {
        puntata -= 50;
        aggiornaPuntata();
        lastMove = millis();
      }
      // Joystick sinistra -> torna a selezione LED
      if (x < 300) {
        stato = 0;
        lcd.clear();
        disegnaBase();
        aggiornaNumero();
        aggiornaSaldo();
        beepBreve();
        lastMove = millis();
      }
    }

    // Conferma (stato 2): sinistra=SI, destra=NO
    if (stato == 2) {
      bool changed = false;
      if (x < 300 && sceltaConferma != 0) { sceltaConferma = 0; changed = true; }
      if (x > 700 && sceltaConferma != 1) { sceltaConferma = 1; changed = true; }
      if (changed) {
        aggiornaConferma();
        beepBreve();
        lastMove = millis();
      }
    }
  }
}

// ================================================================
//  AVVIA ESTRAZIONE  (bloccante fino al risultato)
// ================================================================
void avviaEstrazione() {
  saldo -= puntata;

  // Svuota eventuali byte rimasti nel buffer seriale
  while (mySerial.available()) mySerial.read();

  // Invia segnale START ad Arduino 1 (HIGH per 800ms)
  digitalWrite(OUTPUT_PIN, HIGH);
  delay(800);
  digitalWrite(OUTPUT_PIN, LOW);

  // Schermata di attesa
  lcd.clear();
  lcd.setCursor(2, 0); lcd.print("Estrazione...");
  lcd.setCursor(3, 1); lcd.print("In corso...");

  // Aspetta il byte di risultato da Arduino 1 via SoftwareSerial
  int ledVincente = leggiRisultato();

  // ── Errore di comunicazione ────────────────────────────────
  if (ledVincente < 1 || ledVincente > 13) {
    saldo += puntata;  // rimborso automatico
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Errore segnale!");
    lcd.setCursor(0, 1); lcd.print("Bet rimborsata.");
    tone(BUZZER, 300, 800);
    delay(3000);
    stato = 0;
    lcd.clear();
    disegnaBase();
    aggiornaNumero();
    aggiornaSaldo();
    return;
  }

  // ── Calcola risultato ──────────────────────────────────────
  int  ledPuntato = numero + 1;           // converti 0-12 in 1-13
  bool vinto      = (ledVincente == ledPuntato);
  int  premio     = 0;

  if (vinto) {
    int moltiplicatore = (ledPuntato == 1) ? 13 : 7;
    premio  = puntata * moltiplicatore;
    saldo  += premio;
    beepVincita();
  } else {
    beepSconfitta();
  }

  stato = 3;
  mostraRisultato(vinto, ledVincente, ledPuntato, premio);
}

// ================================================================
//  LEGGI RISULTATO DA ARDUINO 1
//  Aspetta un singolo byte (1-13) via SoftwareSerial.
//  Timeout 35 secondi per coprire spin + lampeggio + invio.
// ================================================================
int leggiRisultato() {
  unsigned long timeout = millis() + 35000UL;

  while (!mySerial.available()) {
    if (millis() > timeout) return -1;
    delay(8);
  }

  return mySerial.read();  // legge direttamente il numero 1-13
}

// ================================================================
//  GAME OVER
// ================================================================
void controllaGameOver() {
  if (saldo < 50) {
    lcd.clear();
    lcd.setCursor(3, 0); lcd.print("GAME OVER!");
    lcd.setCursor(0, 1); lcd.print("Riavvia Arduino!");
    while (true) {
      tone(BUZZER, 220, 400);
      delay(800);
    }
  }
}

// ================================================================
//  FUNZIONI DISPLAY
// ================================================================

void mostraRisultato(bool vinto, int ledVincente, int ledPuntato, int premio) {
  lcd.clear();
  if (vinto) {
    lcd.setCursor(0, 0); lcd.print("** HAI VINTO! **");
    lcd.setCursor(0, 1);
    lcd.print("+");
    lcd.print(premio);
    lcd.print("$ ");
    lcd.write(byte(2));
    lcd.print(saldo);
  } else {
    lcd.setCursor(0, 0); lcd.print("Hai perso  :(");
    lcd.setCursor(0, 1);
    lcd.print("LED:");
    lcd.print(ledVincente);
    lcd.print(" Tu:");
    lcd.print(ledPuntato);
    lcd.print(" ");
    lcd.write(byte(2));
    lcd.print(saldo);
  }
}

void disegnaBase() {
  lcd.setCursor(1, 0); lcd.write(byte(0));  // freccia su
  lcd.setCursor(1, 1); lcd.write(byte(1));  // freccia giu
  lcd.setCursor(9, 1); lcd.print("LINOBET");
  aggiornaSaldo();
}

void aggiornaNumero() {
  lcd.setCursor(3, 0);
  lcd.print("  ");
  lcd.setCursor(3, 0);
  // numero 0-12 mostrato come 01-13
  if ((numero + 1) < 10) lcd.print("0");
  lcd.print(numero + 1);
}

void aggiornaPuntata() {
  lcd.setCursor(3, 0); lcd.write(byte(2));
  lcd.setCursor(4, 0); lcd.print("    ");
  lcd.setCursor(4, 0);
  if (puntata < 1000) lcd.print("0");
  if (puntata < 100)  lcd.print("0");
  if (puntata < 10)   lcd.print("0");
  lcd.print(puntata);
}

void aggiornaSaldo() {
  lcd.setCursor(11, 0); lcd.write(byte(2));
  lcd.setCursor(12, 0); lcd.print("    ");
  lcd.setCursor(12, 0);
  if (saldo > 9999) {
    lcd.print(saldo);
  } else {
    if (saldo < 1000) lcd.print("0");
    if (saldo < 100)  lcd.print("0");
    if (saldo < 10)   lcd.print("0");
    lcd.print(saldo);
  }
}

void schermataConferma() {
  lcd.setCursor(0, 0); lcd.print("LED:");
  lcd.print(numero + 1);
  lcd.print(" Bet:");
  lcd.print(puntata);
  lcd.setCursor(5,  1); lcd.write(byte(3));  // checkmark
  lcd.setCursor(10, 1); lcd.write(byte(4));  // cross
  aggiornaConferma();
}

void aggiornaConferma() {
  lcd.setCursor(4, 1); lcd.print(" ");
  lcd.setCursor(9, 1); lcd.print(" ");
  if (sceltaConferma == 0) {
    lcd.setCursor(4, 1); lcd.write(byte(5));
  } else {
    lcd.setCursor(9, 1); lcd.write(byte(5));
  }
}

// ================================================================
//  SUONI
// ================================================================
void beepBreve() {
  tone(BUZZER, 1200, 40);
}

void beepVincita() {
  int note[] = {523, 659, 784, 1047};
  for (int i = 0; i < 4; i++) {
    // Laser lampeggia in sincronia con ogni nota
    digitalWrite(LASER, HIGH);
    tone(BUZZER, note[i], 130);
    delay(130);
    digitalWrite(LASER, LOW);
    delay(20);
  }
  // Nota finale lunga: laser lampeggia veloce per 500ms
  tone(BUZZER, 1319, 500);
  for (int i = 0; i < 10; i++) {
    digitalWrite(LASER, HIGH);
    delay(50);
    digitalWrite(LASER, LOW);
    delay(50);
  }
  noTone(BUZZER);
  digitalWrite(LASER, LOW);
}

void beepSconfitta() {
  int note[] = {440, 370, 310, 260};
  for (int i = 0; i < 4; i++) {
    tone(BUZZER, note[i], 170);
    delay(195);
   }
  noTone(BUZZER);
}
