# do-not-perish

A V1 self-balancing reaction wheel cube. The goal is to balance an ESP32 on a single vertex using three orthogonal momentum wheels. I'm distracting myself from senior year stress.

## architecture

*   **/hardware** — KiCad schematics. ESP32 DevKit, GY-87 10-DOF IMU, two TB6612FNG dual H-bridges, and three AR0959 DC motors. Power is 12V raw stepped down to 3.3V logic.
*   **/firmware** — PlatformIO C++ workspace. Reads the IMU and outputs 20kHz PWM.

## the hardware rationale

*   **ESP32 DevKit:** A 240MHz dual-core processor is objectively overkill for running three basic PIDs, but it easily handles the 20kHz PWM resolution, I'm already deeply familiar with its C++ environment in PlatformIO, and they are cheap. 
*   **GY-87 10-DOF IMU:** Combines the standard MPU6050 (accel/gyro) needed for the spatial orientation math with an HMC5883L magnetometer and BMP180 barometer. Mainly chosen because it's a reliable, integrated I2C breakout that leaves room to fuse magnetometer data later to fix yaw drift.
*   **TB6612FNG Motor Drivers:** A massive upgrade over ancient, inefficient L298N drivers. They use MOSFETs instead of bipolar transistors, meaning significantly less voltage drop, no massive heatsinks required, and they cleanly handle high-frequency PWM for smooth torque control.
*   **AR0959 DC Motors:** Standard, compact brushed motors that are easy to drive, mount, and source. 

## the physics

The cube stays upright via conservation of angular momentum. Accelerating a flywheel generates a reactive counter-torque on the chassis to counter gravity:
$$\vec{\tau} = I \vec{\alpha} + \vec{\omega} \times (I \vec{\omega})$$

Orientation is tracked via the GY-87. The firmware samples the motion sensor and applies a complementary filter at a 5ms interval ($\Delta t = 0.005$) to estimate spatial angles:
$$\theta_{t} = \alpha(\theta_{t-1} + \omega_{gyro} \Delta t) + (1-\alpha)\theta_{accel}$$

## current status

*   **Motor Logic:** Firmware updated for dual-pin directional logic (`DIR1`/`DIR2`) required by the TB6612FNG drivers. Supports forward, reverse, and coasting.
*   **Control Loop:** Three independent PIDs handling roll, pitch, and yaw. 

## roadmap

*   **LQR Controller:** State-space modeling is parked until the physical parameters (mass distribution, inertia tensor) can be measured from the actual build.
*   **Magnetometer Fusion:** Optional integration of the GY-87's HMC5883L to clamp yaw drift.
*   **Fabrication:** Route the PCB traces and cast the chassis.
*   **Testing:** Tune the PID loops and check step responses.
