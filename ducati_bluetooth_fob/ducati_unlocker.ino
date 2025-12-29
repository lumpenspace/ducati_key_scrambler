/*
 * Ducati Scrambler Bluetooth Unlocker
 * 
 * SIMPLE VERSION - Powered by the bike, no battery needed.
 * 
 * How it works:
 * 1. You turn the key → bike powers up → ESP32 powers up
 * 2. ESP32 scans for your paired phone
 * 3. Phone found nearby? → Enter the code automatically
 * 4. Phone not found? → Do nothing (bike stays locked)
 * 
 * Wiring:
 * - Power the ESP32 from the bike's 12V (via buck converter to 5V)
 * - Use the "key on" 12V so ESP32 only runs when bike is on
 * - Connect 2 relays for INCREASE and MOVE buttons
 * 
 * That's it! No battery, no deep sleep, no complexity.
 */

#include "config.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <Preferences.h>

// ============================================
// CONFIGURATION
// ============================================

// How many times to scan before giving up (on boot)
const int MAX_SCAN_ATTEMPTS = 3;

// Delay between scan attempts (ms)
const int SCAN_RETRY_DELAY = 2000;

// After successful unlock, how long before allowing re-scan (ms)
const int UNLOCK_COOLDOWN = 60000;  // 1 minute

// ============================================
// RELAY FUNCTIONS
// ============================================

void setupRelays() {
  pinMode(PIN_INCREASE, OUTPUT);
  pinMode(PIN_MOVE, OUTPUT);
  digitalWrite(PIN_INCREASE, RELAY_OFF);
  digitalWrite(PIN_MOVE, RELAY_OFF);
}

void pulse(int pin) {
  digitalWrite(pin, RELAY_ON);
  delay(PULSE_DURATION);
  digitalWrite(pin, RELAY_OFF);
  delay(PULSE_GAP);
}

void pulseIncrease() { pulse(PIN_INCREASE); }
void pulseMove() { pulse(PIN_MOVE); }

bool isValidCode(String input) {
  if (input.length() != 4) return false;
  for (int i = 0; i < 4; i++) {
    if (input.charAt(i) < '0' || input.charAt(i) > '9') return false;
  }
  return true;
}

void enterCode(String codeStr) {
  Serial.print("🔓 Entering code: ");
  Serial.println(codeStr);
  
  // Initial pulse to start code entry
  pulseIncrease();
  delay(DIGIT_DELAY);
  
  // Process each digit
  for (int i = 0; i < 4; i++) {
    int digit = codeStr.charAt(i) - '0';
    Serial.print("  Digit ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(digit);
    
    for (int j = 0; j < digit; j++) {
      pulseIncrease();
    }
    
    pulseMove();
    delay(DIGIT_DELAY);
  }
  
  // Submit
  pulseMove();
  delay(DIGIT_DELAY);
  
  Serial.println("✓ Code entry complete!");
}

// ============================================
// STATE
// ============================================

Preferences preferences;
BLEScan* pBLEScan;

String storedCode = "";
String pairedDeviceAddress = "";

bool deviceFound = false;
bool unlockAttempted = false;
unsigned long lastUnlockTime = 0;

// ============================================
// BLE SCAN CALLBACK
// ============================================

class DeviceFoundCallback : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    String foundAddress = String(advertisedDevice.getAddress().toString().c_str());
    foundAddress.toUpperCase();
    
    String targetAddress = pairedDeviceAddress;
    targetAddress.toUpperCase();
    
    if (foundAddress == targetAddress) {
      int rssi = advertisedDevice.getRSSI();
      Serial.print("📱 Found phone! RSSI: ");
      Serial.print(rssi);
      Serial.print(" dBm");
      
      if (rssi >= BLE_RSSI_THRESHOLD) {
        Serial.println(" ✓ NEARBY - will unlock!");
        deviceFound = true;
        pBLEScan->stop();
      } else {
        Serial.println(" ✗ too far away");
      }
    }
  }
};

// ============================================
// PREFERENCES
// ============================================

void loadPreferences() {
  preferences.begin("ducati-fob", true);
  storedCode = preferences.getString("code", "");
  pairedDeviceAddress = preferences.getString("paired", "");
  preferences.end();
}

void saveCode(String code) {
  preferences.begin("ducati-fob", false);
  preferences.putString("code", code);
  preferences.end();
  storedCode = code;
  Serial.print("✓ Code saved: ");
  Serial.println(code);
}

void savePairedDevice(String address) {
  preferences.begin("ducati-fob", false);
  preferences.putString("paired", address);
  preferences.end();
  pairedDeviceAddress = address;
  Serial.print("✓ Paired device saved: ");
  Serial.println(address);
}

// ============================================
// BLE SCANNING
// ============================================

void initBLE() {
  Serial.println("Initializing Bluetooth...");
  BLEDevice::init("Ducati-Unlocker");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new DeviceFoundCallback());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
  Serial.println("✓ Bluetooth ready");
}

bool scanForPhone() {
  if (pairedDeviceAddress.length() == 0) {
    Serial.println("⚠️  No phone paired!");
    return false;
  }
  
  deviceFound = false;
  Serial.println("🔍 Scanning for phone...");
  
  pBLEScan->start(BLE_SCAN_DURATION, false);
  pBLEScan->clearResults();
  
  return deviceFound;
}

void scanAndListDevices() {
  Serial.println();
  Serial.println("🔍 Scanning for all BLE devices (10 sec)...");
  Serial.println();
  
  pBLEScan->setAdvertisedDeviceCallbacks(nullptr);
  BLEScanResults* results = pBLEScan->start(10, false);
  
  int count = results->getCount();
  Serial.print("Found ");
  Serial.print(count);
  Serial.println(" devices:");
  Serial.println();
  
  for (int i = 0; i < count; i++) {
    BLEAdvertisedDevice device = results->getDevice(i);
    String address = String(device.getAddress().toString().c_str());
    address.toUpperCase();
    
    Serial.print("  [");
    Serial.print(i + 1);
    Serial.print("] ");
    Serial.print(address);
    Serial.print(" (");
    Serial.print(device.getRSSI());
    Serial.print(" dBm)");
    
    if (device.haveName()) {
      Serial.print(" - ");
      Serial.print(device.getName().c_str());
    }
    Serial.println();
  }
  
  Serial.println();
  Serial.println("To pair: PAIR XX:XX:XX:XX:XX:XX");
  
  pBLEScan->setAdvertisedDeviceCallbacks(new DeviceFoundCallback());
  pBLEScan->clearResults();
}

// ============================================
// MAIN UNLOCK LOGIC
// ============================================

void attemptUnlock() {
  Serial.println();
  Serial.println("════════════════════════════════════════");
  Serial.println("  BIKE POWERED ON - ATTEMPTING UNLOCK");
  Serial.println("════════════════════════════════════════");
  Serial.println();
  
  for (int attempt = 1; attempt <= MAX_SCAN_ATTEMPTS; attempt++) {
    Serial.print("Scan attempt ");
    Serial.print(attempt);
    Serial.print("/");
    Serial.println(MAX_SCAN_ATTEMPTS);
    
    if (scanForPhone()) {
      // Phone found! Enter the code
      Serial.println();
      Serial.println("📱 Phone detected nearby!");
      delay(500);
      enterCode(storedCode);
      
      unlockAttempted = true;
      lastUnlockTime = millis();
      
      Serial.println();
      Serial.println("🏍️  Bike should be unlocked! Ride safe.");
      Serial.println();
      return;
    }
    
    if (attempt < MAX_SCAN_ATTEMPTS) {
      Serial.println("   Phone not found, retrying...");
      delay(SCAN_RETRY_DELAY);
    }
  }
  
  // Phone not found after all attempts
  Serial.println();
  Serial.println("❌ Phone not detected after all attempts.");
  Serial.println("   Bike remains locked.");
  Serial.println("   (Is your phone's Bluetooth on?)");
  Serial.println();
  
  unlockAttempted = true;  // Don't keep retrying
}

// ============================================
// SERIAL COMMANDS
// ============================================

void printHelp() {
  Serial.println();
  Serial.println("╔═══════════════════════════════════════════════════════╗");
  Serial.println("║        DUCATI UNLOCKER - Commands                     ║");
  Serial.println("╠═══════════════════════════════════════════════════════╣");
  Serial.println("║  SCAN         - Find nearby Bluetooth devices         ║");
  Serial.println("║  PAIR XX:...  - Pair with phone address               ║");
  Serial.println("║  CODE 1234    - Set unlock code                       ║");
  Serial.println("║  UNLOCK       - Try to unlock now                     ║");
  Serial.println("║  STATUS       - Show config                           ║");
  Serial.println("║  TEST         - Test relays                           ║");
  Serial.println("║  HELP         - This help                             ║");
  Serial.println("╚═══════════════════════════════════════════════════════╝");
  Serial.println();
}

void printStatus() {
  Serial.println();
  Serial.println("=== Unlocker Status ===");
  Serial.print("Code: ");
  Serial.println(storedCode.length() == 4 ? storedCode : "(not set)");
  Serial.print("Paired phone: ");
  Serial.println(pairedDeviceAddress.length() > 0 ? pairedDeviceAddress : "(not set)");
  Serial.print("RSSI threshold: ");
  Serial.print(BLE_RSSI_THRESHOLD);
  Serial.println(" dBm");
  Serial.print("Max scan attempts: ");
  Serial.println(MAX_SCAN_ATTEMPTS);
  Serial.println("=======================");
}

void testRelays() {
  Serial.println("Testing relays...");
  
  Serial.println("  INCREASE: ON");
  digitalWrite(PIN_INCREASE, RELAY_ON);
  delay(500);
  digitalWrite(PIN_INCREASE, RELAY_OFF);
  Serial.println("  INCREASE: OFF");
  delay(300);
  
  Serial.println("  MOVE: ON");
  digitalWrite(PIN_MOVE, RELAY_ON);
  delay(500);
  digitalWrite(PIN_MOVE, RELAY_OFF);
  Serial.println("  MOVE: OFF");
  
  Serial.println("✓ Relay test complete");
}

void processCommand(String input) {
  input.trim();
  String upperInput = input;
  upperInput.toUpperCase();
  
  if (upperInput == "HELP" || upperInput == "?") {
    printHelp();
  }
  else if (upperInput == "SCAN") {
    scanAndListDevices();
  }
  else if (upperInput.startsWith("PAIR ")) {
    String address = input.substring(5);
    address.trim();
    address.toUpperCase();
    if (address.length() >= 17) {
      savePairedDevice(address);
    } else {
      Serial.println("⚠️  Invalid format. Use: PAIR XX:XX:XX:XX:XX:XX");
    }
  }
  else if (upperInput.startsWith("CODE ")) {
    String code = input.substring(5);
    code.trim();
    if (isValidCode(code)) {
      saveCode(code);
    } else {
      Serial.println("⚠️  Invalid code. Must be 4 digits.");
    }
  }
  else if (upperInput == "UNLOCK") {
    if (storedCode.length() == 4 && pairedDeviceAddress.length() > 0) {
      unlockAttempted = false;
      attemptUnlock();
    } else {
      Serial.println("⚠️  Configure CODE and PAIR first!");
    }
  }
  else if (upperInput == "STATUS") {
    printStatus();
  }
  else if (upperInput == "TEST") {
    testRelays();
  }
  else if (isValidCode(upperInput)) {
    // Direct code entry
    enterCode(upperInput);
  }
  else if (input.length() > 0) {
    Serial.print("Unknown: ");
    Serial.println(input);
    Serial.println("Type HELP for commands");
  }
}

// ============================================
// SETUP & LOOP
// ============================================

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1000);
  
  setupRelays();
  
  Serial.println();
  Serial.println("╔═══════════════════════════════════════════════════════╗");
  Serial.println("║          DUCATI BLUETOOTH UNLOCKER                    ║");
  Serial.println("║          Bike-Powered Edition                         ║");
  Serial.println("╚═══════════════════════════════════════════════════════╝");
  Serial.println();
  
  loadPreferences();
  
  // Show current config
  if (storedCode.length() == 4) {
    Serial.print("Code: ");
    Serial.println(storedCode);
  } else {
    Serial.println("⚠️  No code set! Use: CODE 1234");
  }
  
  if (pairedDeviceAddress.length() > 0) {
    Serial.print("Phone: ");
    Serial.println(pairedDeviceAddress);
  } else {
    Serial.println("⚠️  No phone paired! Use: SCAN then PAIR");
  }
  
  Serial.println();
  
  initBLE();
  
  // If configured, try to unlock immediately on boot
  if (storedCode.length() == 4 && pairedDeviceAddress.length() > 0) {
    delay(1000);  // Give BLE a moment to stabilize
    attemptUnlock();
  } else {
    Serial.println("Configure with CODE and PAIR commands.");
    Serial.println("Type HELP for all commands.");
    printHelp();
  }
}

void loop() {
  // Process serial commands
  while (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    processCommand(input);
  }
  
  // Check for re-unlock after cooldown (in case bike was turned off/on)
  // This handles the case where ESP32 stays powered but we want to re-scan
  if (unlockAttempted && (millis() - lastUnlockTime > UNLOCK_COOLDOWN)) {
    // Could add periodic re-scan here if desired
    // For now, just reset the flag so manual UNLOCK works
    unlockAttempted = false;
  }
  
  delay(100);
}

