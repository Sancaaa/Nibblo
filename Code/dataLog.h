#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include "config.h"

struct LogData {
  unsigned long timestamp;
  int foodLevel;
  int waterLevel;
  float batteryVolt;
  int feedCount;
  String lastFeedTime;
};

class DataLogger {
private:
  static LogData currentData;
  static unsigned long lastLogTime;
  static int totalFeeds;
  static String lastFeedTime;

public:
  static void init();
  static void logPeriodicData();
  static void logFeeding(String type, String time);
  static void saveToRTC();
  static void loadFromRTC();
  static String getDataSummary();
  static int getTotalFeeds() { return totalFeeds; }
};

LogData DataLogger::currentData;
unsigned long DataLogger::lastLogTime = 0;
int DataLogger::totalFeeds = 0;
String DataLogger::lastFeedTime = "";

void DataLogger::init() {
  loadFromRTC();
  Serial.println("✅ Data Logger initialized");
}

void DataLogger::logPeriodicData() {
  unsigned long now = millis();
  if (now - lastLogTime > DATA_LOG_INTERVAL) {
    currentData.timestamp = now;
    currentData.foodLevel = Hardware::getFoodLevel();
    currentData.waterLevel = Hardware::getWaterLevel();
    currentData.batteryVolt = Hardware::getBatteryVolt();
    currentData.feedCount = totalFeeds;
    currentData.lastFeedTime = lastFeedTime;
    
    lastLogTime = now;
    
    // Print to serial for debugging
    Serial.printf("LOG: F:%d%% W:%d%% B:%.1fV Feeds:%d\n", 
                  currentData.foodLevel, currentData.waterLevel, 
                  currentData.batteryVolt, currentData.feedCount);
  }
}

void DataLogger::logFeeding(String type, String time) {
  totalFeeds++;
  lastFeedTime = time;
  Serial.println("Feed logged: " + type + " at " + time);
}

void DataLogger::saveToRTC() {
  // Implementation for saving to RTC memory
  // (RTC memory code from previous examples)
}

void DataLogger::loadFromRTC() {
  // Implementation for loading from RTC memory
  // (RTC memory code from previous examples)
}

String DataLogger::getDataSummary() {
  String summary = "📊 DATA SUMMARY\n\n";
  summary += "🍽️ Total feeds: " + String(totalFeeds) + "\n";
  summary += "⏰ Last feed: " + lastFeedTime + "\n";
  summary += "🔋 Battery: " + String(Hardware::getBatteryVolt(), 1) + "V\n";
  summary += "📈 Food: " + String(Hardware::getFoodLevel()) + "%\n";
  summary += "💧 Water: " + String(Hardware::getWaterLevel()) + "%\n";
  return summary;
}

#endif