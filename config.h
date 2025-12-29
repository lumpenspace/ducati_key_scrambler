/*
 * Ducati Scrambler Code Entry - Configuration
 * 
 * Edit this file to match your board and wiring
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// BOARD SELECTION - Uncomment ONE option
// ============================================

// Option 1: Waveshare ESP32-S3-Touch-LCD-2.8
#define BOARD_ESP32S3_WAVESHARE

// Option 2: ESP32-P4-WiFi6 Dev Kit
// #define BOARD_ESP32P4

// Option 3: Arduino Uno/Nano
// #define BOARD_ARDUINO_UNO

// Option 4: Generic ESP32
// #define BOARD_ESP32_GENERIC

// ============================================
// PIN DEFINITIONS (auto-configured by board)
// ============================================

#if defined(BOARD_ESP32S3_WAVESHARE)
  // Waveshare ESP32-S3-Touch-LCD-2.8 - using UART + I2C ports
  const int PIN_INCREASE = 43;   // TXD on UART port
  const int PIN_MOVE = 44;       // RXD on UART port
  const int PIN_POWER = 1;       // SCL on I2C port (bruteforce only)
  const int PIN_CODE_CHECK = 2;  // SDA on I2C port (bruteforce only)
  #define SERIAL_BAUD 115200
  #define WAIT_FOR_SERIAL true

#elif defined(BOARD_ESP32P4)
  // ESP32-P4-WiFi6 Dev Kit
  const int PIN_INCREASE = 4;
  const int PIN_MOVE = 5;
  const int PIN_POWER = 6;       // Bruteforce only
  const int PIN_CODE_CHECK = 7;  // Bruteforce only
  #define SERIAL_BAUD 115200
  #define WAIT_FOR_SERIAL true

#elif defined(BOARD_ARDUINO_UNO)
  // Arduino Uno/Nano
  const int PIN_INCREASE = 2;
  const int PIN_MOVE = 3;
  const int PIN_POWER = 4;       // Bruteforce only
  const int PIN_CODE_CHECK = 5;  // Bruteforce only
  #define SERIAL_BAUD 9600
  #define WAIT_FOR_SERIAL false

#elif defined(BOARD_ESP32_GENERIC)
  // Generic ESP32
  const int PIN_INCREASE = 16;
  const int PIN_MOVE = 17;
  const int PIN_POWER = 18;      // Bruteforce only
  const int PIN_CODE_CHECK = 19; // Bruteforce only
  #define SERIAL_BAUD 115200
  #define WAIT_FOR_SERIAL true

#else
  #error "Please select a board in config.h"
#endif

// ============================================
// TIMING CONFIGURATION
// ============================================

const int PULSE_DURATION = 150;   // How long to hold each pulse (ms)
const int PULSE_GAP = 150;        // Gap between pulses (ms)
const int DIGIT_DELAY = 300;      // Delay after moving to next digit (ms)

// Bruteforce timing
const int POWER_ON_DELAY = 2000;  // Wait after turning power on (ms)
const int POWER_OFF_DELAY = 1000; // Wait after turning power off (ms)
const int CODE_ATTEMPTS = 3;      // Number of code attempts per power cycle

// ============================================
// RELAY LOGIC
// ============================================

// Most relay modules are active LOW (pull low to activate)
// Change to HIGH if your relay module is active HIGH
const int RELAY_ON = LOW;
const int RELAY_OFF = HIGH;

#endif // CONFIG_H

