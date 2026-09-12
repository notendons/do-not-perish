#include "Pins.h"
#include "balance.h"
#include <Arduino.h>

volatile uint32_t balanceCycles=0;

//core 1 function
void balanceTask(void *parameter){
  const TickType_t interval=pdMS_TO_TICKS(Robot::balance_interval_ms);
  TickType_t lastWakeTime=xTaskGetTickCount();

  for(;;){
    balanceCycles++;

    //read IMU
    //calculate PID
    //command motors

    vTaskDelayUntil(&lastWakeTime,interval);
  }
}
//core 0 function
void displayTask(void *parameter){
  const TickType_t interval=pdMS_TO_TICKS(Robot::display_interval_ms);
  TickType_t lastWakeTime=xTaskGetTickCount();
  for(;;){
    Serial.printf(
      "Face: core %d | balance cycles: %lu/n",
      xPortGetCoreID(),
      balanceCycles
    );

    //update display

    vTaskDelayUntil(&lastWakeTime,interval);
  }
}

void setup(){
  Serial.begin(115200);
  delay(500); //give it some time

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

  Serial.println("cores startup complete");
  }


void loop(){
  vTaskDelay(pdMS_TO_TICKS(1000));//do nothing, let the tasks run
}