# do-not-perish

## Current state — September 2026

This repository is currently the early build stage of **TiltCube v1**: a two-wheel self-balancing cube robot with an ESP32 and a tiny screen face.

It is no longer the original three-flywheel, corner-balancing reaction-wheel cube concept. That idea was cool, absurdly difficult as a first version, and has been intentionally retired. The current robot is an inverted-pendulum rover: it balances on two wheels and can eventually drive around while looking vaguely alive.

The project is real, organised, and compiling. It is not physically assembled yet and it cannot balance yet. That is normal.

## What exists right now

### Hardware schematic

`hardware/hardware.kicad_sch` contains the current KiCad wiring schematic.

It includes:

- ESP32 30-pin DevKit
- GY-87 IMU
- HW-627 DRV8833 motor-driver module
- Two N20 motors and their connectors
- 2.8-inch ILI9341 SPI TFT connector
- Two adjustable LM2596 buck-converter modules
- 2S battery, BMS, fuse, and main power switch
- Motor noise/rail capacitors and DRV8833 input pulldowns

The intended power tree is:

```text
2S battery → BMS → 3 A fuse → main switch
                            ├─ LM2596 set to 6 V → DRV8833 + motors
                            └─ LM2596 set to 5 V → ESP32 VIN

ESP32 3V3 → GY-87 + TFT VCC + TFT LED
```

The schematic has been drawn. It passes KiCad's Electrical Rules Check (ERC).

### Firmware

The PlatformIO project lives in `firmware/` and currently builds successfully for `esp32dev`.

| File | Current job | State |
| --- | --- | --- |
| `firmware/platformio.ini` | ESP32/Arduino config and Adafruit libraries | working |
| `firmware/include/Pins.h` | GPIO assignments and robot constants | working |
| `firmware/include/balance.h` | PID controller interface and telemetry | working |
| `firmware/src/balance.cpp` | PID calculation, integral clamp, telemetry | working |
| `firmware/include/imu.h` | IMU interface and telemetry | working |
| `firmware/src/imu.cpp` | MPU6050 init, gyro calibration, complementary filter | working |
| `firmware/src/main.cpp` | Two FreeRTOS tasks pinned to ESP32 cores | scaffolded and compiling |
| `firmware/src/motors.cpp` | DRV8833/PWM motor code | intentionally empty |
| `firmware/src/display.cpp` | ILI9341 face code | intentionally empty |

## Current firmware architecture

```text
Core 1 — balance task
every 5 ms / 200 Hz
eventually: IMU → filter → PID → motors

Core 0 — display task
every 100 ms / 10 FPS
eventually: draw/update face
```

The core split exists now. The live balance chain does not yet exist because the motor and display modules have not been written or connected to `main.cpp`.

## Current GPIO plan

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

DRV8833 module
  IN4 → GPIO25
  IN3 → GPIO26
  IN2 → GPIO27
  IN1 → GPIO14
```

The TFT uses 3.3 V for `VCC` and `LED`, matching ESP32 logic levels. Its touch pins are unused in v1.

## Parts plan

The intended v1 parts are:

- ESP32 30-pin DevKit
- GY-87
- 2 × 6 V, 200 RPM N20 metal gear motors
- HW-627 DRV8833 board
- 2.8-inch 240 × 320 ILI9341 SPI TFT
- 2 × LM2596 adjustable buck boards
- 2 × matched 18650 cells in series, a 2S BMS, fuse, and switch
- 34 mm wheels

The parts are being sourced; this is still a software-and-schematic-first phase.

## Important current limitations

- No motor encoders in v1. It can balance by tilt angle but will not accurately know its speed or position, so some wandering is expected.
- The N20s are a compromise for a first working, compact, affordable robot—not unlimited recovery power.
- The IMU axis assumptions are code placeholders until the real module is mounted. Physical testing will decide whether an axis or sign needs reversing.
- The current PID values are starter values. They are not tuned values and should not be treated as physics carved into stone.

## Next actual task

1. Run KiCad ERC and fix genuine unconnected-net errors.
2. Write `motors.cpp`: PWM setup, signed motor commands, and a reliable stop function.
3. Write a minimal `display.cpp`: initialize TFT and draw one static face.
4. Connect `ImuReader`, `BalanceController`, and motors in `balanceTask()`.
5. When hardware arrives, test power rails with a multimeter before connecting anything expensive.

## Rule of the project

The code compiling is not evidence that the robot will balance. It is evidence that the robot has become qualified to fail in a much more interesting way.
