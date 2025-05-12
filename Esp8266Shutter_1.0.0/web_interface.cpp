#include "web_interface.h"
#include "motor_control.h"
#include "eeprom_utils.h"
#include <vector>

ESP8266WebServer server(80);
static String eventLog;

void addLog(const String &message) {
  eventLog += message + "\n";
}

// Универсальный шаблон (шапка + меню + контент)
String getPageTemplate(const String &title, const String &content) {
  bool wifiConnected = (WiFi.status() == WL_CONNECTED);
  bool apActive = ((WiFi.getMode() & WIFI_AP) != 0) && settings.apEnabled;

  String wifiIcon = wifiConnected ? "<span style='color:green;'>Wi-Fi</span>"
                                  : "<span style='color:red;'>Wi-Fi</span>";
  String apIcon   = apActive ? "<span style='color:green;'>AP</span>"
                             : "<span style='color:red;'>AP</span>";

  String html = R"(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>)" + title + R"(</title>
  <style>
    body { margin:0; padding:0; font-family:Arial,sans-serif; }
    .header {
      background:#f0f0f0; padding:10px; text-align:center;
      position:relative; border-bottom:1px solid #ccc;
    }
    .header-icons {
      position:absolute; right:10px; top:10px;
    }
    .sidebar {
      width:200px; background:#fafafa; border-right:1px solid #ccc;
      float:left; padding:10px; box-sizing:border-box; height:calc(100vh - 50px);
    }
    .sidebar a {
      display:block; margin:10px 0; text-decoration:none; color:#333;
      padding:5px; border:1px solid #ccc; text-align:center; background:#eee;
    }
    .sidebar a:hover { background:#ddd; }
    .content { margin-left:200px; padding:20px; }
    .btn {
      display:inline-block; margin:5px; padding:10px; background:#ccc;
      text-decoration:none; color:#000; border:1px solid #999;
    }
    .btn:hover { background:#bbb; }
    input[type="text"], input[type="number"], input[type="password"] {
      width:200px; margin:5px 0; padding:5px;
    }
    button {
      margin:5px 0; padding:6px 12px; cursor:pointer;
    }
  </style>
</head>
<body>
  <div class="header">
    <h2>)" + title + R"(</h2>
    <div class="header-icons">
      )" + wifiIcon + R"( &nbsp; )" + apIcon + R"(
    </div>
  </div>
  <div class="sidebar">
    <a href="/">Домой</a>
    <a href="/motorSettings">Настройки двигателя</a>
    <a href="/networkSettings">Настройки сети</a>
    <a href="/espSettings">Настройки ESP</a>
  </div>
  <div class="content">
)" + content + R"(
  </div>
</body>
</html>
)";

  return html;
}

// ------------------- Главная страница (Home) -------------------
void handleRoot() {
  // Кнопки вперёд/стоп/назад
  // + поля для ввода шагов
  String pageContent = R"(
    <h3>Управление мотором</h3>
    <p>
      <a class="btn" href="/control?action=forward">Вперёд</a>
      <a class="btn" href="/control?action=stop">Стоп</a>
      <a class="btn" href="/control?action=backward">Назад</a>
    </p>
    <hr>
    <h4>Задать количество шагов</h4>
    <form action="/moveSteps" method="GET">
      <label>Шаги вперёд:</label>
      <input type="number" name="forwardSteps" value="0">
      <button type="submit">Поехали</button>
    </form>
    <form action="/moveSteps" method="GET">
      <label>Шаги назад:</label>
      <input type="number" name="backwardSteps" value="0">
      <button type="submit">Поехали</button>
    </form>
  )";

  String html = getPageTemplate("Home", pageContent);
  server.send(200, "text/html", html);
}

// ------------------- Управление (вперёд/назад/стоп) -------------------
void handleControl() {
  String action = server.arg("action");
  if (action == "forward") {
    rotateForward();
    addLog("Мотор: Вращение вперёд (ULN2003)");
  } else if (action == "backward") {
    rotateBackward();
    addLog("Мотор: Вращение назад (ULN2003)");
  } else if (action == "stop") {
    motorStop();
    addLog("Мотор: Стоп");
  }
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

// ------------------- Движение на N шагов -------------------
void handleMoveSteps() {
  long fSteps = 0, bSteps = 0;

  if (server.hasArg("forwardSteps")) {
    fSteps = server.arg("forwardSteps").toInt();
    if (fSteps > 0) {
      moveStepsForward(fSteps);
      addLog("Мотор: " + String(fSteps) + " шагов вперёд");
    }
  }
  if (server.hasArg("backwardSteps")) {
    bSteps = server.arg("backwardSteps").toInt();
    if (bSteps > 0) {
      moveStepsBackward(bSteps);
      addLog("Мотор: " + String(bSteps) + " шагов назад");
    }
  }

  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

// ------------------- Страница «Настройки двигателя» -------------------
void handleMotorSettings() {
  String pageContent;
  String topChecked = settings.useTopEndstop ? "checked" : "";
  String bottomChecked = settings.useBottomEndstop ? "checked" : "";

  pageContent += R"(
    <h3>Параметры двигателя (ULN2003)</h3>
    <form action="/saveMotorSettings" method="GET">
      <label>Шагов на оборот:</label><br>
      <input type="number" name="steps" value=")" + String(settings.stepsPerRevolution) + R"("><br>
      
      <label>Скорость (об/мин):</label><br>
      <input type="number" name="rpm" value=")" + String(settings.rpm) + R"("><br>

      <label>Макс. скорость (шаг/сек):</label><br>
      <input type="number" step="0.1" name="maxSpeed" value=")" + String(settings.maxSpeed) + R"("><br>

      <label>Ускорение (шаг/сек^2):</label><br>
      <input type="number" step="0.1" name="acceleration" value=")" + String(settings.acceleration) + R"("><br>

      <br><h4>Концевые выключатели</h4>
      <label>
        <input type="checkbox" name="useTopEndstop" value="1" )" + topChecked + R"(>
        Использовать верхний концевик
      </label><br>
      <label>
        <input type="checkbox" name="useBottomEndstop" value="1" )" + bottomChecked + R"(>
        Использовать нижний концевик
      </label><br><br>

      <button type="submit">Сохранить</button>
    </form>
    <hr>
    <p>Текущие значения:</p>
    <ul>
      <li>Шагов на оборот: )" + String(settings.stepsPerRevolution) + R"(</li>
      <li>Скорость (об/мин): )" + String(settings.rpm) + R"(</li>
      <li>Макс. скорость (шаг/сек): )" + String(settings.maxSpeed) + R"(</li>
      <li>Ускорение (шаг/сек^2): )" + String(settings.acceleration) + R"(</li>
      <li>Верхний концевик: )" + (settings.useTopEndstop ? "Вкл" : "Выкл") + R"(</li>
      <li>Нижний концевик: )" + (settings.useBottomEndstop ? "Вкл" : "Выкл") + R"(</li>
    </ul>
  )";

  String html = getPageTemplate("Настройки двигателя", pageContent);
  server.send(200, "text/html", html);
}

// ------------------- Сохранение настроек двигателя -------------------
void handleSaveMotorSettings() {
  if (server.hasArg("steps")) {
    settings.stepsPerRevolution = server.arg("steps").toInt();
  }
  if (server.hasArg("rpm")) {
    settings.rpm = server.arg("rpm").toInt();
  }
  if (server.hasArg("maxSpeed")) {
    settings.maxSpeed = server.arg("maxSpeed").toFloat();
  }
  if (server.hasArg("acceleration")) {
    settings.acceleration = server.arg("acceleration").toFloat();
  }

  settings.useTopEndstop = server.hasArg("useTopEndstop");
  settings.useBottomEndstop = server.hasArg("useBottomEndstop");

  updateMotorSettings();
  saveSettings();
  addLog("Настройки двигателя (ULN2003) сохранены");

  server.sendHeader("Location", "/motorSettings");
  server.send(302, "text/plain", "");
}

// ------------------- Страницы «Настройки сети» и «ESP» (без изменений) -------------------
void handleNetworkSettings() {
  int n = WiFi.scanNetworks();
  std::vector<String> ssidList;
  for (int i = 0; i < n; i++) {
    ssidList.push_back(WiFi.SSID(i));
  }

  String ssidSelect = "<select name=\"staSSID\">";
  ssidSelect += "<option value=\"\">(не выбрано)</option>";
  for (auto &s : ssidList) {
    ssidSelect += "<option value=\"" + s + "\">" + s + "</option>";
  }
  ssidSelect += "</select>";

  String apChecked = settings.apEnabled ? "checked" : "";

  String pageContent = R"(
    <h3>Подключение к Wi-Fi</h3>
    <form action="/saveNetworkSettings" method="GET">
      <label>Выбор сети Wi-Fi:</label><br>
  )";
  pageContent += ssidSelect;
  pageContent += R"(<br>
      <label>Пароль к сети:</label><br>
      <input type="password" name="staPassword" value=""><br>
      <br>
      <h3>Точка доступа (AP)</h3>
      <label>
        <input type="checkbox" name="apEnabled" value="1" )" + apChecked + R"(>
        Включить точку доступа
      </label>
      <br><br>
      <button type="submit">Сохранить</button>
    </form>
    <hr>
  )";

  String html = getPageTemplate("Настройки сети", pageContent);
  server.send(200, "text/html", html);
}

void handleSaveNetworkSettings() {
  if (server.hasArg("staSSID")) {
    String newSSID = server.arg("staSSID");
    newSSID.toCharArray(settings.staSSID, sizeof(settings.staSSID));
  }
  if (server.hasArg("staPassword")) {
    String newPass = server.arg("staPassword");
    newPass.toCharArray(settings.staPassword, sizeof(settings.staPassword));
  }
  if (server.hasArg("apEnabled") && server.arg("apEnabled") == "1") {
    settings.apEnabled = true;
  } else {
    settings.apEnabled = false;
  }
  
  saveSettings();
  addLog("Настройки сети сохранены, сеть: " + String(settings.staSSID));

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPdisconnect(true);
  if (settings.apEnabled) {
    WiFi.softAP(settings.apSSID, settings.apPassword);
    addLog("AP включён: " + String(settings.apSSID));
  } else {
    addLog("AP выключен");
  }

  bool ok = connectToWiFi();
  if (!ok) {
    addLog("Не удалось подключиться к Wi-Fi: " + String(settings.staSSID));
  }

  server.sendHeader("Location", "/networkSettings");
  server.send(302, "text/plain", "");
}

void handleESPSettings() {
  String pageContent;
  pageContent += R"(
    <h3>Информация об устройстве (Прошивка 1.2.0)</h3>
    <p>Здесь можно посмотреть логи и текущее состояние ESP.</p>
    <hr>
    <h3>Логи событий</h3>
    <pre>
  )";
  pageContent += eventLog;
  pageContent += "</pre>";

  String html = getPageTemplate("Настройки ESP", pageContent);
  server.send(200, "text/html", html);
}

void webSetup() {
  server.on("/", handleRoot);
  server.on("/control", handleControl);
  server.on("/moveSteps", handleMoveSteps);

  server.on("/motorSettings", handleMotorSettings);
  server.on("/saveMotorSettings", handleSaveMotorSettings);

  server.on("/networkSettings", handleNetworkSettings);
  server.on("/saveNetworkSettings", handleSaveNetworkSettings);

  server.on("/espSettings", handleESPSettings);

  server.begin();
  addLog("Web-сервер запущен на порту 80 (Прошивка 1.2.0)");
}