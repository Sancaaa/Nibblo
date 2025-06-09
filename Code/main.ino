#include "config.h"
#include "hardware.h"
#include "powerManager.h"
#include "timeManager.h"
#include "alertManager.h"
#include "dataLog.h"
#include "telegramConnection.h"

// Global variables
unsigned long lastBotCheck = 0;
unsigned long lastSensorRead = 0;
unsigned long lastDisplayUpdate = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("🐹 Hamster Feeder System Starting...");
  
  // Initialize modules in order
  Hardware::init();
  PowerManager::init();
  TimeManager::init();
  AlertManager::init();
  DataLog::init();
  TelegramConnection::init();
  
  Serial.println("✅ All modules initialized successfully!");
  
  // Send startup notification
  TelegramConnection::sendStartupNotification();
}

void loop() {
  unsigned long currentTime = millis();
  
  // Update time (every minute)
  if (currentTime - lastBotCheck > BOT_CHECK_INTERVAL) {
    TimeManager::update();
    lastBotCheck = currentTime;
  }
  
  // Read sensors (every 5 seconds)
  if (currentTime - lastSensorRead > SENSOR_READ_INTERVAL) {
    Hardware::readAllSensors();
    lastSensorRead = currentTime;
  }
  
  // Check Telegram messages (every 1 second)
  if (currentTime - lastBotCheck > BOT_CHECK_INTERVAL) {
    TelegramConnection::checkMessages();
    lastBotCheck = currentTime;
  }
  
  // Update display (every 2 seconds)
  if (currentTime - lastDisplayUpdate > DISPLAY_UPDATE_INTERVAL) {
    Hardware::updateDisplay();
    lastDisplayUpdate = currentTime;
  }
  
  // Check alerts
  AlertManager::checkAlerts();
  
  // Check auto feeding schedule
  TimeManager::checkAutoFeedSchedule();
  
  // Check power management
  PowerManager::checkPowerStatus();
  
  // Log data periodically
  DataLog::logPeriodicData();
  
  delay(100); // Small delay to prevent overwhelming
}