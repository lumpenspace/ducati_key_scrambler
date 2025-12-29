/*
 * Ducati Scrambler Bluetooth Fob - Configuration
 * 
 * A permanent Bluetooth key fob replacement for your Ducati.
 * Only needs 2 relays (INCREASE + MOVE) — no power cycling required.
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

// Option 3: Generic ESP32 (with BLE support)
// #define BOARD_ESP32_GENERIC

// Option 4: SparkFun Thing Plus ESP32-S3 (has LiPo support!)
// #define BOARD_SPARKFUN_THING_PLUS

// Option 5: Adafruit Feather ESP32-S3 (has LiPo support!)
// #define BOARD_ADAFRUIT_FEATHER

// ============================================
// PIN DEFINITIONS
// ============================================

#if defined(BOARD_ESP32S3_WAVESHARE)
  const int PIN_INCREASE = 43;   // TXD on UART port
  const int PIN_MOVE = 44;       // RXD on UART port
  #define SERIAL_BAUD 115200

#elif defined(BOARD_ESP32P4)
  const int PIN_INCREASE = 4;
  const int PIN_MOVE = 5;
  #define SERIAL_BAUD 115200

#elif defined(BOARD_ESP32_GENERIC)
  const int PIN_INCREASE = 16;
  const int PIN_MOVE = 17;
  #define SERIAL_BAUD 115200

#elif defined(BOARD_SPARKFUN_THING_PLUS)
  const int PIN_INCREASE = 5;
  const int PIN_MOVE = 6;
  #define SERIAL_BAUD 115200

#elif defined(BOARD_ADAFRUIT_FEATHER)
  const int PIN_INCREASE = 5;
  const int PIN_MOVE = 6;
  #define SERIAL_BAUD 115200

#else
  #error "Please select a board in config.h"
#endif

// ============================================
// TIMING (for button simulation)
// ============================================

const int PULSE_DURATION = 150;   // How long to "press" each button (ms)
const int PULSE_GAP = 150;        // Gap between button presses (ms)
const int DIGIT_DELAY = 300;      // Delay after moving to next digit (ms)

// ============================================
// RELAY LOGIC
// ============================================

// Most relay modules are active LOW (pull low to activate)
// Swap these if your relays work backwards
const int RELAY_ON = LOW;
const int RELAY_OFF = HIGH;

// ============================================
// BLUETOOTH SETTINGS
// ============================================

// RSSI threshold for "nearby" detection
// -50 dBm = very close (~1 meter)
// -70 dBm = nearby (~3-5 meters)
// -90 dBm = far away (~10+ meters)
const int BLE_RSSI_THRESHOLD = -70;

// How long to scan for the device (seconds)
const int BLE_SCAN_DURATION = 3;

// Delay between scans when NOT in deep sleep mode (ms)
const unsigned long SCAN_INTERVAL_MS = 5000;  // 5 seconds

// Cooldown after entering code (don't spam it)
const unsigned long CODE_COOLDOWN_MS = 30000;  // 30 seconds

// ============================================
// POWER DETECTION (Optional)
// ============================================

// Uncomment to enable bike power detection
// Only enters code when phone is nearby AND bike is on
// #define FOB_POWER_DETECT_ENABLED

#if defined(FOB_POWER_DETECT_ENABLED)
  #if defined(BOARD_ESP32S3_WAVESHARE)
    const int PIN_POWER_DETECT = 2;   // SDA on I2C port
  #elif defined(BOARD_ESP32P4)
    const int PIN_POWER_DETECT = 7;
  #elif defined(BOARD_ESP32_GENERIC)
    const int PIN_POWER_DETECT = 19;
  #elif defined(BOARD_SPARKFUN_THING_PLUS)
    const int PIN_POWER_DETECT = 9;
  #elif defined(BOARD_ADAFRUIT_FEATHER)
    const int PIN_POWER_DETECT = 9;
  #endif
#endif

// ============================================
// DEEP SLEEP (for LiPo battery operation)
// ============================================

// Uncomment to enable deep sleep between scans
// This DRAMATICALLY reduces power consumption for battery operation
// #define FOB_DEEP_SLEEP_ENABLED

// How long to sleep between scans (seconds)
// Trade-off: longer sleep = better battery life, slower unlock response
//
// | Sleep Duration | Response Time | ~Battery Life (1000mAh) |
// |----------------|---------------|-------------------------|
// | 5 seconds      | ~5 sec        | ~1 day                  |
// | 30 seconds     | ~30 sec       | ~5 days                 |
// | 60 seconds     | ~1 min        | ~10 days                |
//
const int DEEP_SLEEP_SECONDS = 5;

// ============================================
// BATTERY MONITORING (Optional)
// ============================================

// Uncomment if your board has a battery voltage pin (like SparkFun/Adafruit boards)
// #define FOB_BATTERY_MONITOR_ENABLED

#if defined(FOB_BATTERY_MONITOR_ENABLED)
  #if defined(BOARD_SPARKFUN_THING_PLUS)
    const int PIN_BATTERY = 34;  // Battery voltage divider
  #elif defined(BOARD_ADAFRUIT_FEATHER)
    const int PIN_BATTERY = A13; // Built-in battery monitor
  #else
    const int PIN_BATTERY = 35;  // Define your own
  #endif
  
  // Voltage thresholds (adjust based on your voltage divider)
  const float BATTERY_FULL = 4.2;     // 100%
  const float BATTERY_LOW = 3.3;      // ~10% - warn user
  const float BATTERY_CRITICAL = 3.0; // ~0% - shutdown to protect battery
#endif

#endif // CONFIG_H

