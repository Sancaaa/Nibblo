#include "alertManager.h"
#include "hardware.h"
#include "telegramHandler.h"


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
  int status = checkLevelState(level, FOOD_WARNING_THRESHOLD, FOOD_CRITICAL_THRESHOLD);

  if (status == 2 && !state.foodCritical) {
    TelegramHandler::sendFoodAlert(level, true);
    state.foodCritical = true;
    state.lastAlertTime = millis();
  }
  else if (status == 1 && !state.foodWarning && !state.foodCritical) {
    TelegramHandler::sendFoodAlert(level, false);
    state.foodWarning = true;
    state.lastAlertTime = millis();
  }
  else if (status == 0 && level > FOOD_WARNING_THRESHOLD + 10) {
    state.foodWarning = false;
    state.foodCritical = false;
  }
}

void AlertManager::checkWaterAlerts(int level) {
  int status = checkLevelState(level, WATER_WARNING_THRESHOLD, WATER_CRITICAL_THRESHOLD);

  if (status == 2 && !state.waterCritical) {
    TelegramHandler::sendWaterAlert(level, true);
    state.waterCritical = true;
    state.lastAlertTime = millis();
  }
  else if (status == 1 && !state.waterWarning && !state.waterCritical) {
    TelegramHandler::sendWaterAlert(level, false);
    state.waterWarning = true;
    state.lastAlertTime = millis();
  }
  else if (status == 0 && level > WATER_WARNING_THRESHOLD + 10) {
    state.waterWarning = false;
    state.waterCritical = false;
  }
}

void AlertManager::checkBatteryAlerts(float percent) {
  if (Hardware::isCriticalBattery() && !state.batteryCritical) {
    TelegramHandler::sendBatteryAlert(percent, true);
    state.batteryCritical = true;
    state.batteryWarning = true; 
    state.lastAlertTime = millis();
  }
  else if (Hardware::isLowBattery() && !state.batteryWarning && !state.batteryCritical) {
    TelegramHandler::sendBatteryAlert(percent, false);
    state.batteryWarning = true;
    state.lastAlertTime = millis();
  }
  else if (percent > LOW_BATTERY_THRESHOLD + 5) {
    state.batteryWarning = false;
    state.batteryCritical = false;
  }
}

int AlertManager::checkLevelState(int level, int warning, int critical){
  if (level <= critical) return 2;
  if (level <= warning) return 1;
  return 0;
}
