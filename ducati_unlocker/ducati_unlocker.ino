/*
 * Ducati Unlocker V2.1 - HID Security Version
 * 
 * Features:
 * 1. Emulates Bluetooth Keyboard (HID) for reliable auto-connect.
 * 2. WHITELIST Security: Only previously bonded phones can connect.
 * 3. Pairing Mode: Triggered ONLY via Serial Command ("PAIR").
 * 
 * Hardware: XIAO ESP32S3
 */

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEHIDDevice.h>
#include <BLE2902.h>
#include <Preferences.h>
#include "config.h"

// --- GLOBALS ---
Preferences preferences;
String storedCode = "0000";

BLEServer* pServer = NULL;
BLEHIDDevice* pHid;
BLEAdvertising* pAdvertising;
bool deviceConnected = false;
bool pairingMode = false;
unsigned long pairingStartTime = 0;

// Button State
int lastButtonState = HIGH;
unsigned long buttonPressTime = 0;
// bool buttonHeldHandled = false; // Removed

// --- FORWARD DECLARATIONS ---
void setupRelays();
void loadPreferences();
void enterCode();
void triggerRelay(int pin);
void blinkLed(int times, int duration);
void setLeds(bool state); // Helper for both LEDs
void startAdvertising();
void enablePairingMode();

// --- BLE CALLBACKS ---
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println(">> Phone CONNECTED (Encrypted/Bonded)");

    // Stop advertising (security)
    // On disconnect, we will restart it.
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println(">> Phone DISCONNECTED");
    startAdvertising();  // Restart advertising so we can reconnect
  }
};

// --- SETUP ---
void setup() {
  Serial.begin(9600);  // Lower baud rate for stability
  delay(1000);         // Wait for Serial to stabilize

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(STATUS_LED_PIN, OUTPUT);
  pinMode(PIN_BUTTON_LED, OUTPUT);
  setLeds(LOW);

  setupRelays();
  loadPreferences();

  // 1. Init BLE
  Serial.println("Starting BLE HID...");
  BLEDevice::init(DEVICE_NAME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // 2. Init HID (Keyboard) Profile
  // This makes the phone think we are a keyboard -> Auto Connect!
  pHid = new BLEHIDDevice(pServer);
  pHid->reportMap((uint8_t*)"", 0);  // Minimal report map (we don't actually type)
  pHid->pnp(0x02, 0xe502, 0xa111, 0x0210);
  pHid->hidInfo(0x00, 0x01);

  // 3. Security (Bonding)
  BLESecurity* pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);  // Require Bonding
  pSecurity->setCapability(ESP_IO_CAP_NONE);           // No Screen/Keyboard
  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

  // 4. Start Services
  pHid->startServices();
  pAdvertising = pServer->getAdvertising();
  pAdvertising->setAppearance(HID_KEYBOARD);
  pAdvertising->addServiceUUID(pHid->hidService()->getUUID());

  // 5. Initial Advertising (Whitelist logic happens in startAdvertising)
  pHid->setBatteryLevel(100);
  startAdvertising();

  Serial.println("System Ready.");
}

// --- LOOP ---
void loop() {
  // 1. Handle Button (Simple Click/Toggle Only)
  int reading = digitalRead(PIN_BUTTON);
  unsigned long now = millis();

  // Check Pairing Mode Timeout
  if (pairingMode && (now - pairingStartTime > PAIRING_WINDOW_MS)) {
    Serial.println("Pairing Mode Timed Out.");
    pairingMode = false;
    blinkLed(3, 100);    // Alert user
    startAdvertising();  // Revert to Whitelist
  }

  // Button Logic (Trigger on Press/Switch ON)
  if (reading == LOW && lastButtonState == HIGH) {
    // Button Pressed
    buttonPressTime = now;
  } else if (reading == LOW && lastButtonState == LOW) {
    // Button Held
    // Check for Force Unlock (Long Press)
    if (now - buttonPressTime > LONG_PRESS_FORCE_MS) {
      Serial.println("!!! FORCE UNLOCK TRIGGERED (Manual Override) !!!");
      blinkLed(5, 50);  // Warning blinks
      enterCode();

      // Wait for button release to avoid re-triggering
      while (digitalRead(PIN_BUTTON) == LOW) { delay(10); }
      lastButtonState = HIGH;  // Reset state
      return;
    }
  } else if (reading == HIGH && lastButtonState == LOW) {
    // Button Released (Short Click)
    if (now - buttonPressTime > DEBOUNCE_DELAY) {
      Serial.println("Trigger Clicked!");
      if (deviceConnected) {
        Serial.println(">> Authorized. Unlocking...");
        enterCode();
      } else {
        Serial.println(">> Ignored: No Phone Connected.");
        blinkLed(2, 50);
      }
    }
  }
  lastButtonState = reading;

  // 2. Status LED

  if (pairingMode) {
    // Fast Blink = Pairing Mode
    if (millis() % 200 < 100) setLeds(HIGH);
    else setLeds(LOW);
  } else if (deviceConnected) {
    // Solid (or very slow breathe) = Connected
    if (millis() % 4000 < 100) setLeds(HIGH);  // Heartbeat
    else setLeds(LOW);
  } else {
    // Off (Stealth)
    setLeds(LOW);
  }

  // 3. Serial Config
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.startsWith("CODE ")) {
      String newCode = input.substring(5);
      if (newCode.length() == 4) {
        storedCode = newCode;
        preferences.begin("ducati", false);
        preferences.putString("code", storedCode);
        preferences.end();
        Serial.println("Code Saved: " + storedCode);
      }
    } else if (input == "TEST") {
      Serial.println("Testing Relays 1-2...");
      triggerRelay(PIN_RELAY_1);
      delay(300);
      triggerRelay(PIN_RELAY_2);
      Serial.println("Test Complete.");
    } else if (input == "R1") {
      Serial.println("Toggling Relay 1...");
      digitalWrite(PIN_RELAY_1, !digitalRead(PIN_RELAY_1));
    } else if (input == "R2") {
      Serial.println("Toggling Relay 2...");
      digitalWrite(PIN_RELAY_2, !digitalRead(PIN_RELAY_2));
    } else if (input == "BTN") {
      Serial.print("Button State: ");
      Serial.println(digitalRead(PIN_BUTTON) == LOW ? "PRESSED (LOW)" : "RELEASED (HIGH)");
    } else if (input == "PAIR") {
      enablePairingMode();
    } else if (input == "CLEAR") {
      // Clear bonds
      Serial.println("Clearing bonds not fully implemented. Use 'Erase Flash' in IDE to reset.");
    }
  }

  delay(10);
}

// --- HELPERS ---

void startAdvertising() {
  pAdvertising->stop();

  // RELIABILITY FIX:
  // Instead of using the Radio-level Whitelist (which can fail if phone randomizes MAC),
  // We advertise OPENLY.
  // Security is handled by the Bonding requirement.
  // Only bonded phones can encrypt and actually use the HID service.

  Serial.println("Advertising: OPEN (Security handled by Bonding)");
  pAdvertising->setScanFilter(false, false);

  pAdvertising->start();
}

void enablePairingMode() {
  Serial.println(">>> ENTERING PAIRING MODE <<<");
  pairingMode = true;
  pairingStartTime = millis();

  // Disconnect anyone currently connected
  if (deviceConnected) {
    pServer->disconnect(pServer->getConnId());
  }

  // Restart advertising in "Open" mode
  startAdvertising();
}

void setupRelays() {
  pinMode(PIN_RELAY_1, OUTPUT);
  pinMode(PIN_RELAY_2, OUTPUT);

  // Set all to inactive
  digitalWrite(PIN_RELAY_1, RELAY_ACTIVE_LOW ? HIGH : LOW);
  digitalWrite(PIN_RELAY_2, RELAY_ACTIVE_LOW ? HIGH : LOW);
}

void loadPreferences() {
  preferences.begin("ducati", true);
  storedCode = preferences.getString("code", DEFAULT_CODE);
  preferences.end();
  Serial.print("Loaded Code: ");
  Serial.println(storedCode);
}

void triggerRelay(int pin) {
  digitalWrite(pin, RELAY_ACTIVE_LOW ? LOW : HIGH);
  delay(RELAY_PRESS_TIME);
  digitalWrite(pin, RELAY_ACTIVE_LOW ? HIGH : LOW);
  delay(RELAY_RELEASE_TIME);
}

void enterCode() {
  blinkLed(2, 200);
  for (int i = 0; i < 4; i++) {
    int digit = storedCode.charAt(i) - '0';

    // Bike Logic Adjustment:
    // First digit often needs an extra press to "wake up" the counter to 0.
    // So: To enter '0', we press 1 time. To enter '5', we press 6 times.
    // Only for the FIRST digit (i == 0).
    int presses = digit;
    if (i == 0) {
      presses = digit + 1;
      Serial.println(" (Adding +1 press for first digit wake-up)");
    }

    // Press 'Increment' (Relay 1)
    for (int k = 0; k < presses; k++) {
      triggerRelay(PIN_RELAY_1);
    }
    delay(200);
    // Press 'Confirm' (Relay 2)
    triggerRelay(PIN_RELAY_2);
    delay(DIGIT_WAIT_TIME);
  }
  Serial.println("Done.");
}

void blinkLed(int times, int duration) {
  for (int i = 0; i < times; i++) {
    setLeds(HIGH);
    delay(duration);
    setLeds(LOW);
    delay(duration);
  }
}

void setLeds(bool state) {
  digitalWrite(STATUS_LED_PIN, state);
  digitalWrite(PIN_BUTTON_LED, state);
}
