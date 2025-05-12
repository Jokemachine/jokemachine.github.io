#include "config.h"
#include "motor_control.h"
#include "web_interface.h"
#include "eeprom_utils.h"

Settings settings;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("Booting... (FW 1.2.0, ULN2003)");

  EEPROM.begin(sizeof(Settings));
  loadSettings();

  // AP + STA
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(settings.apSSID, settings.apPassword);
  Serial.print("AP started: ");
  Serial.println(settings.apSSID);

  connectToWiFi();

  // Инициализация шагового
  motorSetup();

  // Веб-интерфейс
  webSetup();
}

void loop() {
  server.handleClient();

  // Проверяем концевики
  checkEndstops();

  // Если достигли цели и isMoving = true, останавливаемся
  if (stepper.distanceToGo() == 0 && isMoving) {
    stopMotor();
    // В этот момент stopMotor() установит isMoving=false,
    // а при следующем заходе в loop() distanceToGo=0, isMoving=false,
    // мы можем вызвать motorDisable().
    // Однако AccelStepper сам может вызвать stopMotor() плавно,
    // поэтому дополним логику:
  }

  // Если мы уже не двигаемся (isMoving == false), можно отключить обмотки
  // Но, чтобы отключить точно после достижения 0:
  if (!isMoving && stepper.distanceToGo() == 0) {
    motorDisable();
  }

  stepper.run();
}