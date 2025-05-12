#pragma once
#include "config.h"
#include <ESP8266WebServer.h>

extern ESP8266WebServer server;

// Инициализация
void webSetup();

// Обработчики
void handleRoot();
void handleControl();
void handleMoveSteps();

void handleMotorSettings();
void handleSaveMotorSettings();

void handleNetworkSettings();
void handleSaveNetworkSettings();

void handleESPSettings();

// Логи
void addLog(const String &message);