# Breadboard Layout for Ducati Code Entry

## Rails
- Left  (+) = 3V3 from ESP32
- Left  (-) = GND from ESP32
- Right (+) = 12V from bike
- Right (-) = Bike GND (jumper to left GND)

## Component Placement

```
   3V3 GND  a b c d e   f g h i j  12V  GND
    +  -    ─ ─ ─ ─ ─   ─ ─ ─ ─ ─   +   -
    │  │                            │   │
    │  │  1 ■-■-■-■-■   ■-■-■-■-■   │   │
    │  │  2 S-■-■-■-■   ■-■-■-■-■   │   │  ← Blue module S (row 2)
    │  │  3 +-■-■-■-■   ■-■-■-■-■   │   │  ← Blue module + (row 3)
    │  │  4 −-■-■-■-■   ■-■-■-■-■   │   │  ← Blue module - (row 4)
    │  │  5 ■-■-■-■-■   ■-■-■-■-■   │   │
    │  │  6 E-■-■-■-■   ■-■-■-■-■   │   │  ← Transistor E (row 6)
    │  │  7 B-■-■-■-R~~~R-■-■-■-■   │   │  ← Transistor B (row 7) + Resistor to row 7
    │  │  8 C-■-■-■-■   ■-■-■-■-■   │   │  ← Transistor C (row 8)
    │  │  9 ■-■-■-■-■   ■-■-■-■-■   │   │
    │  │ 10 ■-■-■-■-■   ■-■-■-■-■   │   │
    │  │                            │   │
    ─  ─    ─ ─ ─ ─ ─   ─ ─ ─ ─ ─   ─   ─
    +  -    a b c d e   f g h i j   +   -
   3V3 GND                         12V  GND
```

## Jumper Wires Needed

| From | To | Purpose |
|------|----|---------|
| ESP32 TXD | Row 2 (any b-e) | Signal to blue module |
| ESP32 RXD | Row 7, right side (f-i) | Signal through resistor to transistor |
| ESP32 3V3 | Left + rail | Power rail |
| ESP32 GND | Left - rail | Ground rail |
| Left + rail | Row 3 (any b-e) | Power to blue module |
| Left - rail | Row 4 (any b-e) | Ground to blue module |
| Left - rail | Row 6 (any b-e) | Ground to transistor E |
| Left - rail | Right - rail | Connect grounds together |
| Row 8 (any b-e) | Relay WHITE wire (85) | Transistor C to relay coil - |
| Right + rail | Relay BLACK wire (86) | 12V to relay coil + |
| Right + rail | Diode STRIPE end | Flyback protection |
| Row 8 (any b-e) | Diode other end | Flyback protection |
| Bike 12V+ | Right + rail | Power from bike |
| Bike GND | Right - rail | Ground from bike |

## Off-Breadboard Connections

### Blue Module Screw Terminals → Bike
- COM → Increase wire A
- NO → Increase wire B

### Automotive Relay Harness → Bike  
- BLUE (30) → Move wire A
- YELLOW (87) → Move wire B
- BLACK (86) → already on breadboard (12V rail)
- WHITE (85) → already on breadboard (row 8)
- RED (87a) → not used, leave disconnected