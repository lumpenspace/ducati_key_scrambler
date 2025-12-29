/*
 * Ducati Scrambler Code Bruteforce
 * 
 * Automatically try codes from 0000-9999
 * 
 * See config.h to select your board and adjust settings.
 * See README.md for wiring instructions.
 */

#include "config.h"
#include "relay_utils.h"

// State
int currentCode = 0;
bool bruteforceRunning = false;
bool codeFound = false;

void setup() {
  setupSerial();
  setupRelaysBruteforce();
  
  printMenu();
}

void printMenu() {
  Serial.println();
  Serial.println("==========================================");
  Serial.println("  Ducati Scrambler Code Bruteforce");
  Serial.println("==========================================");
  Serial.println();
  Serial.println("Commands:");
  Serial.println("  START      - Begin bruteforce from current code");
  Serial.println("  STOP       - Abort bruteforce");
  Serial.println("  0000-9999  - Set starting code");
  Serial.println("  STATUS     - Show current state");
  Serial.println("  TEST       - Test all relays");
  Serial.println("  TESTON     - Turn power ON (for testing)");
  Serial.println("  TESTOFF    - Turn power OFF (for testing)");
  Serial.println();
  Serial.print("Current starting code: ");
  Serial.println(formatCode(currentCode));
  Serial.println();
}

void testRelays() {
  Serial.println("Testing relays...");
  
  Serial.println("  INCREASE relay...");
  pulse(PIN_INCREASE);
  delay(500);
  
  Serial.println("  MOVE relay...");
  pulse(PIN_MOVE);
  delay(500);
  
  Serial.println("  POWER relay...");
  pulse(PIN_POWER);
  delay(500);
  
  Serial.println("Relay test complete!");
}

void bruteforce() {
  bruteforceRunning = true;
  codeFound = false;
  
  Serial.println();
  Serial.println("==========================================");
  Serial.println("  BRUTEFORCE STARTED");
  Serial.print("  Starting from code: ");
  Serial.println(formatCode(currentCode));
  Serial.println("  Type 'STOP' to abort");
  Serial.println("==========================================");
  Serial.println();
  
  while (bruteforceRunning && currentCode <= 9999) {
    // Step 1: Turn on main power
    powerOn();
    
    // Step 2: Try CODE_ATTEMPTS codes per power cycle
    for (int attempt = 0; attempt < CODE_ATTEMPTS && bruteforceRunning; attempt++) {
      
      // Check if we've exceeded max code
      if (currentCode > 9999) {
        Serial.println();
        Serial.println("ERROR: Reached 9999 without finding code!");
        bruteforceRunning = false;
        break;
      }
      
      // Enter the code
      enterCodeInt(currentCode, false);
      
      // Check if code was accepted
      delay(500);
      
      if (isCodeAccepted()) {
        // SUCCESS!
        codeFound = true;
        bruteforceRunning = false;
        
        Serial.println();
        Serial.println("==========================================");
        Serial.println("  *** CODE FOUND! ***");
        Serial.print("  The code is: ");
        Serial.println(formatCode(currentCode));
        Serial.println("==========================================");
        Serial.println();
        
        powerOff();
        return;
      }
      
      // Code didn't work, try next
      currentCode++;
      
      // Check for serial commands (allow abort)
      if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        input.toUpperCase();
        if (input == "STOP") {
          Serial.println();
          Serial.println("BRUTEFORCE ABORTED BY USER");
          bruteforceRunning = false;
          powerOff();
          return;
        }
      }
    }
    
    // Step 3: Turn off power and wait
    if (bruteforceRunning) {
      powerOff();
      
      // Progress update
      Serial.print("  Progress: ");
      Serial.print(currentCode);
      Serial.println(" / 10000");
    }
  }
  
  if (!codeFound && currentCode > 9999) {
    Serial.println();
    Serial.println("==========================================");
    Serial.println("  BRUTEFORCE COMPLETE - NO CODE FOUND");
    Serial.println("  Check wiring or detection method");
    Serial.println("==========================================");
  }
  
  bruteforceRunning = false;
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    String upperInput = input;
    upperInput.toUpperCase();
    
    if (upperInput == "START") {
      bruteforce();
      printMenu();
    }
    else if (upperInput == "STOP") {
      bruteforceRunning = false;
      Serial.println("Stopped.");
    }
    else if (upperInput == "STATUS") {
      Serial.print("Current code: ");
      Serial.println(formatCode(currentCode));
      Serial.print("Bruteforce running: ");
      Serial.println(bruteforceRunning ? "YES" : "NO");
    }
    else if (upperInput == "TEST") {
      testRelays();
    }
    else if (upperInput == "TESTON") {
      Serial.println("Turning power ON...");
      digitalWrite(PIN_POWER, RELAY_ON);
    }
    else if (upperInput == "TESTOFF") {
      Serial.println("Turning power OFF...");
      digitalWrite(PIN_POWER, RELAY_OFF);
    }
    else if (isValidCode(input)) {
      currentCode = input.toInt();
      Serial.print("Starting code set to: ");
      Serial.println(formatCode(currentCode));
    }
    else {
      Serial.println("Invalid input. Type START, STOP, STATUS, TEST, or a 4-digit code.");
    }
  }
}
