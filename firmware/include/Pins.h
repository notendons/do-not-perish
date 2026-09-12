#pragma once
#include <Arduino.h>

namespace Pins{

  constexpr uint8_t IMU_sda=21;
  constexpr uint8_t IMU_scl=22;
  //ILI9341 TFT display pins
  constexpr uint8_t tft_clk=18;
  constexpr uint8_t tft_mosi=23;
  constexpr uint8_t tft_dc=33;
  constexpr uint8_t tft_cs=5;
  constexpr uint8_t tft_rst=17;
  //DRV8833 motor driver
  constexpr uint8_t motor_left_in1=25;
  constexpr uint8_t motor_left_in2=26;
  constexpr uint8_t motor_right_in1=27;
  constexpr uint8_t motor_right_in2=14;
}
namespace Robot{

  //robot behaviour setings
  constexpr uint32_t balance_interval_ms=5;//5ms=200Hz
  constexpr uint32_t display_interval_ms=100;//10FPS

  constexpr int max_motor_pwm=255;
  constexpr float fall_angle_deg=35.0f;
}