# Wiring

The build has two distinct configurations: one for flashing the ESP32-CAM, one
for running it. The only difference is the `IO0` jumper and the actuation side.

## 1. Flashing configuration

The ESP32-CAM has no USB connector, so an Arduino UNO is used as a USB-to-serial
bridge. Tying the UNO's `RESET` to `GND` holds the ATmega328P in reset, which
leaves the on-board USB-serial chip connected straight to the header pins — so
`TX0` and `RX0` go to the ESP32's `U0T` and `U0R` **without** crossing.

| ESP32-CAM | Arduino UNO |
|---|---|
| `5V` | `5V` |
| `GND` | `GND` |
| `U0T` | `TX0` (pin 1) |
| `U0R` | `RX0` (pin 0) |
| `IO0` | `GND` (boot mode) |
| — | `RESET` → `GND` |

Upload, then **remove the `IO0`–`GND` jumper** and press `RST` to boot normally.

## 2. Run configuration

| ESP32-CAM pin | Connected to | Function |
|---|---|---|
| `GPIO 2` | Relay module `IN` | Unlock command |
| `GPIO 12` | Green LED anode (via ~220 Ω) | Door unlocked |
| `GPIO 13` | Red LED anode (via ~220 Ω) | Door locked |
| `5V` | Relay module `VCC` | Relay coil supply |
| `GND` | Relay module `GND`, LED cathodes | Common ground |

Lock side, kept separate from the logic:

```
12 V adapter (+) ──> relay COM
relay NO ─────────> solenoid lock (+)
12 V adapter (−) ──> solenoid lock (−)
```

The relay is the only connection between the 3.3 V logic and the 12 V lock. The
solenoid draws far more current than any ESP32 pin can source, and it is an
inductive load, so it must never be driven directly.

## Notes

- Power the ESP32-CAM from a supply that can deliver the current peaks during
  Wi-Fi transmission; a weak 5 V rail shows up as brown-out resets at boot.
- `GPIO 0` must be floating in run mode, otherwise the module re-enters the
  bootloader instead of starting the sketch.
- The relay is driven `LOW` in `setup()` before anything else, so the door is
  locked during boot and after a reset.
