#include "Pins.h"
#include "balance.h"
#include "imu.h"
#include "motors.h"

#include <Arduino.h>
#include <math.h>

ImuReader imu;
BalanceController balance;
Motors motors;

volatile uint32_t balanceCycles=0;
volatile int latestMotorCommand=0;
volatile bool balanceArmed=false;

//core 1 function
void balanceTask(void *parameter){
  const TickType_t interval=pdMS_TO_TICKS(Robot::balance_interval_ms);

  const float dtSec= Robot::balance_interval_ms/1000.0f;
  TickType_t lastWakeTime=xTaskGetTickCount();

  for(;;){
    int motorCommand=0;

    if(balanceArmed){
      ImuTelemetry imuData=imu.update(dtSec); //read IMU
      if(!imuData.valid){
        balanceArmed=false;
        Serial.println("IMU error: balance disabled");
      } else if (fabsf(imuData.angleDeg) > Robot::fall_angle_deg) {
        balance.reset();
        balanceArmed=false;
        Serial.println("fall detected: balance disabled");
      } else {
        BalanceTelemetry balanceData= balance.update(imuData.angleDeg, imuData.gyroRateDegPerSec, dtSec);// calculate PID
        motorCommand=balanceData.motorCommand;
      }
    }

    motors.drive(motorCommand);//command motors

    latestMotorCommand=motorCommand;
    balanceCycles++;

    vTaskDelayUntil(&lastWakeTime,interval);
  }
}
//core 0 function
void displayTask(void *parameter){
  const TickType_t interval=pdMS_TO_TICKS(Robot::display_interval_ms);
  TickType_t lastWakeTime=xTaskGetTickCount();
  for(;;){
    Serial.printf(
      "Face: core %d | cycles: %lu | armed: %s | motor: %d\n",
      xPortGetCoreID(),
      balanceCycles,
      balanceArmed ? "yes":"no",
      latestMotorCommand
    );

    //update display

    vTaskDelayUntil(&lastWakeTime,interval);
  }
}

void setup(){
  Serial.begin(115200);
  delay(500); //give it some time

  //motor pins are config first, the explicitly turned off
  motors.begin();
  motors.stop();

  if(!imu.begin()){
    Serial.println("GY-87 not found. Motors locked.");

    while(true){
      motors.stop();
      delay(1000);
    }
  }
  Serial.println("keep the cube still: calibration in progress...");
  delay(1000);
  imu.calibrateGyro(500);//takes 500 samples

  balance.setTargetAngle(0.0f);
  balance.reset();
  balanceArmed=true;

  //high-priority balance task is on core 1
  xTaskCreatePinnedToCore(
    balanceTask,//func to run
    "balance",//name of task
    4096,//stack size in bytes
    nullptr,//no task parameter yet
    4, //priority
    nullptr, //no task handle yet
    1//core 1
  );
  //low-priority display task is on core 0
  xTaskCreatePinnedToCore(
    displayTask,
    "display",
    4096,
    nullptr,
    1, 
    nullptr, 
    0//core 0
  );

  Serial.println("tasks started.");
  }


void loop(){
  vTaskDelay(pdMS_TO_TICKS(1000));//do nothing, let the tasks run
}