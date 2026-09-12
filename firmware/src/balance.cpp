#include <Arduino.h>
#include "balance.h"
#include "Pins.h"

BalanceController::BalanceController()
: kp_(20.0f), ki_(0.0f), kd_(1.5f), targetAngleDeg_(0.0f), integralError_(0.0f)
{
}

void BalanceController::setGains(float kp, float ki, float kd){
    kp_ = kp;
    ki_ = ki;
    kd_ = kd;
}

void BalanceController::setTargetAngle(float targetAngleDeg){
    targetAngleDeg_ = targetAngleDeg;
}

void BalanceController::reset(){
    integralError_ = 0.0f;
}

BalanceTelemetry BalanceController::update(float angleDeg, float rateDegPerSec, float dtSec){
    //safety fallback - the normal value will be 0.005 sec
    if(dtSec<=0.0f||dtSec>0.1f){
        dtSec=0.005F;
    }
    float errorDeg = targetAngleDeg_ - angleDeg;

    integralError_ +=errorDeg*dtSec; //error accumulation
    integralError_=constrain(integralError_,-20.0f,20.0f); //prevent integral windup
    
    float proportional = kp_ * errorDeg;
    float integral = ki_ * integralError_;
    float derivative = -kd_ * rateDegPerSec;//the gyro actually tells us rotational RATE => d term
    
    float rawCommand= proportional + integral + derivative;

    int motorCommand=constrain(static_cast<int>(rawCommand), -Robot::max_motor_pwm, Robot::max_motor_pwm);
    //static_cast<int> converts float rawCommand into an int for the motor controller

    return {
        angleDeg,
        rateDegPerSec,
        errorDeg,
        proportional,
        integral,
        derivative,
        motorCommand
    };
    }