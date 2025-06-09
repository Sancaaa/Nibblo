#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <ESP8266WiFi.h>
#include "config.h"

class PowerManager {
private:
  static bool deepSleepEnabled;
  static unsigned long lastActivity;
  static unsigned long sleepTimeout;
  static bool lowPowerMode;

public:
  static void init();
  static void checkPowerStatus();
  static void updateActivity();
  static void enterDeepSleep(unsigned long seconds);
  static void enterLowPowerMode();
  static void exitLowPowerMode();
};

bool PowerManager::deepSleepEnabled = true;
unsigned long PowerManager::lastActivity = 0;
unsigned long PowerManager::sleepTimeout = 300000; // 5 minutes
bool PowerManager::lowPowerMode = false;

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
    enterDeepSleep(3600); // Sleep for 1 hour
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
  if (deepSleepEnabled && (millis() - lastActivity > sleepTimeout)) {
    Serial.println("💤 Idle timeout - entering sleep mode");
    enterDeepSleep(SLEEP_DURATION_SECONDS);
  }
}

void PowerManager::updateActivity() {
  lastActivity = millis();
}

void PowerManager::enterDeepSleep(unsigned long seconds) {
  // Save state to RTC memory
  DataLogger::saveToRTC();
  
  // Cleanup
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  Hardware::displayMessage("Sleeping...");
  delay(1000);
  
  // Deep sleep
  ESP.deepSleep(seconds * 1000000);
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

#endif
