# Ducati Unlocker V2.1 (Secure HID)

A secure, auto-connecting unlocker for the Ducati Scrambler using XIAO ESP32C6 (default) or ESP32 DevKit V1.

## How to Use

### 1. Initial Setup (Pairing)

Pairing is initiated via a USB serial command and is time-limited by the firmware.

1. Connect ESP32 to computer via USB.
2. Open Serial Monitor (9600 baud).
3. Type: `PAIR`
    * The LED will start **blinking fast**.
4. Open Bluetooth settings on your phone.
5. Connect to **"Ducati-Key"**.
6. Accept the "Pair" or "Bond" request.

### 2. Normal Operation

1. Walk up to the bike.
2. Turn Key ON.
3. Your phone will **automatically connect** (usually within 5-10 seconds).
4. Dashboard asks for PIN code.
5. **Toggle the Switch/Button**.
6. Bike unlocks.

*Note: The device requires bonding; only bonded phones can connect and trigger unlocks.*

## Hardware

### Option A: Optocoupler Layout (Compact & Silent)

Using 2x PC817 optocouplers instead of relays (INCREMENT + CONFIRM).

**Breadboard Layout:**

* **ESP32 3V3** → (+) Rail
* **ESP32 GND** → (-) Rail

| Opto Pin | Function | Connection |
| - | - | - |
| **1 (Anode)** | Input (+) | ESP32 GPIO via **220Ω Resistor** |
| **2 (Cathode)** | Input (-) | Breadboard (-) Rail (GND) |
| **4 (Collector)** | Output (+) | Bike Button Signal Wire |
| **3 (Emitter)** | Output (-) | Bike Ground |

**Pin Assignments (XIAO ESP32C6):**

| Component | ESP32 Pin | Logic |
| - | - | - |
| **Opto 1 (Increment)** | D0 (GPIO0) | Active HIGH |
| **Opto 2 (Confirm)** | D1 (GPIO1) | Active HIGH |
| **Trigger Button** | D2 (GPIO2) | Pull-to-GND |
| **Button LED** | D3 (GPIO3) | Active HIGH |
| **Status LED** | LED_BUILTIN (GPIO15) | Built-in |

**Pin Assignments (DevKit V1):**

| Component | ESP32 Pin | Logic |
| - | - | - |
| **Opto 1 (Increment)** | D18 | Active HIGH |
| **Opto 2 (Confirm)** | D19 | Active HIGH |
| **Trigger Button** | D23 | Pull-to-GND |
| **Status LED** | D2 | Built-in |

### Wiring Diagram (Opto Channel)

```text
       PC817
      ┌───U───┐
(+) 1-│ •     │-4 (Collector) ──► Bike Button Signal
      │       │
(-) 2-│       │-3 (Emitter)   ──► Bike Ground
      └───────┘

GPIO ──[220Ω]──(1)
GND  ──────────(2)
```

## Configuration

Set your PIN code via Serial Monitor (9600 baud):

```text
CODE 1234
```

Test relays/optos manually:

```text
TEST    (Cycles relays 1-2)
R1      (Toggles Channel 1)
R2      (Toggles Channel 2)
BTN     (Reads Trigger Button status)
```
