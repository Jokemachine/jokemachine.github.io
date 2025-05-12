#pragma once
#include <ESP8266WiFi.h>
#include <AccelStepper.h>

// --------------------- Двигатель (ULN2003 + 28BYJ-48) ---------------------
// Пины IN1, IN2, IN3, IN4 на ULN2003
// Пример: {5,4,0,2} (D1, D2, D3, D4)
const uint8_t MOTOR_PINS[4] = {5, 4, 0, 2}; // GPIO5, GPIO4, GPIO0, GPIO2

// --------------------- Датчики Холла (концевики) ---------------------
const uint8_t TOP_ENDSTOP_PIN    = 14; // GPIO14 (D5)
const uint8_t BOTTOM_ENDSTOP_PIN = 12; // GPIO12 (D6)

// --------------------- Настройки (EEPROM) ---------------------
struct Settings {
  // Сеть
  char apSSID[32] = "ESP_Shutter";
  char apPassword[32] = "12345678";
  char staSSID[32] = "";
  char staPassword[32] = "";
  
  // Двигатель
  int stepsPerRevolution = 2048; // Шагов на оборот
  int rpm = 10;                  // Скорость (об/мин)
  float maxSpeed = 500;          // Макс. скорость (шаг/сек)
  float acceleration = 300;      // Ускорение (шаг/сек^2)

  // Концевики
  bool useTopEndstop = false;
  bool useBottomEndstop = false;

  // Точка доступа
  bool apEnabled = true;
};

extern Settings settings;