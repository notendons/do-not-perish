# do-not-perish

## Current state — September 2026

This repository is the early build stage of **Cogito v1**: a two-wheel self-balancing cube robot with an ESP32 and a tiny screen face, and frankly, my current attempt to escape from uni preparation stress.

It used to be a three-flywheel cube balanced on one corner. That was a beautiful physics problem and an unreasonable first hardware project, so it has been deliberately retired. The current design is an inverted-pendulum rover.
The firmware builds successfully. The KiCad schematic passes ERC. No physical hardware has been assembled or tested yet.

## What exists right now

### Hardware

`hardware/hardware.kicad_sch` is the current wiring schematic and passes KiCad’s Electrical Rules Check.

It includes:

- ESP32 30-pin DevKit
- GY-87 IMU
- HW-627 DRV8833 motor-driver module
- Two N20 motors
- 2.8-inch ILI9341 SPI TFT connector
- Two adjustable LM2596 buck converters
- 2S battery, BMS, fuse, and main switch
- Motor-noise capacitors, power-rail capacitors, and DRV8833 pulldown resistors

Power plan:

```text
2S battery → BMS → 3 A fuse → main switch
                            ├─ 6 V LM2596 → DRV8833 + motors
                            └─ 5 V LM2596 → ESP32 VIN

ESP32 3V3 → GY-87 + TFT VCC + TFT LED
```

The schematic has been drawn. It passes KiCad's Electrical Rules Check (ERC).

### Firmware

The PlatformIO project is in `firmware/` and builds for `esp32dev`.

| File | Current job | State |
| --- | --- | --- |
| `firmware/include/balance.h` | PID controller interface and telemetry struct | working |
| `firmware/include/imu.h` | IMU reader interface and telemetry struct | working |
| `firmware/include/motors.h` | Motor-control interface | working |
| `firmware/include/Pins.h` | GPIO assignments and robot constants | working |
| `firmware/src/balance.cpp` | PID calculation, integral limit, and telemetry | working |
| `firmware/src/display.cpp` | TFT face code | intentionally empty |
| `firmware/src/main.cpp` | Startup, safety logic, and two FreeRTOS tasks | working |
| `firmware/src/motors.cpp` | ESP32 PWM and signed DRV8833 motor control | working |
| `firmware/src/imu.cpp` | MPU6050 setup, calibration, and complementary filter | working |
| `firmware/platformio.ini` | ESP32/Arduino setup and library dependencies | working |

## Current control loop

```text
Core 1 — every 5 ms / 200 Hz
GY-87 → complementary filter → PID → both motors

Core 0 — every 100 ms / 10 Hz
serial status now, TFT face later
```

`main.cpp` now creates and connects the real control objects:

```text
ImuReader
BalanceController
Motors
```

Core 1 reads the IMU, checks for failure/fall conditions, calculates a PID motor correction, and sends that correction to both wheels.

If the IMU fails or the robot exceeds the 35° fall limit, the motors are disabled until reboot. That is intentional.

## Current GPIO map

```text
GY-87
  SDA → GPIO21
  SCL → GPIO22

ILI9341 TFT
  SCK   → GPIO18
  MOSI  → GPIO23
  DC    → GPIO33
  CS    → GPIO32
  RESET → GPIO13

DRV8833
  IN1 → GPIO14
  IN2 → GPIO27
  IN3 → GPIO26
  IN4 → GPIO25
```

The TFT runs from ESP32 `3V3` for matching 3.3 V logic levels. Its touch functionality is unused in the current version.

## Current limitations

- No wheel encoders in v1: it can balance by angle but cannot accurately hold position, so wandering is expected.
- PID values are starter values, not tuned values.
- Actual IMU axis direction, motor direction, and upright target angle must be verified after physical assembly.
- The display source file is still empty.
- The robot has not yet received its inevitable first opportunity to fall over in real life.

## Next task

1. Create the TFT display module in `display.cpp`.
2. Initialize the ILI9341 and draw a static face from Core 0.
3. Replace serial-only display status with an actual face update.
4. When hardware arrives, test power rails with a multimeter before connecting boards.
5. Test IMU direction, motor direction, and PWM with wheels lifted.
6. Begin PID tuning only after every subsystem works independently.

## Rule of the project

The code compiling means the robot is now qualified to fail in a much more interesting way. Measure voltages, test subsystems separately, and never trust a schematic more than a multimeter.