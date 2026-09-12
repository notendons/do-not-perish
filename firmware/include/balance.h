#pragma once

// A 'struct' is just a data container.
struct BalanceTelemetry{
    float angleDeg;
    float rateDegPerSec;

    float errorDeg;
    float proportional;
    float integral;
    float derivative;

    int motorCommand;
};

// A 'class' groups variables and the functions that act upon them together.
class BalanceController{
    public:
    BalanceController();

    void setGains(float kp, float ki, float kd);
    void setTargetAngle(float targetAngleDeg);
    void reset();

    BalanceTelemetry update(float angleDeg, float rateDegPerSec, float dtSec);
    private:
    float kp_;
    float ki_;
    float kd_;
    float targetAngleDeg_;
    float integralError_;
};