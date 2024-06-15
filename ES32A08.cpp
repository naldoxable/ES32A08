#include "ES32A08.h"

// Initialize the board and reset it
void ES32A08::begin() {
  // 74HC595D shift register config (Relay + 7-Seg disp.)
  pinMode(DATA_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(OE_PIN, OUTPUT);

  digitalWrite(DATA_PIN, LOW);
  digitalWrite(LATCH_PIN, LOW);
  digitalWrite(CLOCK_PIN, LOW);
  digitalWrite(OE_PIN, LOW);

  pinMode(PWR_LED_PIN, OUTPUT); // "PWR" board Led

  pinMode(buttonPins[0], INPUT_PULLUP);
  pinMode(buttonPins[1], INPUT_PULLUP);
  pinMode(buttonPins[2], INPUT_PULLUP);
  pinMode(buttonPins[3], INPUT_PULLUP);

  // Digital inputs shift register config
  pinMode(LOAD165_PIN, OUTPUT);
  pinMode(CLK165_PIN, OUTPUT);
  pinMode(DATA165_PIN, INPUT);
  digitalWrite(LOAD165_PIN, HIGH);

  reset();

  xTaskCreatePinnedToCore(ES32A08::updateRegisters, "Update Registers",
                          2048, // Stack size
                          this, // Parameter
                          1,    // Priority
                          NULL, // Task handle
                          1     // Core on which the task will run
  );
}

// Open relays and clear display
void ES32A08::reset() {
  setPWRLED(RESET_LED_ON);
  currentRelays = RESET_RELAY_ON ? 0b11111111 : 0b00000000;
  clearDisplay(); // it also calls sendToShiftRegister()
}

// Send relays and display settings to the s.r.
void ES32A08::sendToShiftRegister() {
  digitalWrite(LATCH_PIN, LOW);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, currentRelays);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, currentDigits);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, currentSegments);
  digitalWrite(LATCH_PIN, HIGH);
  delayMicroseconds(DIGIT_PERS);
} // There are 3 shift registers for a total of 24 bits

// Clear entire 7-seg display
void ES32A08::clearDisplay() {
  memset(displayBuffer, 0, sizeof(displayBuffer));
}

// Task to continuosly feed the registers
void ES32A08::updateRegisters(void *instance) {
  ES32A08 *self = static_cast<ES32A08 *>(instance); // Cast del puntatore
  for (;;) {
    for (uint8_t pos = 0; pos < 4; pos++) {
      self->currentDigits = self->digitNumber[1 + pos];
      self->currentSegments = self->displayBuffer[pos];
      self->sendToShiftRegister();
    }
  }
}

// Char to segment conversion by LUT
uint8_t ES32A08::charToSegments(char c) {
  switch (c) {
  case '0':
    return 0b00111111; // 0
  case '1':
    return 0b00000110; // 1
  case '2':
    return 0b01011011; // 2
  case '3':
    return 0b01001111; // 3
  case '4':
    return 0b01100110; // 4
  case '5':
    return 0b01101101; // 5
  case '6':
    return 0b01111101; // 6
  case '7':
    return 0b00000111; // 7
  case '8':
    return 0b01111111; // 8
  case '9':
    return 0b01101111; // 9
  case ' ':
    return 0b00000000; // white space
  case '.':
    return 0b10000000; // decimal point
  case '-':
    return 0b01000000; // score
  case '_':
    return 0b00001000; // underscore
  case 'A':
    return 0b01110111; // A
  case 'a':
    return 0b01011111; // a
  case 'B':
  case 'b':
    return 0b01111100; // B-b
  case 'C':
    return 0b00111001; // C
  case 'c':
    return 0b01011000; // c
  case 'D':
  case 'd':
    return 0b01011110; // D-d
  case 'E':
    return 0b01111001; // E
  case 'e':
    return 0b01111011; // e
  case 'F':
  case 'f':
    return 0b01110001; // F-f
  case 'G':
  case 'g':
    return 0b01101111; // G-g
  case 'H':
    return 0b01110110; // H
  case 'h':
    return 0b01110100; // h
  case 'I':
    return 0b00000110; // I
  case 'i':
    return 0b00010000; // i
  case 'J':
  case 'j':
    return 0b00011110; // J-j
  case 'L':
    return 0b00111000; // L
  case 'l':
    return 0b00011000; // l
  case 'M':
  case 'm':
    return 0b00110111; // M-m
  case 'N':
  case 'n':
    return 0b01010100; // N-n
  case 'O':
  case 'o':
    return 0b01011100; // O-o
  case 'P':
  case 'p':
    return 0b01110011; // P-p
  case 'Q':
  case 'q':
    return 0b01100111; // Q-q
  case 'R':
  case 'r':
    return 0b01010000; // R-r
  case 'S':
  case 's':
    return 0b01101101; // S-s
  case 'T':
  case 't':
    return 0b01111000; // T-t
  case 'U':
    return 0b00111110; // U
  case 'u':
    return 0b00011100; // u
  case 'Y':
  case 'y':
    return 0b01100110; // Y-y
  case 'Z':
  case 'z':
    return 0b01011011; // Z-z

  default:
    return 0b00000000; // Blank if not found
  }
}

// Show a 4-char message on the display
void ES32A08::display(const char *message) {
  memset(displayBuffer, 0, sizeof(displayBuffer));
  for (int pos = 0; message[pos] != '\0' && pos < 4; pos++)
    displayBuffer[pos] = charToSegments(message[pos]);
}

// Show a number on the 4-digit display
void ES32A08::display(int number) {
  if (number > 9999 || number < -999)
    display(" -- ");
  else {
    char buffer[5]; // Chars buffer + endl ('\0')
    snprintf(buffer, sizeof(buffer), "%4d", number);

    for (int i = 0; i < 4; i++)
      displayBuffer[i] = charToSegments(buffer[i]);
  }
}

// Show a float on the 4-digit display
void ES32A08::display(float number) {
  memset(displayBuffer, 0, sizeof(displayBuffer));
  if (number > 9999 || number < -999)
    display(" -- ");
  else {
    char buffer[6]; // Chars buffer + endl ('\0')
    snprintf(buffer, sizeof(buffer), "%.2f", number);

    int i;
    for (i = 0; i < 4 && buffer[i] != '.'; i++) {
      displayBuffer[i] = charToSegments(buffer[i]);
    }
    if (i == 4)
      return; // No decimal point found

    if (buffer[i] == '.') {
      displayBuffer[i - 1] |= 0b10000000; // Decimal point

      for (i; i < 5; i++) {
        displayBuffer[i] = charToSegments(buffer[i + 1]);
      }
    }
  }
}

// Read single 4-20mA input
float ES32A08::readAnalogmA(int channel) {
  if (channel < 0 || channel > 3)
    return 0;
  int adcValue = analogRead(mAInputPins[channel]);
  float current = ((adcValue / 4095.0) * (20.0 - 4.0)) * 2.0692;
  return current;
}

// Read single 0-10V input
float ES32A08::readAnalogVoltage(int channel) {
  if (channel < 0 || channel > 3)
    return 0;
  int adcValue = analogRead(voltageInputPins[channel]);
  float voltage = ((adcValue / 4095.0) * 10.0) * 2.0692;
  return voltage;
}

// Raw read single 0-10V input
int ES32A08::rawReadAnalogVoltage(int channel) {
  if (channel < 0 || channel > 3)
    return 0;
  return analogRead(voltageInputPins[channel]);
}

// Set single relay
void ES32A08::setRelay(int relay, bool state) {
  bitWrite(currentRelays, relay, state);
  sendToShiftRegister();
}

// Set all relays (0b00000000 - 0b11111111)
void ES32A08::setRelays(unsigned long relayStates) {
  currentRelays = relayStates;
  sendToShiftRegister();
}

// Read all digital inputs
uint8_t ES32A08::readDigitalInputs() {
  uint8_t inputs = 0;
  digitalWrite(LOAD165_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(LOAD165_PIN, HIGH);
  // delayMicroseconds(5);

  for (int i = 0; i < 8; i++) {
    digitalWrite(CLK165_PIN, LOW); // Init clock cycle
    delayMicroseconds(5);
    bitWrite(inputs, 7 - i, digitalRead(DATA165_PIN));
    digitalWrite(CLK165_PIN, HIGH);
    // delayMicroseconds(5);
  }

  return inputs;
}

// Read single digital input
bool ES32A08::readDigitalInput(int inputNumber) {
  if (inputNumber < 1 || inputNumber > 8)
    return false;
  uint8_t digitalInputs = readDigitalInputs();
  return digitalInputs & (1 << (inputNumber - 1));
}

// Set "PWR" onboard LED
void ES32A08::setPWRLED(bool state) { digitalWrite(PWR_LED_PIN, !state); }

// Read single button state
bool ES32A08::readButton(int buttonNumber) {
  return !digitalRead(buttonPins[buttonNumber - 1]);
}