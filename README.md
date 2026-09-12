# do-not-perish

A V1 self-balancing reaction wheel cube. The goal is to balance an ESP32 on a single vertex using three orthogonal momentum wheels. I'm distracting myself from senior year stress.

## architecture

*   **/hardware** — The original KiCad design is for brushed motors. Revise it before using this firmware: replace the TB6612FNG stages with three 3-phase BLDC stages and add three AS5048A encoders.
*   **/firmware** — PlatformIO C++ workspace. It uses SimpleFOC to run three 2804 gimbal motors from AS5048A SPI feedback, with a 500Hz corner-attitude outer loop.

## the hardware rationale

*   **ESP32 DevKit:** A 240MHz dual-core processor is objectively overkill for running three basic PIDs, but it easily handles the 20kHz PWM resolution, I'm already deeply familiar with its C++ environment in PlatformIO, and they are cheap. 
*   **GY-87 10-DOF IMU:** Combines the standard MPU6050 (accel/gyro) needed for the spatial orientation math with an HMC5883L magnetometer and BMP180 barometer. Mainly chosen because it's a reliable, integrated I2C breakout that leaves room to fuse magnetometer data later to fix yaw drift.
*   **3-phase BLDC drivers:** A 2804 is a three-phase brushless motor. Each wheel needs a proper 3-PWM (or 6-PWM) BLDC inverter with 3.3V logic inputs; a TB6612FNG cannot commutate it.
*   **AS5048A encoders:** One encoder per wheel supplies the absolute rotor angle required for field-oriented control. The three encoders share SPI clock/data lines and use separate chip-select pins.
*   **2804 gimbal motors:** Run in FOC torque mode. A q-axis voltage/current command creates rotor torque and therefore flywheel acceleration; it is not an rpm target.

## the physics

The cube stays upright via conservation of angular momentum. Accelerating a flywheel generates a reactive counter-torque on the chassis to counter gravity:
$$\vec{\tau} = I \vec{\alpha} + \vec{\omega} \times (I \vec{\omega})$$

Orientation is tracked as a gravity vector in cube coordinates rather than roll and pitch. At a balanced corner, gravity points along a cube diagonal, $(\pm1,\pm1,\pm1)/\sqrt3$, not along a face normal. The outer controller uses gravity-vector error and body angular rate to command wheel torque:
$$V_q = K_p(\hat g \times g_{target}) - K_d\omega_{tilt} - K_w\omega_{wheel}$$

SimpleFOC turns $V_q$ into phase voltages using the AS5048A rotor angle. This is voltage-torque control; adding phase-current sensing later turns it into calibrated current/torque control.

## current status

*   **Motor logic:** Three sensor-based FOC loops, one per reaction wheel. The old dual-pin TB6612 logic is retired.
*   **Control loop:** A vector PD controller stabilizes the two gravity-defined tilt axes. Yaw about gravity is intentionally not controlled by the accelerometer; it does not generate gravitational toppling torque.

## roadmap

*   **Current sensing and LQR:** Add inline phase-current sensing to make torque repeatable, then identify cube/wheel inertia and move to LQR or full state feedback.
*   **Magnetometer Fusion:** Optional integration of the GY-87's HMC5883L to clamp yaw drift.
*   **Fabrication:** Route the PCB traces and cast the chassis.
*   **Testing:** Tune the PID loops and check step responses.
