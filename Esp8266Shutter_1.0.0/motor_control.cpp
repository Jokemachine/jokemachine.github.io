#include "motor_control.h"

// Инициализируем AccelStepper в режиме FULL4WIRE
// Порядок: (pin1, pin3, pin2, pin4), обычно AccelStepper так требует
AccelStepper stepper(AccelStepper::FULL4WIRE,
                     MOTOR_PINS[0], MOTOR_PINS[2],
                     MOTOR_PINS[1], MOTOR_PINS[3]);

bool isMoving = false;

void motorSetup() {
  // Настраиваем пины концевиков
  pinMode(TOP_ENDSTOP_PIN, INPUT_PULLUP);
  pinMode(BOTTOM_ENDSTOP_PIN, INPUT_PULLUP);

  stepper.setAcceleration(settings.acceleration);
  stepper.setMaxSpeed(settings.maxSpeed);

  // Установим базовую скорость (в шагах/сек) из RPM:
  stepper.setSpeed(settings.rpm * (settings.stepsPerRevolution / 60.0));
}

// Обновление настроек (accel, maxSpeed, speed)
void updateMotorSettings() {
  stepper.setAcceleration(settings.acceleration);
  stepper.setMaxSpeed(settings.maxSpeed);
  stepper.setSpeed(settings.rpm * (settings.stepsPerRevolution / 60.0));
}

// После завершения — отключим все обмотки (LOW)
void motorDisable() {
  // Просто выставим все моторные пины LOW
  for (int i = 0; i < 4; i++) {
    digitalWrite(MOTOR_PINS[i], LOW);
  }
}

// Плавная остановка
void stopMotor() {
  stepper.stop(); // попросим AccelStepper остановиться плавно
  isMoving = false;
  // Когда реально distanceToGo() станет 0, в loop() вызываем motorDisable()
}

// --------------------------------------------------
// Вращение вперёд/назад без ограничения (большое число)
// --------------------------------------------------
void rotateForward() {
  stepper.setSpeed(settings.rpm * (settings.stepsPerRevolution / 60.0));
  stepper.moveTo(stepper.currentPosition() + 1000000); // большое число
  isMoving = true;
}

void rotateBackward() {
  stepper.setSpeed(-1 * settings.rpm * (settings.stepsPerRevolution / 60.0));
  stepper.moveTo(stepper.currentPosition() - 1000000);
  isMoving = true;
}

// Полная остановка "по кнопке"
void motorStop() {
  stopMotor();
}

// --------------------------------------------------
// Движение на N шагов вперёд/назад
// --------------------------------------------------
void moveStepsForward(long steps) {
  if (steps <= 0) return;
  stepper.setSpeed(settings.rpm * (settings.stepsPerRevolution / 60.0));
  stepper.moveTo(stepper.currentPosition() + steps);
  isMoving = true;
}

void moveStepsBackward(long steps) {
  if (steps <= 0) return;
  stepper.setSpeed(-1 * settings.rpm * (settings.stepsPerRevolution / 60.0));
  stepper.moveTo(stepper.currentPosition() - steps);
  isMoving = true;
}

// --------------------------------------------------
// Проверка концевиков в loop()
// --------------------------------------------------
void checkEndstops() {
  if (!settings.useTopEndstop && !settings.useBottomEndstop) {
    return; // ничего не делаем
  }

  bool topTriggered = (digitalRead(TOP_ENDSTOP_PIN) == LOW);    // Холл даёт LOW при срабатывании
  bool bottomTriggered = (digitalRead(BOTTOM_ENDSTOP_PIN) == LOW);

  // Текущая скорость
  float spd = stepper.speed(); // >0 вперед, <0 назад

  // Если двигаемся вперёд и верхний включён и сработал
  if (settings.useTopEndstop && topTriggered && spd > 0) {
    stopMotor();
  }
  // Если двигаемся назад и нижний включён и сработал
  if (settings.useBottomEndstop && bottomTriggered && spd < 0) {
    stopMotor();
  }
}