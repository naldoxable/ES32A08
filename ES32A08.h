#ifndef ES32A08_h
#define ES32A08_h

#include "Arduino.h"

// ######### BOARD CONFIG REGION ##########################################

#define PWR_LED_PIN 15 // "PWR" board Led

// 74HC595D shift register pins (Relay + 4x7-Seg disp.)
#define DATA_PIN 13
#define LATCH_PIN 14
#define CLOCK_PIN 27
#define OE_PIN 4

// 74HC165 shift register pins (Digital inputs)
#define LOAD165_PIN 16
#define CLK165_PIN 17
#define DATA165_PIN 5

// ######### USER CONFIG REGION ###########################################

#define RESET_RELAY_ON false // State of the relays at boot
#define RESET_LED_ON true    // State of the "PWR" LED at boot
#define DIGIT_PERS 1000      // uS to hold the digit lit (1000=1ms default)

// ######### CLASS DEFINITION REGION #####################################

class ES32A08 {
public:
  void begin(); // Initialize the board and reset it

  float readAnalogmA(int channel);       // Read single 4-20mA input
  float readAnalogVoltage(int channel);  // Read single 0-10V input
  int rawReadAnalogVoltage(int channel); // Raw read single 0-10V input

  bool readButton(int buttonNumber);      // Read single button state
  bool readDigitalInput(int inputNumber); // Read single digital input
  uint8_t readDigitalInputs();            // Read all digital inputs

  void setRelay(int relay, bool state);      // Set single relay
  void setRelays(unsigned long relayStates); // Set all relays (i.e. 0b00000000)
  void setPWRLED(bool state);                // Set "PWR" onboard LED

  void reset();        // Open relays and clear display
  void clearDisplay(); // Clear entire 7-seg display

  void display(int number);          // Show a number on the 4-digit display
  void display(float number);        // Show a float on the 4-digit display
  void display(const char *message); // Show a 4-char message on the display

  static void
  updateRegisters(void *instance); // Task to continuosly feed the registers

private:
  byte displayBuffer[4]; // Buffer for the 4-digit display
  byte currentRelays;    // Relays status
  byte currentDigits;    // Selected digit(s)
  byte currentSegments;  // Segments to display

  uint8_t charToSegments(char c); // Char to segment conversion by LUT
  void sendToShiftRegister(); // Send relays and display settings to the s.r.

  // Pin definitions
  const int mAInputPins[4] = {36, 39, 34, 35};      // In1 a In4 for 4-20mA
  const int voltageInputPins[4] = {32, 33, 25, 26}; // V1 a V4 for 0-10V
  const int buttonPins[4] = {18, 19, 21, 23};       // Onboard buttons 1-4

  // 7-segment display digit selection
  const byte digitNumber[8] = {
      0b11111111, // none.
      0b11111110, // digit 1.
      0b11111101, // digit 2.
      0b11111011, // digit 3.
      0b11110111, // digit 4.
      0b11111100, // digit 1+2.
      0b11111000, // digit 1+2+3.
      0b11110000  // digit 1+2+3+4.
  };
};

#endif