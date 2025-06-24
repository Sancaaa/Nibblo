#include "config.h"
#include "hardware.h"
#include "powerManager.h"
#include "timeManager.h"
#include "alertManager.h"
#include "dataLogger.h"
#include "telegramHandler.h"

// Global variables
unsigned long lastTimeUpdate = 0;
unsigned long lastBotCheck = 0;
unsigned long lastSensorRead = 0;
unsigned long lastDisplayUpdate = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("🐹 Hamster Feeder System Starting...");
  
  Hardware::init();
  PowerManager::init();
  TimeManager::init();
  AlertManager::init();
  DataLogger::init();
  TelegramHandler::init();
  
  Serial.println("✅ All modules initialized!");
  TelegramHandler::sendStartupNotification();
}

void loop() {
  unsigned long currentTime = millis();

  // 1. Update time (tiap menit)
  if (currentTime - lastTimeUpdate > TIME_UPDATE_INTERVAL) {
    TimeManager::update();
    lastTimeUpdate = currentTime;
  }

  // 2. Check Telegram messages (tiap 1 detik)
  if (currentTime - lastBotCheck > BOT_CHECK_INTERVAL) {
    TelegramHandler::checkMessages();
    lastBotCheck = currentTime;
  }

  // 3. Read sensors (tiap 5 detik)
  if (currentTime - lastSensorRead > SENSOR_READ_INTERVAL) {
    Hardware::readAllSensors();
    lastSensorRead = currentTime;
  }

  // 4. Update display (tiap 2 detik)
  if (currentTime - lastDisplayUpdate > DISPLAY_UPDATE_INTERVAL) {
    Hardware::updateDisplay();
    lastDisplayUpdate = currentTime;
  }

  // 5. Logic lainnya
  AlertManager::checkAlerts();
  TimeManager::checkAutoFeedSchedule();
  PowerManager::checkPowerStatus();
  DataLogger::logPeriodicData();
}
