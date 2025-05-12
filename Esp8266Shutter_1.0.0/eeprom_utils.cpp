#include "eeprom_utils.h"
#include "config.h"
#include "web_interface.h"

void loadSettings() {
  EEPROM.get(0, settings);
  // Базовые проверки
  if (settings.stepsPerRevolution < 1) {
    settings.stepsPerRevolution = 2048;
  }
  if (settings.rpm < 1) {
    settings.rpm = 10;
  }
  if (settings.maxSpeed < 10) {
    settings.maxSpeed = 500;
  }
  if (settings.acceleration < 10) {
    settings.acceleration = 300;
  }
}

void saveSettings() {
  EEPROM.put(0, settings);
  EEPROM.commit();
}

bool connectToWiFi() {
  if (strlen(settings.staSSID) == 0) {
    return false;
  }
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(settings.staSSID, settings.staPassword);

  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 50) {
    delay(200);
    tries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    addLog("Wi-Fi подключён: " + String(settings.staSSID) +
           ", IP=" + WiFi.localIP().toString());
    return true;
  } else {
    addLog("Wi-Fi НЕ подключён: " + String(settings.staSSID));
    return false;
  }
}