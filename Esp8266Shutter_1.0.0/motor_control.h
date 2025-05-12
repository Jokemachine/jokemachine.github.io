#pragma once
#include "config.h"
#include <AccelStepper.h>

extern AccelStepper stepper;
extern bool isMoving;

// Инициализация и обновление
void motorSetup();
void updateMotorSettings();

// Функции управления
void stopMotor();
void rotateForward();
void rotateBackward();
void motorStop();

// Движение на заданное число шагов
void moveStepsForward(long steps);
void moveStepsBackward(long steps);

// Проверка концевых выключателей
void checkEndstops();

// Вспомогательная логика отключения обмоток
void motorDisable();