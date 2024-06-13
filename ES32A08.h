#ifndef ES32A08_h
#define ES32A08_h

#include "Arduino.h"

// Pin utilizzati dai 3 registri a scorrimento 74HC595D.
// Questi permettono di controllare gli 8 relè e l'output sui 4 display a cifre.
#define DATA_PIN 13
#define LATCH_PIN 14
#define CLOCK_PIN 27
#define OE_PIN 4

// Pin utilizzati dal 74HC165 (shift register per gli 8 ingressi digitali)
#define LOAD165_PIN 16
#define CLK165_PIN 17
#define DATA165_PIN 5

#define PWR_LED_PIN 15 // Pin per il LED di alimentazione ("PWR") sulla scheda
#define LED_PIN 2      // Pin per il LED integrato sull'ESP32

class ES32A08 {
public:
  ES32A08();    // Costruttore
  void begin(); // Inizializza la scheda

  float readAnalogmA(int channel);      // Read single 4-20mA input
  float readAnalogVoltage(int channel); // Read single 0-10V input

  bool    readButton(int buttonNumber);       // Read single button state
  bool    readDigitalInput(int inputNumber);  // Read single digital input
  uint8_t readDigitalInputs();                // Read all digital inputs

  void setRelay(int relay, bool state);       // Set single relay
  void setRelays(unsigned long relayStates);  // Set all relays (i.e. 0b00000000)
  void setPWRLED(bool state);                 // Set "PWR" onboard LED

  void reset();         // Open relays and clear display
  void clearDisplay();  // Clear entire 7-seg display
  
  void display(int number);   // Show a number on the 4-digit display
  void display(float number); // Show a float on the 4-digit display
  void display(const char *message); // Show a 4-char message on the display

  void updateDisplay(byte digit, byte segments); // Show custom segment
  
  static const byte digitToSegment[11]; // Dichiarazione dell'array dei segmenti
                                        // corrispondenti alle cifre.
  static const byte digitNumber[8]; // Dichiarazione dell'array di parole a 8
                                    // bit corrispondenti ai digit.

private:
  byte displayBuffer[4]; // Array per memorizzare lo stato dei segmenti per i 4
                         // digit
  byte currentRelays;    // Stato attuale dei relè
  byte currentDigits;    // Digit attualmente selezionato per il display
  byte currentSegments;  // Segmenti attualmente attivati per il display

  uint8_t charToSegments(char c); // Char to segment conversion by LUT
  void sendToShiftRegister();     // Send relays and display settings to the s.r.

  // Dichiarazione dei pin per gli ingressi analogici
  const int mAInputPins[4] = {36, 39, 34, 35};      // In1 a In4 per 4-20mA
  const int voltageInputPins[4] = {32, 33, 25, 26}; // V1 a V4 per 0-10V
  const int buttonPins[4] = {18, 19, 21, 23};       // Onboard buttons 1-4
};

#endif