#pragma once
#include <stdint.h>
#include <Adafruit_MPU6050.h>

struct ImuTelemetry{
    float accelAngleDeg;
    float angleDeg;
    float gyroRateDegPerSec; //becomes the d term measurement in PID

    float accelX;
    float accelZ; // Y is along the wheel axle, so it is not part of forward/back tilt

    bool valid;
};

class ImuReader{
    public:
    bool begin();
    //call only while the robot is completely still
    void calibrateGyro(uint16_t sampleCount);
    //call every balance update - normally every 0.005 sec
    ImuTelemetry update(float dtSec);
    void resetFilter(float angleDeg=0.0f);

    private:
    Adafruit_MPU6050 mpu_;
    float angleDeg_;
    float gyroBiasDegPerSec_;
    bool ready_;
};