/*
 * Ducati Scrambler Code Entry - Relay Utilities
 * 
 * Common functions for relay control
 */

#ifndef RELAY_UTILS_H
#define RELAY_UTILS_H

#include "config.h"

// ============================================
// RELAY CONTROL FUNCTIONS
// ============================================

void setupRelays() {
  pinMode(PIN_INCREASE, OUTPUT);
  pinMode(PIN_MOVE, OUTPUT);
  
  digitalWrite(PIN_INCREASE, RELAY_OFF);
  digitalWrite(PIN_MOVE, RELAY_OFF);
}

void setupRelaysBruteforce() {
  setupRelays();
  
  pinMode(PIN_POWER, OUTPUT);
  pinMode(PIN_CODE_CHECK, INPUT);
  
  digitalWrite(PIN_POWER, RELAY_OFF);
}

void pulse(int pin) {
  digitalWrite(pin, RELAY_ON);
  delay(PULSE_DURATION);
  digitalWrite(pin, RELAY_OFF);
  delay(PULSE_GAP);
}

void pulseIncrease() {
  pulse(PIN_INCREASE);
}

void pulseMove() {
  pulse(PIN_MOVE);
}

void powerOn() {
  Serial.println("  [POWER ON]");
  digitalWrite(PIN_POWER, RELAY_ON);
  delay(POWER_ON_DELAY);
}

void powerOff() {
  Serial.println("  [POWER OFF]");
  digitalWrite(PIN_POWER, RELAY_OFF);
  delay(POWER_OFF_DELAY);
}

bool isCodeAccepted() {
  return digitalRead(PIN_CODE_CHECK) == HIGH;
}

// ============================================
// CODE ENTRY FUNCTIONS
// ============================================

String formatCode(int code) {
  String s = String(code);
  while (s.length() < 4) {
    s = "0" + s;
  }
  return s;
}

void enterCode(String codeStr, bool verbose = true) {
  if (verbose) {
    Serial.print("Entering code: ");
    Serial.println(codeStr);
    Serial.println();
    Serial.println("Starting code entry...");
  } else {
    Serial.print("  Trying code: ");
    Serial.println(codeStr);
  }
  
  // Initial pulse to start code entry (moves display to 0)
  pulseIncrease();
  delay(DIGIT_DELAY);
  
  // Process each digit
  for (int i = 0; i < 4; i++) {
    int digit = codeStr.charAt(i) - '0';
    
    if (verbose) {
      Serial.print("Digit ");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(digit);
      Serial.print(" -> pulsing INCREASE ");
      Serial.print(digit);
      Serial.println(" times");
    }
    
    // Pulse increase 'digit' times
    for (int j = 0; j < digit; j++) {
      pulseIncrease();
    }
    
    // Pulse move cursor once
    if (verbose) {
      Serial.println("       -> pulsing MOVE once");
    }
    pulseMove();
    delay(DIGIT_DELAY);
  }
  
  // Final MOVE pulse to submit
  if (verbose) {
    Serial.println("Submitting code...");
  }
  pulseMove();
  delay(DIGIT_DELAY);
  
  if (verbose) {
    Serial.println();
    Serial.println("Code entry complete!");
  }
}

void enterCodeInt(int code, bool verbose = true) {
  enterCode(formatCode(code), verbose);
}

// ============================================
// SERIAL SETUP
// ============================================

void setupSerial() {
  Serial.begin(SERIAL_BAUD);
  
  #if WAIT_FOR_SERIAL
  while (!Serial) {
    delay(10);
  }
  delay(2000);
  #endif
}

// ============================================
// INPUT VALIDATION
// ============================================

bool isValidCode(String input) {
  if (input.length() != 4) {
    return false;
  }
  
  for (int i = 0; i < 4; i++) {
    if (input.charAt(i) < '0' || input.charAt(i) > '9') {
      return false;
    }
  }
  
  return true;
}

#endif // RELAY_UTILS_H



