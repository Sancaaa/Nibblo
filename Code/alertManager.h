#ifndef ALERT_MANAGER_H
#define ALERT_MANAGER_H

#include "config.h"

struct AlertState {
  bool foodWarning = false;
  bool foodCritical = false;
  bool waterWarning = false;
  bool waterCritical = false;
  bool batteryWarning = false;
  bool batteryCritical = false;
  unsigned long lastAlertTime = 0;
};

class AlertManager {
private:
  static AlertState state;
  static unsigned long alertCooldown;

public:
  static void init();
  static void checkAlerts();
  static void checkFoodAlerts(int level);
  static void checkWaterAlerts(int level);
  static void checkBatteryAlerts(float percent);

};

AlertState AlertManager::state;
unsigned long AlertManager::alertCooldown = ALERT_COOLDOWN_MINUTES * 60000;

void AlertManager::init() {
  Serial.println("✅ Alert Manager initialized");
}

void AlertManager::checkAlerts() {
  unsigned long now = millis();
  
  // Check cooldown
  if (now - state.lastAlertTime < alertCooldown) {
    return;
  }
  
  checkFoodAlerts(Hardware::getFoodLevel());
  checkWaterAlerts(Hardware::getWaterLevel());
  checkBatteryAlerts(Hardware::getBatteryPercent());
}

void AlertManager::checkFoodAlerts(int level) {
  if (level <= FOOD_CRITICAL_THRESHOLD && !state.foodCritical) {
    TelegramHandler::sendFoodAlert(level, true);
    state.foodCritical = true;
    state.lastAlertTime = millis();
  }
  else if (level <= FOOD_WARNING_THRESHOLD && !state.foodWarning && !state.foodCritical) {
    TelegramHandler::sendFoodAlert(level, false);
    state.foodWarning = true;
    state.lastAlertTime = millis();
  }
  else if (level > FOOD_WARNING_THRESHOLD + 10) {
    state.foodWarning = false;
    state.foodCritical = false;
  }
}

void AlertManager::checkWaterAlerts(int level) {
  if (level <= WATER_CRITICAL_THRESHOLD && !state.waterCritical) {
    TelegramHandler::sendWaterAlert(level, true);
    state.waterCritical = true;
    state.lastAlertTime = millis();
  }
  else if (level <= WATER_WARNING_THRESHOLD && !state.waterWarning && !state.waterCritical) {
    TelegramHandler::sendWaterAlert(level, false);
    state.waterWarning = true;
    state.lastAlertTime = millis();
  }
  else if (level > WATER_WARNING_THRESHOLD + 10) {
    state.waterWarning = false;
    state.waterCritical = false;
  }
}

void AlertManager::checkBatteryAlerts(float percent) {
  if (percent <= CRITICAL_BATTERY_THRESHOLD && !state.batteryCritical) {
    TelegramHandler::sendBatteryAlert(percent, true);
    state.batteryCritical = true;
    state.batteryWarning = true; 
    state.lastAlertTime = millis();
  }
  else if (percent <= LOW_BATTERY_THRESHOLD && !state.batteryWarning && !state.batteryCritical) {
    TelegramHandler::sendBatteryAlert(percent, false);
    state.batteryWarning = true;
    state.lastAlertTime = millis();
  }
  else if (percent > LOW_BATTERY_THRESHOLD + 5) {
    state.batteryWarning = false;
    state.batteryCritical = false;
  }
}


#endif
