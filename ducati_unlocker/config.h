#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================
// BOARD SELECTION
// Uncomment ONE of the following:
// #define BOARD_XIAO_S3  // Seeed Studio XIAO ESP32S3
#define BOARD_XIAO_C6  // Seeed Studio XIAO ESP32C6
// #define BOARD_DEVKIT_V1   // ESP32 DevKit V1 (Standard)
// #define BOARD_WAVESHARE_S3 // Waveshare ESP32-S3 (P4/Zero/Pico)
// ============================================

// --- PIN DEFINITIONS ---

#ifdef BOARD_XIAO_C6
  // Seeed Studio XIAO ESP32C6 Pinout
  // D0=GPIO0, D1=GPIO1, D2=GPIO2, D3=GPIO3
  const int PIN_RELAY_1 = 0;   // D0 - Increment (Opto 1)
  const int PIN_RELAY_2 = 1;   // D1 - Confirm (Opto 2)
  
  const int PIN_BUTTON  = 2;   // D2 - Button Input
  const int PIN_BUTTON_LED = 3; // D3 - Button LED
  
  // XIAO C6 Built-in LED is usually defined as LED_BUILTIN (GPIO 15)
  const int STATUS_LED_PIN = LED_BUILTIN; 
#endif

#ifdef BOARD_DEVKIT_V1
  // ESP32 DevKit V1 Pinout
  const int PIN_RELAY_1 = 14;  // D14 - Increment 
  const int PIN_RELAY_2 = 26;  // D26 - Confirm
  // const int PIN_RELAY_3 = 21;  // D21 (Unused)
  // const int PIN_RELAY_4 = 22;  // D22 (Unused)
  
  const int PIN_BUTTON  = 23;  // D23
  const int STATUS_LED_PIN = 2;   // Built-in Blue LED
  const int PIN_BUTTON_LED = 27;  // External Button LED (D27)
#endif

// --- BEHAVIOR SETTINGS ---

// Set to TRUE for Standard Relay Modules (Low Trigger)
// Set to FALSE for Optocouplers / Transistors (High Trigger)
const bool RELAY_ACTIVE_LOW = false;  

// Code (4 digits)
const String DEFAULT_CODE = "0000"; 

// Timing
const int RELAY_PRESS_TIME = 150; 
const int RELAY_RELEASE_TIME = 150;
const int DIGIT_WAIT_TIME = 1000;

// --- BLE SETTINGS (New) ---
#define DEVICE_NAME "Ducati-Key"
#define DEVICE_MANUFACTURER "Ducati DIY"

// Pairing Config
const int PAIRING_WINDOW_MS = 60000; // 60 seconds to pair after Serial Command

// Button Timing
const int DEBOUNCE_DELAY = 50;       // Ignore noise
const int LONG_PRESS_FORCE_MS = 5000; // 5s Hold = Force Unlock (Bypass BLE)

#endif
