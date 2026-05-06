# LINOBET — Arduino Roulette System

## Diritti

Tutti i diritti riservati all’Istituto **ISIS Gobetti Volta di Bagno a Ripoli**.

---

## Descrizione del progetto

LINOBET è un sistema di roulette elettronica realizzato con **due Arduino** che lavorano in parallelo:

* **Arduino 1** → gestisce la roulette fisica (LED + animazione + risultato)
* **Arduino 2** → gestisce l’interfaccia utente (LCD, joystick, puntate, logica di gioco)

Il sistema simula una roulette con 13 posizioni (LED), permettendo all’utente di:

* selezionare un numero
* scegliere una puntata
* avviare l’estrazione
* visualizzare il risultato con effetti sonori e luminosi

---

## Funzionamento

### Flusso generale

1. L’utente utilizza il **joystick** (Arduino 2) per:

   * scegliere un LED (1–13)
   * impostare una puntata (minimo 50 crediti)

2. Conferma la giocata tramite pulsante

3. Arduino 2:

   * invia un segnale HIGH su D5 → avvia Arduino 1

4. Arduino 1:

   * esegue l’animazione della roulette (LED in sequenza)
   * rallenta progressivamente
   * si ferma su un LED vincente
   * invia il risultato (1–13) via SoftwareSerial

5. Arduino 2:

   * riceve il risultato
   * confronta con la scelta dell’utente
   * aggiorna saldo
   * mostra esito su LCD
   * attiva buzzer e laser

---

## Regole di gioco

* Saldo iniziale: **1000 crediti**
* Puntata minima: **50 crediti**
* Incrementi: multipli di 50

### Vincite

* LED 1 → moltiplicatore x13
* LED 2–13 → moltiplicatore x7

---

## Struttura del sistema

### Arduino 1 — Roulette LED

Responsabilità:

* gestione LED (animazione)
* generazione casuale risultato
* invio risultato ad Arduino 2
* feedback sonoro locale

### Arduino 2 — Interfaccia

Responsabilità:

* input utente (joystick)
* gestione puntate e saldo
* display LCD
* effetti (buzzer + laser)
* comunicazione con Arduino 1

---

## Collegamenti hardware

### Arduino 1

| Pin     | Funzione                       |
| ------- | ------------------------------ |
| D2–D13  | LED 1–12 (con resistenza 220Ω) |
| A0      | LED 13 (con resistenza 220Ω)   |
| A2      | Buzzer (+)                     |
| A1      | Input START da Arduino 2       |
| A3 (TX) | Invio risultato                |
| A4 (RX) | Non utilizzato                 |
| GND     | Massa comune                   |

---

### Arduino 2

| Pin     | Funzione                      |
| ------- | ----------------------------- |
| D7–D12  | LCD (RS, E, D4–D7)            |
| A0–A1   | Joystick (X, Y)               |
| D2      | Pulsante joystick             |
| D3      | Buzzer                        |
| D5      | Segnale START verso Arduino 1 |
| A3 (RX) | Ricezione risultato           |
| A4 (TX) | Non utilizzato                |
| D13     | Laser                         |
| GND     | Massa comune                  |

---

## Comunicazione tra Arduino

* Tipo: **SoftwareSerial**
* Baud rate: **9600**

### Segnali:

* START → Arduino 2 → Arduino 1 (digitale HIGH)
* RISULTATO → Arduino 1 → Arduino 2 (byte 1–13)

### Nota fondamentale

La **massa (GND)** deve essere condivisa tra i due Arduino.

---

## Installazione

### Requisiti

* Arduino IDE
* Librerie:

  * `LiquidCrystal`
  * `SoftwareSerial`

### Procedura

1. Collegare entrambi gli Arduino al computer
2. Caricare:

   * codice roulette su Arduino 1
   * codice interfaccia su Arduino 2
3. Verificare i collegamenti hardware
4. Alimentare il sistema

---

## Montaggio del circuito

### Componenti necessari

* 2 Arduino (Uno o compatibili)
* 13 LED
* 13 resistenze da 220Ω
* 1 LCD 16x2
* 1 joystick analogico
* 2 buzzer
* 1 modulo laser
* cavi jumper
* breadboard

---

### Note di montaggio

* Ogni LED deve avere **resistenza in serie**
* Il joystick usa:

  * 2 pin analogici (X, Y)
  * 1 pin digitale (click)
* Il laser deve essere collegato con attenzione alla polarità
* Evitare di usare pin 0 e 1 (seriale hardware)

---

## Come usare il sistema

1. Accensione → schermata iniziale
2. Selezionare il numero con il joystick
3. Premere per passare alla puntata
4. Impostare la puntata
5. Confermare (SI/NO)
6. Attendere estrazione
7. Visualizzare risultato
8. Ripetere finché saldo > 0

---

## Gestione errori

* Timeout comunicazione → rimborso automatico
* Puntata non valida → messaggio su LCD
* Saldo insufficiente → GAME OVER

---

## Crediti

Progetto sviluppato presso:

**ISIS Gobetti Volta di Bagno a Ripoli**

Ambito:

* Sistemi embedded
* Programmazione Arduino
* Interazione hardware/software

Sviluppo:

* Logica di gioco
* Interfaccia LCD
* Comunicazione tra microcontrollori
* Progettazione circuito elettronico

---

## Licenza

Tutti i diritti riservati.
Uso consentito esclusivamente per scopi didattici interni all’istituto.
