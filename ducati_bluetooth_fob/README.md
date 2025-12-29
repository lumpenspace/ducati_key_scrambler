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

## Two Variants

| Sketch | Power Source | How It Works |
|--------|--------------|--------------|
| `ducati_unlocker.ino` | **Bike** (simple!) | Turn key → ESP32 boots → scans for phone → unlocks |
| `ducati_bluetooth_fob.ino` | **Battery/USB** | Always on, continuously scanning for phone |

### Which One Should I Use?

**Use `ducati_unlocker.ino` if:**
- You want the simplest setup
- You don't want to deal with batteries
- You're OK with the ESP32 only running when the key is turned

**Use `ducati_bluetooth_fob.ino` if:**
- You want the ESP32 always on (even before turning the key)
- You're using a battery (LiPo) or USB power bank
- You want "approach and it's ready" behavior

---

## Files

| File | Purpose |
|------|---------|
| `config.h` | Board selection and settings |
| `ducati_unlocker.ino` | **Simple:** Bike-powered, scan on boot |
| `ducati_bluetooth_fob.ino` | **Advanced:** Battery-powered, continuous scan |

---

## Quick Start (Unlocker - Recommended)

The unlocker is the simplest variant: powered by the bike, no battery needed.

### How It Works

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│             │     │             │     │             │     │             │
│  Turn key   │────►│ ESP32 boots │────►│ Scans for   │────►│ Phone found │
│             │     │             │     │ your phone  │     │ → UNLOCK!   │
│             │     │             │     │             │     │             │
└─────────────┘     └─────────────┘     └─────────────┘     └─────────────┘
                                              │
                                              ▼
                                        ┌─────────────┐
                                        │ Phone NOT   │
                                        │ found →     │
                                        │ stays locked│
                                        └─────────────┘
```

### Setup

1. Edit `config.h` to select your board
2. Open `ducati_unlocker.ino` in Arduino IDE
3. Upload to your ESP32
4. Open Serial Monitor (115200 baud)
5. Type `SCAN` to find your phone's Bluetooth address
6. Type `PAIR XX:XX:XX:XX:XX:XX` with your phone's address
7. Type `CODE 1234` with your actual unlock code
8. Mount the ESP32 on your bike, powered from 12V (key-on circuit)

### Wiring (Unlocker)

```
Bike 12V (key on) ──► Buck Converter ──► ESP32 5V
                      (12V → 5V)

ESP32 GPIO 43 ──────► Relay 1 (INCREASE) ──► Bike button wire
ESP32 GPIO 44 ──────► Relay 2 (MOVE) ──────► Bike button wire
```

Power from the **key-on 12V circuit** so the ESP32 only runs when the bike is turned on.

### Commands

| Command | Description |
|---------|-------------|
| `SCAN` | List nearby Bluetooth devices |
| `PAIR XX:XX:XX:XX:XX:XX` | Pair with phone address |
| `CODE 1234` | Set unlock code |
| `UNLOCK` | Manually try to unlock now |
| `STATUS` | Show configuration |
| `TEST` | Test relays |
| `HELP` | Show all commands |

---

## Advanced: Battery-Powered Fob

If you want the ESP32 running *before* you turn the key (true "approach and unlock"), use the battery-powered version.

### Quick Start (Battery Version)

1. Edit `config.h` to select your board
2. Open `ducati_bluetooth_fob.ino` in Arduino IDE
3. Upload to your ESP32
4. Configure with `SCAN`, `PAIR`, and `CODE` commands
5. Power with USB power bank or LiPo battery

### Power Options

| Option | Pros | Cons |
|--------|------|------|
| USB Power Bank | Simple, no wiring | Need to recharge, bulky |
| LiPo + Deep Sleep | Compact, lasts days | More complex, needs charging circuit |
| Bike 12V (always on) | No battery | Drains bike battery slowly |

### LiPo Battery Setup

For portable operation with a LiPo battery:

**Parts:**
- LiPo Battery (3.7V 1000mAh)
- TP4056 charging module
- MT3608 boost converter (3.7V → 5V)

Or use a board with built-in LiPo support:
- SparkFun Thing Plus ESP32-S3
- Adafruit Feather ESP32-S3

**Enable deep sleep in `config.h`:**

```cpp
#define FOB_DEEP_SLEEP_ENABLED
```

**Battery Life Estimates:**

| Sleep Duration | Response Time | Battery Life (1000mAh) |
|----------------|---------------|------------------------|
| 5 seconds | ~5 sec | ~1 day |
| 30 seconds | ~30 sec | ~5 days |
| 60 seconds | ~1 min | ~10 days |

---

## Hardware

### What You Need

| Part | Qty | Purpose |
|------|-----|---------|
| ESP32-S3 board | 1 | Brain (with Bluetooth) |
| Relay module (SRD-05VDC) | 2 | Button simulation |
| Buck converter (12V→5V) | 1 | Power from bike (for unlocker) |
| Jumper wires | ~10 | Connections |

### Wiring Diagram

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

---

## Configuration

### RSSI Threshold

Controls how close your phone needs to be (in `config.h`):

```cpp
const int BLE_RSSI_THRESHOLD = -70;  // ~3-5 meters
```

| Value | Distance |
|-------|----------|
| `-50` | Very close (~1 meter) |
| `-70` | Nearby (~3-5 meters) |
| `-90` | Far away (~10+ meters) |

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Phone not found in SCAN | Make sure Bluetooth is ON, phone is awake |
| Wrong device address | Try scanning again, phone address can change |
| Code not entering correctly | Adjust timing in `config.h`, use `TEST` command |
| Relays not clicking | Check power (need 5-12V, not 3.3V) |
| ESP32 not booting | Check buck converter output is 5V |

---

## Related

This is a companion project to [ducati_code_entry](../ducati_code_entry/) — a one-time bruteforce tool for recovering forgotten codes.

- **ducati_code_entry** = Recovery tool (finds the code)
- **ducati_bluetooth_fob** = Permanent solution (replaces the key fob)

---

## License

MIT - Use at your own risk. Only use on motorcycles you own.
