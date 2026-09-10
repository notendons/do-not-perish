#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <PID_v1.h>

// PIN DEFINITIONS 
// Motor X (roll axis)
#define M1_PWM   25
#define M1_DIR1  26
#define M1_DIR2  32

// Motor Y (pitch axis)
#define M2_PWM   27
#define M2_DIR1  14
#define M2_DIR2  4

// Motor Z (yaw axis)
#define M3_PWM   12
#define M3_DIR1  13
#define M3_DIR2  23   
// Shared standby pin for TB6612FNG (set HIGH to enable)
#define DRIVER_STBY 33

// PWM CONFIG 
#define PWM_FREQ   20000   // 20 kHz
#define PWM_RES    8       // 8-bit (0..255)
#define PWM_CH_M1  0
#define PWM_CH_M2  1
#define PWM_CH_M3  2

// GLOBALS 
Adafruit_MPU6050 mpu;

// Filter state
float angleRoll  = 0.0;
float anglePitch = 0.0;

// Gyro zero-rate offsets
float gyroOffsetX = 0.0;
float gyroOffsetY = 0.0;
float gyroOffsetZ = 0.0;

// Complementary filter weight
const float ALPHA = 0.98;
const float RAD2DEG = 57.2957795f;

// Timing
unsigned long lastMicros = 0;

// PID Variables (Roll & Pitch)
double setpointRoll  = 0.0, inputRoll  = 0.0, outputRoll  = 0.0;
double setpointPitch = 0.0, inputPitch = 0.0, outputPitch = 0.0;

// PID Tunings
double Kp = 18.0;
double Ki = 0.0;
double Kd = 0.9;

PID pidRoll (&inputRoll,  &outputRoll,  &setpointRoll,  Kp, Ki, Kd, DIRECT);
PID pidPitch(&inputPitch, &outputPitch, &setpointPitch, Kp, Ki, Kd, DIRECT);

// YAW SCAFFOLDING (Disabled for V1) 
float gyroZ = 0.0;              
double yawRateSetpoint = 0.0;   
double yawRateInput = 0.0;
double yawRateOutput = 0.0;
double Kyaw = 0.0;              // SET TO 0 FOR NOW. Tune later.

// ================= FUNCTION PROTOTYPES =================
void calibrateGyro();
void readIMU(float &roll, float &pitch, float dt);
void driveMotor(int pwmChannel, int dirPin1, int dirPin2, double command);
void setMotorOutputs(double rollCmd, double pitchCmd, double yawCmd);

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Boot: self-balancing cube V1");

  Wire.begin(21, 22);

  if (!mpu.begin()) {
    Serial.println("ERROR: MPU6050 not found.");
    while (1) delay(10);
  }
  Serial.println("MPU6050 OK");

  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("Calibrating gyro — DO NOT MOVE THE CUBE");
  calibrateGyro();
  Serial.print("Gyro offsets (deg/s): ");
  Serial.print(gyroOffsetX); Serial.print(", ");
  Serial.print(gyroOffsetY); Serial.print(", ");
  Serial.println(gyroOffsetZ);

  // Motor pins
 pinMode(M1_DIR1, OUTPUT);
 pinMode(M1_DIR2, OUTPUT);
 pinMode(M2_DIR1, OUTPUT);
 pinMode(M2_DIR2, OUTPUT);
 pinMode(M3_DIR1, OUTPUT);
 pinMode(M3_DIR2, OUTPUT);
 pinMode(DRIVER_STBY, OUTPUT);
 digitalWrite(DRIVER_STBY, HIGH);

  // PWM channels
  ledcSetup(PWM_CH_M1, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_M2, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_M3, PWM_FREQ, PWM_RES);
  ledcAttachPin(M1_PWM, PWM_CH_M1);
  ledcAttachPin(M2_PWM, PWM_CH_M2);
  ledcAttachPin(M3_PWM, PWM_CH_M3);

  // PID setup
  pidRoll.SetMode(AUTOMATIC);
  pidPitch.SetMode(AUTOMATIC);
  pidRoll.SetOutputLimits(-255, 255);
  pidPitch.SetOutputLimits(-255, 255);
  pidRoll.SetSampleTime(5);
  pidPitch.SetSampleTime(5);

  lastMicros = micros();
  Serial.println("Ready.");
}

// ================= MAIN LOOP =================
void loop() {
  unsigned long now = micros();
  float dt = (now - lastMicros) / 1000000.0f;
  lastMicros = now;
  if (dt <= 0.0f || dt > 0.05f) dt = 0.005f;   // Safety clamp

  // 1. Read sensor & filter
  readIMU(angleRoll, anglePitch, dt);

  // 2. Feed roll/pitch PID
  inputRoll  = angleRoll;
  inputPitch = anglePitch;
  pidRoll.Compute();
  pidPitch.Compute();

  // 3. Yaw scaffolding (currently disabled because Kyaw = 0)
  yawRateInput = gyroZ;
  yawRateOutput = Kyaw * (yawRateSetpoint - yawRateInput);

  // 4. Drive motors (Passing yawRateOutput, which is 0 for now)
  setMotorOutputs(outputRoll, outputPitch, yawRateOutput);

  // 5. Debug print (20 Hz)
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 50) {
    lastPrint = millis();
    Serial.print("roll=");  Serial.print(angleRoll, 2);
    Serial.print("  pitch="); Serial.print(anglePitch, 2);
    Serial.print("  outR="); Serial.print(outputRoll, 1);
    Serial.print("  outP="); Serial.println(outputPitch, 1);
  }

  delay(5);
}

// ================= HELPER FUNCTIONS =================

void calibrateGyro() {
  const int N = 1000;
  float sx = 0, sy = 0, sz = 0;
  for (int i = 0; i < N; i++) {
    sensors_event_t a, g, t;
    mpu.getEvent(&a, &g, &t);
    sx += g.gyro.x;
    sy += g.gyro.y;
    sz += g.gyro.z;
    delay(2);
  }
  gyroOffsetX = (sx / N) * RAD2DEG;
  gyroOffsetY = (sy / N) * RAD2DEG;
  gyroOffsetZ = (sz / N) * RAD2DEG;
}

void readIMU(float &roll, float &pitch, float dt) {
  sensors_event_t a, g, t;
  mpu.getEvent(&a, &g, &t);

  // Convert to deg/s
  float gx = g.gyro.x * RAD2DEG - gyroOffsetX;
  float gy = g.gyro.y * RAD2DEG - gyroOffsetY;
  gyroZ = g.gyro.z * RAD2DEG - gyroOffsetZ;   // <-- Fixed: gyroZ is now updated

  // Accelerometer angles
  float ax = a.acceleration.x;
  float ay = a.acceleration.y;
  float az = a.acceleration.z;

  float accelRoll  = atan2f(ay, az) * RAD2DEG;
  float accelPitch = atan2f(-ax, sqrtf(ay * ay + az * az)) * RAD2DEG;

  // Complementary filter
  roll  = ALPHA * (roll  + gx * dt) + (1.0f - ALPHA) * accelRoll;
  pitch = ALPHA * (pitch + gy * dt) + (1.0f - ALPHA) * accelPitch;
}

void driveMotor(int pwmChannel, int dirPin1, int dirPin2, double command) {
  int duty = (int)fabsf(command);
  if (duty > 255) duty = 255;

  if (command > 0) {
    digitalWrite(dirPin1, HIGH);
    digitalWrite(dirPin2, LOW);
  } 
  else if (command < 0) {
    digitalWrite(dirPin1, LOW);
    digitalWrite(dirPin2, HIGH);
  } 
  else {
    digitalWrite(dirPin1, LOW);
    digitalWrite(dirPin2, LOW);
  }
  
  ledcWrite(pwmChannel, duty);
}

void setMotorOutputs(double rollCmd, double pitchCmd, double yawCmd) {
  driveMotor(PWM_CH_M1, M1_DIR1, M1_DIR2, rollCmd);
  driveMotor(PWM_CH_M2, M2_DIR1, M2_DIR2, pitchCmd);
  driveMotor(PWM_CH_M3, M3_DIR1, M3_DIR2, yawCmd);
}