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
unsigned long lastAlertCheck = 0;

// System status variables
bool systemInitialized = false;
unsigned long bootTime = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n" + String('=', 50));
  Serial.println("🐹 HAMSTER FEEDER SYSTEM v2.0");
  Serial.println("🚀 Booting up...");
  Serial.println(String('=', 50));
  
  bootTime = millis();
  
  // Initialize modules in order
  if (!initializeSystem()) {
    Serial.println("❌ SYSTEM INITIALIZATION FAILED!");
    Serial.println("🔄 Restarting in 10 seconds...");
    delay(10000);
    ESP.restart();
  }
  
  systemInitialized = true;
  unsigned long initTime = millis() - bootTime;
  
  Serial.println(String('=', 50));
  Serial.println("✅ ALL SYSTEMS READY!");
  Serial.printf("⚡ Boot time: %lu ms\n", initTime);
  Serial.printf("💾 Free memory: %u bytes\n", ESP.getFreeHeap());
  Serial.println("🌐 IP: " + WiFi.localIP().toString());
  Serial.println(String('=', 50));
  
  // Send startup notification
  TelegramHandler::sendStartupNotification();
  TelegramHandler::sendDebugInfo("System booted in " + String(initTime) + "ms");
}

bool initializeSystem() {
  // Hardware initialization
  Serial.print("🔧 Initializing hardware... ");
  Hardware::init();
  Serial.println("✅");
  
  // Power management
  Serial.print("⚡ Initializing power manager... ");
  PowerManager::init();
  Serial.println("✅");
  
  // WiFi connection with timeout
  Serial.print("📶 Connecting to WiFi... ");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < 30) {
    delay(1000);
    Serial.print(".");
    wifiAttempts++;
    
    if (wifiAttempts % 10 == 0) {
      Hardware::displayMessage("WiFi: " + String(wifiAttempts) + "/30");
    }
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(" ❌ FAILED");
    Hardware::displayMessage("WiFi Failed!");
    return false;
  }
  Serial.println(" ✅");
  Serial.println("   📶 SSID: " + WiFi.SSID());
  Serial.println("   🌐 IP: " + WiFi.localIP().toString());
  Serial.println("   📶 RSSI: " + String(WiFi.RSSI()) + " dBm");
  
  // Time manager
  Serial.print("🕐 Initializing time manager... ");
  TimeManager::init();
  Serial.println("✅");
  Serial.println("   🕐 Current time: " + TimeManager::getCurrentTimeString());
  
  // Alert manager
  Serial.print("🚨 Initializing alert manager... ");
  AlertManager::init();
  Serial.println("✅");
  
  // Data logger
  Serial.print("📊 Initializing data logger... ");
  DataLogger::init();
  Serial.println("✅");
  Serial.println("   📈 Total feeds: " + String(DataLogger::getTotalFeeds()));
  
  // Telegram handler
  Serial.print("📱 Initializing Telegram handler... ");
  TelegramHandler::init();
  Serial.println("✅");
  
  // Initial sensor reading
  Serial.print("📡 Reading initial sensors... ");
  Hardware::readAllSensors();
  Serial.println("✅");
  Serial.printf("   🔋 Battery: %.1fV (%.0f%%)\n", 
                Hardware::getBatteryVolt(), Hardware::getBatteryPercent());
  Serial.printf("   🍽 Food: %d%%\n", Hardware::getFoodLevel());
  Serial.printf("   💧 Water: %d%%\n", Hardware::getWaterLevel());
  
  return true;
}

void loop() {
  if (!systemInitialized) {
    delay(1000);
    return;
  }
  
  unsigned long currentTime = millis();
  
  // Handle potential millis() overflow (every ~49 days)
  if (currentTime < lastTimeUpdate) {
    resetTimers();
  }
  
  // 1. Time management (every minute)
  if (currentTime - lastTimeUpdate >= TIME_UPDATE_INTERVAL) {
    TimeManager::update();
    lastTimeUpdate = currentTime;
  }
  
  // 2. Telegram bot communication (every 5 seconds)
  if (currentTime - lastBotCheck >= BOT_CHECK_INTERVAL) {
    TelegramHandler::checkMessages();
    lastBotCheck = currentTime;
  }
  
  // 3. Sensor readings (every 5 seconds)
  if (currentTime - lastSensorRead >= SENSOR_READ_INTERVAL) {
    Hardware::readAllSensors();
    lastSensorRead = currentTime;
    
    // Send debug info periodically (every 5 minutes)
    static unsigned long lastDebugSend = 0;
    if (currentTime - lastDebugSend >= 300000) {
      TelegramHandler::sendDebugInfo(
        "Sensors - F:" + String(Hardware::getFoodLevel()) + 
        "% W:" + String(Hardware::getWaterLevel()) + 
        "% B:" + String(Hardware::getBatteryPercent(), 0) + "%"
      );
      lastDebugSend = currentTime;
    }
  }
  
  // 4. Display updates (every 2 seconds)
  if (currentTime - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    Hardware::updateDisplay();
    lastDisplayUpdate = currentTime;
  }
  
  // 5. Alert checking (every 30 seconds)
  if (currentTime - lastAlertCheck >= ALERT_CHECK_INTERVAL) {
    AlertManager::checkAlerts();
    lastAlertCheck = currentTime;
  }
  
  // 6. Core system functions (run every loop with internal timing)
  TimeManager::checkAutoFeedSchedule();
  PowerManager::checkPowerStatus();
  DataLogger::logPeriodicData();
  
  // 7. Watchdog and system health
  checkSystemHealth();
  
  // Small delay to prevent excessive CPU usage
  delay(50);
}

void resetTimers() {
  unsigned long now = millis();
  lastTimeUpdate = now;
  lastBotCheck = now;
  lastSensorRead = now;
  lastDisplayUpdate = now;
  lastAlertCheck = now;
  
  Serial.println("⚠ System timers reset (millis overflow detected)");
  TelegramHandler::sendDebugInfo("System timers reset - millis overflow");
}

void checkSystemHealth() {
  static unsigned long lastHealthCheck = 0;
  static int consecutiveWifiFailures = 0;
  
  if (millis() - lastHealthCheck < 60000) return; // Check every minute
  lastHealthCheck = millis();
  
  // Check WiFi health
  if (WiFi.status() != WL_CONNECTED) {
    consecutiveWifiFailures++;
    Serial.println("⚠ WiFi disconnected (attempt " + String(consecutiveWifiFailures) + ")");
    Hardware::displayMessage("WiFi Error: " + String(consecutiveWifiFailures));
    
    if (consecutiveWifiFailures >= 3) {
      Serial.println("🔄 WiFi reconnection attempt...");
      WiFi.disconnect();
      delay(1000);
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      
      if (consecutiveWifiFailures >= 10) {
        Serial.println("❌ WiFi failed permanently - restarting system");
        TelegramHandler::sendSystemAlert("WiFi failed - system restarting");
        delay(2000);
        ESP.restart();
      }
    }
  } else {
    if (consecutiveWifiFailures > 0) {
      Serial.println("✅ WiFi reconnected after " + String(consecutiveWifiFailures) + " failures");
      TelegramHandler::sendDebugInfo("WiFi reconnected after " + String(consecutiveWifiFailures) + " attempts");
    }
    consecutiveWifiFailures = 0;
  }
  
  // Check memory health
  uint32_t freeHeap = ESP.getFreeHeap();
  if (freeHeap < 5000) { // Less than 5KB free
    Serial.println("⚠ Low memory: " + String(freeHeap) + " bytes");
    TelegramHandler::sendSystemAlert("Low memory warning: " + String(freeHeap) + " bytes");
    
    if (freeHeap < 2000) { // Critical memory
      Serial.println("❌ Critical memory - forcing restart");
      TelegramHandler::sendSystemAlert("Critical memory - system restarting");
      delay(2000);
      ESP.restart();
    }
  }
}