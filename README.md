# Ducati Scrambler Security Tools

DIY tools for Ducati Scrambler owners who've lost their key fob.

```
┌─────────────────────────────────────────────────────────────────────────┐
│                                                                         │
│   🔑 Lost your Ducati key fob?                                          │
│                                                                         │
│   Step 1: Recover your code    →  ducati_code_entry/                    │
│   Step 2: Never need it again  →  ducati_bluetooth_fob/                 │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

## Projects

### [ducati_code_entry/](ducati_code_entry/) — Code Recovery Tool

Bruteforce the 4-digit security code when you've forgotten it or lost your fob.

- **Use case:** One-time recovery
- **Hardware:** ESP32/Arduino + 3 relays
- **Time:** Up to 11 hours (worst case)
- **Result:** Your 4-digit code

### [ducati_bluetooth_fob/](ducati_bluetooth_fob/) — Bluetooth Key Fob

Turn your phone into a wireless key fob. Automatically enters your code when you approach the bike.

- **Use case:** Permanent key fob replacement
- **Hardware:** ESP32 + 2 relays + optional LiPo battery
- **Time:** Set up once, works forever
- **Result:** Phone = Key

## Quick Comparison

| Feature | Code Entry | Bluetooth Fob |
|---------|------------|---------------|
| Purpose | Recover forgotten code | Replace key fob |
| Relays needed | 3 | 2 |
| Power cycling | Yes | No |
| Bluetooth | No | Yes |
| Battery powered | No (use charger) | Optional (LiPo) |
| One-time use | Yes | No (permanent) |

## Workflow

```
┌──────────────────┐     ┌───────────────────┐     ┌──────────────────┐
│                  │     │                   │     │                  │
│  Lost key fob    │────►│  Bruteforce code  │────►│  Know your code  │
│                  │     │  (ducati_code_    │     │                  │
│                  │     │   entry)          │     │                  │
└──────────────────┘     └───────────────────┘     └────────┬─────────┘
                                                            │
                                                            ▼
                                                  ┌──────────────────┐
                                                  │                  │
                                                  │  Set up BT fob   │
                                                  │  (ducati_        │
                                                  │   bluetooth_fob) │
                                                  │                  │
                                                  └────────┬─────────┘
                                                            │
                                                            ▼
                                                  ┌──────────────────┐
                                                  │                  │
                                                  │  Phone = Key 📱  │
                                                  │                  │
                                                  └──────────────────┘
```

## Hardware Overview

### For Code Recovery (3 relays)

```
ESP32 ──► Relay 1 (INCREASE) ──► Bike button wire
      ──► Relay 2 (MOVE)     ──► Bike button wire  
      ──► Relay 3 (POWER)    ──► Power cycling
```

### For Bluetooth Fob (2 relays)

```
ESP32 ──► Relay 1 (INCREASE) ──► Bike button wire
      ──► Relay 2 (MOVE)     ──► Bike button wire
      
📱 Phone (Bluetooth) ──► ESP32 detects proximity ──► Enters code
```

## Requirements

- **ESP32-S3** (with Bluetooth) or Arduino
- **Relay modules** (SRD-05VDC or automotive relays)
- **Multimeter** (to find the right wires)
- **Soldering iron** (optional, for permanent install)
- **Battery tender** (for bruteforce — takes hours)

## ⚠️ Legal Notice

These tools are for **legitimate owners only**. Using them on a vehicle you don't own is theft. Always have proof of ownership handy. See the full disclaimer in each project's README.

## License

MIT - Use at your own risk.

