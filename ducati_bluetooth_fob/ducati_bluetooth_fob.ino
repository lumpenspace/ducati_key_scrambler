/*
 * Ducati Scrambler Bluetooth Fob
 * 
 * Turns your ESP32 into a DIY key fob replacement.
 * 
 * How it works:
 * 1. Pairs with your phone via Bluetooth
 * 2. Runs in low-power mode, scanning for your phone
 * 3. When your phone is nearby AND the bike is powered on,
 *    automatically enters your preset security code
 * 
 * Requires: ESP32 with Bluetooth (ESP32-S3, etc.)
 */

#include "config.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <Preferences.h>

#ifdef FOB_DEEP_SLEEP_ENABLED
  #include "esp_sleep.h"
  
  // Store state in RTC memory (survives deep sleep)
  RTC_DATA_ATTR int bootCount = 0;
  RTC_DATA_ATTR bool wasCodeEntered = false;
  RTC_DATA_ATTR unsigned long codeEntryBootCount = 0;
#endif

// ============================================
// RELAY FUNCTIONS
// ============================================

void setupRelays() {
  pinMode(PIN_INCREASE, OUTPUT);
  pinMode(PIN_MOVE, OUTPUT);
  digitalWrite(PIN_INCREASE, RELAY_OFF);
  digitalWrite(PIN_MOVE, RELAY_OFF);
  
  #ifdef FOB_POWER_DETECT_ENABLED
    pinMode(PIN_POWER_DETECT, INPUT);
  #endif
}

void pulse(int pin) {
  digitalWrite(pin, RELAY_ON);
  delay(PULSE_DURATION);
  digitalWrite(pin, RELAY_OFF);
  delay(PULSE_GAP);
}

void pulseIncrease() { pulse(PIN_INCREASE); }
void pulseMove() { pulse(PIN_MOVE); }

String formatCode(int code) {
  String s = String(code);
  while (s.length() < 4) s = "0" + s;
  return s;
}

bool isValidCode(String input) {
  if (input.length() != 4) return false;
  for (int i = 0; i < 4; i++) {
    if (input.charAt(i) < '0' || input.charAt(i) > '9') return false;
  }
  return true;
}

void enterCode(String codeStr, bool verbose = true) {
  if (verbose) {
    Serial.print("Entering code: ");
    Serial.println(codeStr);
  }
  
  // Initial pulse to start code entry
  pulseIncrease();
  delay(DIGIT_DELAY);
  
  // Process each digit
  for (int i = 0; i < 4; i++) {
    int digit = codeStr.charAt(i) - '0';
    
    if (verbose) {
      Serial.print("  Digit ");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.println(digit);
    }
    
    // Pulse increase 'digit' times
    for (int j = 0; j < digit; j++) {
      pulseIncrease();
    }
    
    // Move to next digit
    pulseMove();
    delay(DIGIT_DELAY);
  }
  
  // Submit
  pulseMove();
  delay(DIGIT_DELAY);
  
  if (verbose) {
    Serial.println("✓ Code entry complete");
  }
}

// ============================================
// RUNTIME STATE
// ============================================

// Stored in flash (survives reboot)
String storedCode = "";
String pairedDeviceAddress = "";

Preferences preferences;
BLEScan* pBLEScan;

bool deviceNearby = false;
bool codeEntered = false;
unsigned long lastScanTime = 0;
unsigned long lastCodeEntryTime = 0;

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
      Serial.print("📱 Found paired device! RSSI: ");
      Serial.print(rssi);
      Serial.print(" dBm");
      
      if (rssi >= BLE_RSSI_THRESHOLD) {
        Serial.println(" ✓ (nearby)");
        deviceNearby = true;
      } else {
        Serial.println(" ✗ (too far)");
        deviceNearby = false;
      }
      
      // Stop scanning once we find our device
      pBLEScan->stop();
    }
  }
};

// ============================================
// PREFERENCES (FLASH STORAGE)
// ============================================

void loadPreferences() {
  preferences.begin("ducati-fob", true);  // Read-only
  storedCode = preferences.getString("code", "");
  pairedDeviceAddress = preferences.getString("paired", "");
  preferences.end();
  
  Serial.println();
  Serial.println("=== Stored Configuration ===");
  if (storedCode.length() == 4) {
    Serial.print("Code: ");
    Serial.println(storedCode);
  } else {
    Serial.println("Code: (not set)");
  }
  if (pairedDeviceAddress.length() > 0) {
    Serial.print("Paired device: ");
    Serial.println(pairedDeviceAddress);
  } else {
    Serial.println("Paired device: (not set)");
  }
  Serial.println("============================");
}

void saveCode(String code) {
  preferences.begin("ducati-fob", false);  // Read-write
  preferences.putString("code", code);
  preferences.end();
  storedCode = code;
  Serial.print("✓ Code saved: ");
  Serial.println(code);
}

void savePairedDevice(String address) {
  preferences.begin("ducati-fob", false);  // Read-write
  preferences.putString("paired", address);
  preferences.end();
  pairedDeviceAddress = address;
  Serial.print("✓ Paired device saved: ");
  Serial.println(address);
}

void clearPreferences() {
  preferences.begin("ducati-fob", false);
  preferences.clear();
  preferences.end();
  storedCode = "";
  pairedDeviceAddress = "";
  Serial.println("✓ All settings cleared");
}

// ============================================
// BLE SCANNING
// ============================================

void initBLE() {
  Serial.println("Initializing Bluetooth...");
  BLEDevice::init("Ducati-Fob");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new DeviceFoundCallback());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
  Serial.println("✓ Bluetooth initialized");
}

void scanForDevice() {
  if (pairedDeviceAddress.length() == 0) {
    return;
  }
  
  deviceNearby = false;
  Serial.println();
  Serial.println("🔍 Scanning for paired device...");
  
  BLEScanResults* results = pBLEScan->start(BLE_SCAN_DURATION, false);
  
  if (!deviceNearby) {
    Serial.println("   Device not found nearby");
  }
  
  pBLEScan->clearResults();
}

void scanAndListDevices() {
  Serial.println();
  Serial.println("🔍 Scanning for all BLE devices...");
  Serial.println("   (Look for your phone's name or address)");
  Serial.println();
  
  // Temporarily remove callback to see all devices
  pBLEScan->setAdvertisedDeviceCallbacks(nullptr);
  
  BLEScanResults* results = pBLEScan->start(10, false);  // 10 second scan
  
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
    Serial.print(" (RSSI: ");
    Serial.print(device.getRSSI());
    Serial.print(")");
    
    if (device.haveName()) {
      Serial.print(" - ");
      Serial.print(device.getName().c_str());
    }
    
    Serial.println();
  }
  
  Serial.println();
  Serial.println("To pair, type: PAIR XX:XX:XX:XX:XX:XX");
  Serial.println("(Replace with your device's address)");
  
  // Restore callback
  pBLEScan->setAdvertisedDeviceCallbacks(new DeviceFoundCallback());
  pBLEScan->clearResults();
}

// ============================================
// BIKE POWER DETECTION
// ============================================

bool isBikePowered() {
  #ifdef FOB_POWER_DETECT_ENABLED
    return digitalRead(PIN_POWER_DETECT) == HIGH;
  #else
    // No detection circuit — assume bike is ready
    return true;
  #endif
}

// ============================================
// MAIN LOGIC
// ============================================

void attemptUnlock() {
  if (storedCode.length() != 4) {
    Serial.println("⚠️  No code configured! Use: CODE 1234");
    return;
  }
  
  Serial.println();
  Serial.println("🔓 Phone nearby + Bike powered on = Entering code!");
  Serial.println();
  
  enterCode(storedCode, true);
  
  codeEntered = true;
  lastCodeEntryTime = millis();
  
  Serial.println();
  Serial.println("✓ Code sent! Waiting for cooldown...");
}

void printHelp() {
  Serial.println();
  Serial.println("╔══════════════════════════════════════════════════════════╗");
  Serial.println("║        DUCATI BLUETOOTH FOB - Commands                   ║");
  Serial.println("╠══════════════════════════════════════════════════════════╣");
  Serial.println("║  SCAN        - List all nearby Bluetooth devices         ║");
  Serial.println("║  PAIR XX:... - Pair with device at address XX:XX:XX...   ║");
  Serial.println("║  UNPAIR      - Remove paired device                      ║");
  Serial.println("║  CODE 1234   - Set the 4-digit unlock code               ║");
  Serial.println("║  ENTER       - Manually enter the stored code now        ║");
  Serial.println("║  STATUS      - Show current configuration                ║");
  Serial.println("║  CLEAR       - Clear all stored settings                 ║");
  Serial.println("║  TEST        - Test relay connections                    ║");
  Serial.println("║  HELP        - Show this help                            ║");
  Serial.println("╚══════════════════════════════════════════════════════════╝");
  Serial.println();
}

void printStatus() {
  Serial.println();
  Serial.println("=== Bluetooth Fob Status ===");
  Serial.print("Code: ");
  if (storedCode.length() == 4) {
    Serial.println(storedCode);
  } else {
    Serial.println("(not set)");
  }
  Serial.print("Paired device: ");
  if (pairedDeviceAddress.length() > 0) {
    Serial.println(pairedDeviceAddress);
  } else {
    Serial.println("(not set)");
  }
  Serial.print("Device nearby: ");
  Serial.println(deviceNearby ? "Yes" : "No");
  Serial.print("Bike powered: ");
  Serial.println(isBikePowered() ? "Yes" : "Unknown");
  Serial.print("RSSI threshold: ");
  Serial.print(BLE_RSSI_THRESHOLD);
  Serial.println(" dBm");
  Serial.println("=============================");
}

void testRelays() {
  Serial.println();
  Serial.println("Testing relays...");
  
  Serial.println("  INCREASE relay: ON");
  digitalWrite(PIN_INCREASE, RELAY_ON);
  delay(500);
  digitalWrite(PIN_INCREASE, RELAY_OFF);
  Serial.println("  INCREASE relay: OFF");
  delay(500);
  
  Serial.println("  MOVE relay: ON");
  digitalWrite(PIN_MOVE, RELAY_ON);
  delay(500);
  digitalWrite(PIN_MOVE, RELAY_OFF);
  Serial.println("  MOVE relay: OFF");
  
  Serial.println("✓ Relay test complete");
}

void processCommand(String input) {
  input.trim();
  input.toUpperCase();
  
  if (input == "HELP" || input == "?") {
    printHelp();
  }
  else if (input == "SCAN") {
    scanAndListDevices();
  }
  else if (input.startsWith("PAIR ")) {
    String address = input.substring(5);
    address.trim();
    if (address.length() >= 17) {  // XX:XX:XX:XX:XX:XX
      savePairedDevice(address);
    } else {
      Serial.println("⚠️  Invalid address format. Use: PAIR XX:XX:XX:XX:XX:XX");
    }
  }
  else if (input == "UNPAIR") {
    preferences.begin("ducati-fob", false);
    preferences.remove("paired");
    preferences.end();
    pairedDeviceAddress = "";
    Serial.println("✓ Device unpaired");
  }
  else if (input.startsWith("CODE ")) {
    String code = input.substring(5);
    code.trim();
    if (isValidCode(code)) {
      saveCode(code);
    } else {
      Serial.println("⚠️  Invalid code. Must be exactly 4 digits (0-9)");
    }
  }
  else if (input == "ENTER") {
    if (storedCode.length() == 4) {
      enterCode(storedCode, true);
    } else {
      Serial.println("⚠️  No code set! Use: CODE 1234");
    }
  }
  else if (input == "STATUS") {
    printStatus();
  }
  else if (input == "CLEAR") {
    clearPreferences();
  }
  else if (input == "TEST") {
    testRelays();
  }
  else if (isValidCode(input)) {
    // Direct code entry
    enterCode(input, true);
  }
  else if (input.length() > 0) {
    Serial.print("Unknown command: ");
    Serial.println(input);
    Serial.println("Type HELP for available commands");
  }
}

// ============================================
// DEEP SLEEP FUNCTIONS
// ============================================

#ifdef FOB_DEEP_SLEEP_ENABLED

void goToDeepSleep() {
  Serial.println("💤 Entering deep sleep...");
  Serial.flush();
  
  // Configure wake-up timer
  esp_sleep_enable_timer_wakeup(DEEP_SLEEP_SECONDS * 1000000ULL);
  
  // Turn off BLE before sleeping
  BLEDevice::deinit(false);
  
  // Enter deep sleep
  esp_deep_sleep_start();
}

void handleDeepSleepWake() {
  bootCount++;
  
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  
  if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
    Serial.print("⏰ Wake #");
    Serial.print(bootCount);
    Serial.println(" (timer)");
    
    // Check if we're in cooldown (entered code recently)
    if (wasCodeEntered) {
      // Stay in cooldown for ~6 wake cycles (30 seconds at 5 sec intervals)
      int cooldownCycles = CODE_COOLDOWN_MS / (DEEP_SLEEP_SECONDS * 1000);
      if (bootCount - codeEntryBootCount < cooldownCycles) {
        Serial.println("   Still in cooldown, going back to sleep...");
        goToDeepSleep();
        return;
      }
      wasCodeEntered = false;
    }
  } else {
    Serial.println("🔌 First boot (power on)");
  }
}

#endif

// ============================================
// SETUP & LOOP
// ============================================

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(2000);  // Wait for Serial
  
  setupRelays();
  
  Serial.println();
  Serial.println("╔══════════════════════════════════════════════════════════╗");
  Serial.println("║          DUCATI BLUETOOTH FOB v1.0                       ║");
  Serial.println("║          DIY Key Fob Replacement                         ║");
  Serial.println("╚══════════════════════════════════════════════════════════╝");
  
  #ifdef FOB_DEEP_SLEEP_ENABLED
    Serial.println("║          ⚡ Deep Sleep Mode ENABLED                      ║");
    Serial.println("╚══════════════════════════════════════════════════════════╝");
    handleDeepSleepWake();
  #endif
  
  loadPreferences();
  initBLE();
  
  #ifdef FOB_DEEP_SLEEP_ENABLED
    // In deep sleep mode, skip interactive setup — just scan and sleep
    if (pairedDeviceAddress.length() > 0 && storedCode.length() == 4) {
      // Do one scan
      scanForDevice();
      
      if (deviceNearby && isBikePowered()) {
        attemptUnlock();
        wasCodeEntered = true;
        codeEntryBootCount = bootCount;
      }
      
      // Go back to sleep
      goToDeepSleep();
    } else {
      // Not configured — stay awake for setup
      Serial.println();
      Serial.println("⚠️  Deep sleep mode requires configuration first!");
      Serial.println("   Configure via Serial, then reset to enable deep sleep.");
    }
  #endif
  
  printHelp();
  
  if (pairedDeviceAddress.length() == 0) {
    Serial.println("⚠️  No device paired yet!");
    Serial.println("   1. Make sure your phone's Bluetooth is ON");
    Serial.println("   2. Type SCAN to find your phone");
    Serial.println("   3. Type PAIR XX:XX:XX:XX:XX:XX with your phone's address");
    Serial.println();
  }
  
  if (storedCode.length() != 4) {
    Serial.println("⚠️  No unlock code set!");
    Serial.println("   Type CODE 1234 (replace with your actual code)");
    Serial.println();
  }
}

void loop() {
  // Process serial commands
  while (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    processCommand(input);
  }
  
  // Skip scanning if not configured
  if (pairedDeviceAddress.length() == 0 || storedCode.length() != 4) {
    delay(100);
    return;
  }
  
  // Cooldown after successful code entry
  if (codeEntered) {
    if (millis() - lastCodeEntryTime < CODE_COOLDOWN_MS) {
      delay(100);
      return;
    }
    codeEntered = false;
    Serial.println("Cooldown complete, resuming scanning...");
  }
  
  // Time to scan?
  if (millis() - lastScanTime >= SCAN_INTERVAL_MS) {
    lastScanTime = millis();
    scanForDevice();
    
    // Check if we should enter the code
    if (deviceNearby && isBikePowered() && !codeEntered) {
      attemptUnlock();
    }
  }
  
  delay(100);
}

