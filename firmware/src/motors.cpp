#include <Arduino.h>
#include "motors.h"
#include "Pins.h"

namespace{
    constexpr uint8_t lIn1Channel=0;
    constexpr uint8_t lIn2Channel=1;
    constexpr uint8_t rIn1Channel=2;
    constexpr uint8_t rIn2Channel=3;

    constexpr uint32_t motorPWMFreqHz=20000;
    constexpr uint32_t motorPWMResBits=8;
}

void Motors::begin(){
   //create 4 PWM channels, each with a range of 0-255
   ledcSetup(lIn1Channel, motorPWMFreqHz, motorPWMResBits);
   ledcSetup(lIn2Channel, motorPWMFreqHz, motorPWMResBits);
   ledcSetup(rIn1Channel, motorPWMFreqHz, motorPWMResBits);
   ledcSetup(rIn2Channel, motorPWMFreqHz, motorPWMResBits);


   // Connect each PWM channel to its physical ESP32 GPIO.
   ledcAttachPin(Pins::motor_left_in1, lIn1Channel);
   ledcAttachPin(Pins::motor_left_in2, lIn2Channel);
   ledcAttachPin(Pins::motor_right_in1, rIn1Channel);
   ledcAttachPin(Pins::motor_right_in2, rIn2Channel);

   stop();
}

void Motors::set(int lCommand, int rCommand){
    setOneMotor(lIn1Channel, lIn2Channel, lCommand);
    setOneMotor(rIn1Channel, rIn2Channel, rCommand);
}

void Motors::drive(int command){
    set(command, command);
}

void Motors::stop(){
    set(0,0);
}

void Motors::setOneMotor(uint8_t in1Channel, uint8_t in2Channel, int command){
    command=constrain(command, -Robot::max_motor_pwm, Robot::max_motor_pwm);

    if(command>0){
        ledcWrite(in1Channel,command);
        ledcWrite(in2Channel, 0);
    } else if (command<0){
        ledcWrite(in1Channel,0);
        ledcWrite(in2Channel,-command);
    } else {
        ledcWrite(in1Channel,0);
        ledcWrite(in2Channel,0);
    }
}