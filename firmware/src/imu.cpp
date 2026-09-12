#include <Arduino.h>
#include <Wire.h>
#include "imu.h"
#include "Pins.h"

constexpr float rad_to_deg=180.0f/3.14159265358979323846f;
constexpr float gyro_weight=0.98f;//complementary filter weight

bool ImuReader::begin(){
    ready_=false;
    Wire.begin(Pins::IMU_sda, Pins::IMU_scl);
    Wire.setClock(400000);//400kHz I2C

    if(!mpu_.begin(0x68,&Wire)){
        return false;
    }

    mpu_.setAccelerometerRange(MPU6050_RANGE_4_G);
    mpu_.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu_.setFilterBandwidth(MPU6050_BAND_94_HZ);

    angleDeg_=0.0f;
    gyroBiasDegPerSec_=0.0f;
    ready_=true;
    return true;
}

void ImuReader::calibrateGyro(uint16_t sampleCount){
    if(!ready_||sampleCount==0){
        return;
    }
    float totalGyroRate=0.0f;
    float totalAccelAngle=0.0f;

    for(uint16_t i=0;i<sampleCount;i++){
        sensors_event_t accel, gyro, temp;

        mpu_.getEvent(&accel,&gyro,&temp);

        totalGyroRate+= gyro.gyro.y*rad_to_deg;//assumes Y is parallel to wheel axle
        totalAccelAngle+=atan2f(accel.acceleration.x,accel.acceleration.z)*rad_to_deg;
        delay(2);//wait a bit for the next sample
    }
    gyroBiasDegPerSec_=totalGyroRate/sampleCount;
    angleDeg_=totalAccelAngle/sampleCount;
}

ImuTelemetry ImuReader::update(float dtSec){
    if(!ready_||dtSec<=0.0f||dtSec>0.1f){
        return {0.0f,0.0f,0.0f,0.0f,0.0f,false};
    }
    if (dtSec<=0.0f||dtSec>0.1f){
        dtSec=0.005f;
    }

    sensors_event_t accel, gyro, temp;
    mpu_.getEvent(&accel,&gyro,&temp);

    float accelAngleDeg=atan2f(accel.acceleration.x,accel.acceleration.z)*rad_to_deg;
    float gyroRateDegPerSec=(gyro.gyro.y*rad_to_deg)-gyroBiasDegPerSec_;

    //predict new angle from gyro movement
    float gyroPredictedAngleDeg=angleDeg_+(gyroRateDegPerSec*dtSec);
    //correct long-term gyro drift using gravity
    angleDeg_= (gyroPredictedAngleDeg*gyro_weight)+(accelAngleDeg*(1.0f-gyro_weight));

    return {
        accelAngleDeg,
        angleDeg_,
        gyroRateDegPerSec,
        accel.acceleration.x,
        accel.acceleration.z,
        true
    };
}

void ImuReader::resetFilter(float angleDeg){
    angleDeg_=angleDeg;
}