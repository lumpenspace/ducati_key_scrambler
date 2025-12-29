/*
 * Ducati Scrambler Code Entry
 * 
 * Single code entry mode - enter a 4-digit code via Serial Monitor
 * 
 * See config.h to select your board and adjust settings.
 * See README.md for wiring instructions.
 */

#include "config.h"
#include "relay_utils.h"

void setup() {
  setupSerial();
  setupRelays();
  
  Serial.println();
  Serial.println("=================================");
  Serial.println("  Ducati Scrambler Code Entry");
  Serial.println("=================================");
  Serial.println();
  Serial.println("Enter your 4-digit code and press Enter:");
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (!isValidCode(input)) {
      Serial.println("Error: Please enter exactly 4 digits (0-9)");
      return;
    }
    
    enterCode(input, true);
    
    Serial.println();
    Serial.println("Enter another code or reset to try again:");
  }
}

