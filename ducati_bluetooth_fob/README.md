# Ducati Bluetooth Fob

DIY Bluetooth key fob replacement for Ducati Scrambler motorcycles.

## What Is This?

Your phone becomes your key fob. When you approach your bike with your phone in your pocket, the ESP32 detects it via Bluetooth and automatically enters your security code.

```
┌──────────────────────────────────────────────────────────────┐
│                                                              │
│   📱 Phone in pocket                                         │
│         │                                                    │
│         ▼                                                    │
│   🔍 ESP32 detects phone nearby (BLE)                        │
│         │                                                    │
│         ▼                                                    │
│   🔓 Enters your 4-digit code automatically                  │
│         │                                                    │
│         ▼                                                    │
│   🏍️ Bike unlocked! Ride away.                              │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

## Files

| File | Purpose |
|------|---------|
| `config.h` | Board selection and settings |
| `ducati_bluetooth_fob.ino` | Main sketch |

## Quick Start

1. Edit `config.h` to select your board
2. Open `ducati_bluetooth_fob.ino` in Arduino IDE
3. Upload to your ESP32
4. Open Serial Monitor (115200 baud)
5. Type `SCAN` to find your phone
6. Type `PAIR XX:XX:XX:XX:XX:XX` with your phone's address
7. Type `CODE 1234` with your actual unlock code
8. Done! Mount it on your bike.

## Commands

| Command | Description |
|---------|-------------|
| `SCAN` | List all nearby Bluetooth devices |
| `PAIR XX:XX:XX:XX:XX:XX` | Pair with a device address |
| `UNPAIR` | Remove the paired device |
| `CODE 1234` | Set the 4-digit unlock code |
| `ENTER` | Manually enter the stored code |
| `STATUS` | Show current configuration |
| `CLEAR` | Clear all stored settings |
| `TEST` | Test relay connections |
| `HELP` | Show all commands |

## Hardware

### What You Need

| Part | Qty | Purpose |
|------|-----|---------|
| ESP32-S3 board | 1 | Brain (with Bluetooth) |
| Relay module (SRD-05VDC) | 2 | Button simulation |
| Jumper wires | ~10 | Connections |
| Power source | 1 | See Power Options below |

### Wiring

```
ESP32                  Relay 1 (INCREASE)     Bike
─────                  ──────────────────     ────
GPIO 43 (TXD) ───────► S (signal)
                       + ◄───────────────────► 12V+
                       - ◄───────────────────► GND
                       COM ◄─────────────────► 12V+ (switch common)
                       NO ─────────────────────► Increase signal wire


ESP32                  Relay 2 (MOVE)         Bike
─────                  ──────────────────     ────
GPIO 44 (RXD) ───────► S (signal)
                       + ◄───────────────────► 12V+
                       - ◄───────────────────► GND
                       COM ◄─────────────────► 12V+ (switch common)
                       NO ─────────────────────► Move signal wire
```

## Power Options

### Option 1: USB Power Bank (Easiest)
- Plug a small power bank into the ESP32
- Stash under seat or in tank bag
- Recharge weekly

### Option 2: Bike's 12V (Always On)
- Use a 12V→5V buck converter
- Connect to always-on 12V source
- No charging needed

### Option 3: LiPo Battery (Portable)
- Add LiPo + charging circuit
- Enable deep sleep in `config.h`
- Lasts days to weeks depending on settings

See [LiPo Battery Setup](#lipo-battery-setup) below.

---

## LiPo Battery Setup

For truly portable operation, run the fob from a LiPo battery with deep sleep.

### Parts

| Part | Example | Purpose |
|------|---------|---------|
| LiPo Battery | 3.7V 1000mAh | Power |
| TP4056 module | — | Charges from USB |
| Boost converter | MT3608 | 3.7V → 5V |

Or use a board with built-in LiPo support:
- **SparkFun Thing Plus ESP32-S3**
- **Adafruit Feather ESP32-S3**

### Enable Deep Sleep

In `config.h`:

```cpp
#define FOB_DEEP_SLEEP_ENABLED
```

Adjust sleep duration:

```cpp
const int DEEP_SLEEP_SECONDS = 5;  // Wake every 5 seconds
```

### Battery Life Estimates

| Sleep Duration | Response Time | Battery Life (1000mAh) |
|----------------|---------------|------------------------|
| 5 seconds | ~5 sec | ~1 day |
| 30 seconds | ~30 sec | ~5 days |
| 60 seconds | ~1 min | ~10 days |

---

## Configuration

### RSSI Threshold

Controls how close your phone needs to be:

```cpp
const int BLE_RSSI_THRESHOLD = -70;  // ~3-5 meters
```

- `-50` = very close (~1 meter)
- `-70` = nearby (~3-5 meters)  
- `-90` = far away (~10+ meters)

### Power Detection (Optional)

Only enter code when bike is turned on:

```cpp
#define FOB_POWER_DETECT_ENABLED
```

Wire a voltage divider from the bike's 12V (key-on) to the detection pin.

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Phone not found in SCAN | Make sure Bluetooth is ON, phone is awake |
| Wrong device address | Try scanning again, phone may have changed address |
| Code not entering correctly | Adjust timing in `config.h`, test with `TEST` command |
| Relays not clicking | Check power to relay modules (need 5-12V, not 3.3V) |
| Deep sleep not waking | Check battery voltage, may be too low |

---

## Related

This is a companion project to [ducati_code_entry](../ducati_code_entry/) — a one-time bruteforce tool for recovering forgotten codes.

- **ducati_code_entry** = Recovery tool (finds the code)
- **ducati_bluetooth_fob** = Permanent solution (replaces the key fob)

---

## License

MIT - Use at your own risk. Only use on motorcycles you own.

