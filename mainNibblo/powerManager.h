#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <ESP8266WiFi.h>
#include "config.h"
#include "credential.h"

class Hardware;

class PowerManager {
private:
  static unsigned long lastActivity;
  static unsigned long sleepTimeout;
  static bool lowPowerMode;
  static bool idleMode;

public:
  static void init();
  static void checkPowerStatus();
  static void updateActivity();
  static void enterLowPowerMode();
  static void exitLowPowerMode();
  static void enterIdleMode();
};

bool PowerManager::lowPowerMode = false;
bool PowerManager::idleMode = false;
unsigned long PowerManager::lastActivity = 0;
unsigned long PowerManager::sleepTimeout = 300000; // 5 menit

void PowerManager::init() {
  lastActivity = millis();
  Serial.println("✅ Power Manager initialized");
}

void PowerManager::checkPowerStatus() {
  float batteryPercent = Hardware::getBatteryPercent();
  
  // Critical battery - emergency sleep
  if (batteryPercent < CRITICAL_BATTERY_THRESHOLD) {
    Serial.println("⚠️ Critical battery - entering emergency sleep");
    TelegramHandler::sendBatteryAlert(batteryPercent, true);
    delay(2000);
    enterIdleMode();
  }
  
  // Low battery - enter power saving mode
  else if (batteryPercent < LOW_BATTERY_THRESHOLD && !lowPowerMode) {
    Serial.println("⚠️ Low battery - entering power saving mode");
    enterLowPowerMode();
  }
  
  // Normal operation
  else if (batteryPercent > LOW_BATTERY_THRESHOLD + 5 && lowPowerMode) {
    Serial.println("✅ Battery recovered - exiting power saving mode");
    exitLowPowerMode();
  }
  
  // Idle sleep check
  if (millis() - lastActivity > sleepTimeout) {
    Serial.println("💤 Idle timeout - entering sleep mode");
    enterIdleMode();
  }
}

void PowerManager::updateActivity() {
  lastActivity = millis();

    //matiin lalu nyalain wifi
    if (idleMode) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    idleMode = false;
    Serial.println("⚡ Waking up from idle mode...");
  }
}

void PowerManager::enterLowPowerMode() {
  lowPowerMode = true;
  // Reduce WiFi power
  WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
}

void PowerManager::exitLowPowerMode() {
  lowPowerMode = false;
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
}

void PowerManager::enterIdleMode() {
  idleMode = true;
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  Hardware::displayMessage("Idle mode");
  Serial.println("🛑 WiFi OFF to save power");
}
#endif
