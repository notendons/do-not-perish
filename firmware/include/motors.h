#pragma once
#include <stdint.h>

class Motors {
    public:
    void begin();
    void set(int lCommand, int rCommand);// each command is [-255,255], used for each wheel
    void drive(int command);//same command on both wheels
    void stop();

    private:
    void setOneMotor(uint8_t in1Channel, uint8_t in2Channel, int command);
};