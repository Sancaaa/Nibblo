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

struct RTCData {
  uint32_t marker;
  int totalFeeds;
  char lastFeedTime[32];
};

class DataLogger {
private:
  static LogData currentData;
  static unsigned long lastLogTime;
  static int totalFeeds;
  static String lastFeedTime;
  static bool rtcDirty;
  static unsigned long lastRTCSave;

public:
  static void init();
  static void logPeriodicData();
  static void logFeeding(String type, String time);
  static void saveToRTC();    //RTC digunakan agar data tidak hilang walau device mati
  static void loadFromRTC();
  static String getDataSummary();
  static int getTotalFeeds() { return totalFeeds; }
};

LogData DataLogger::currentData;
unsigned long DataLogger::lastLogTime = 0;
int DataLogger::totalFeeds = 0;
String DataLogger::lastFeedTime = "";
bool DataLogger::rtcDirty = false;
unsigned long DataLogger::lastRTCSave = 0;

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

  updateRTC();
}

void DataLogger::logFeeding(String type, String time) {
  totalFeeds++;
  lastFeedTime = time;
  rtcDirty = true;
  Serial.println("Feed logged: " + type + " at " + time);
}

void DataLogger::updateRTC() {
  if (rtcDirty && millis() - lastRTCSave > 10000) {  // simpan minimal 10 detik setelah update terakhir
    saveToRTC();
    rtcDirty = false;
    lastRTCSave = millis();
  }
}

void DataLogger::saveToRTC() {
  RTCData data;
  data.marker = 0xDEADBEEF;
  data.totalFeeds = totalFeeds;
  lastFeedTime.toCharArray(data.lastFeedTime, sizeof(data.lastFeedTime));

  if (!ESP.rtcUserMemoryWrite(0, (uint32_t*)&data, sizeof(data))) {
    Serial.println("❌ Failed to save to RTC memory");
  } else {
    Serial.println("💾 Saved to RTC memory");
  }
}

void DataLogger::loadFromRTC() {
  RTCData data;
  if (!ESP.rtcUserMemoryRead(0, (uint32_t*)&data, sizeof(data))) {
    Serial.println("❌ Failed to read from RTC memory");
    return;
  }

  if (data.marker != 0xDEADBEEF) {
    Serial.println("⚠️ RTC data invalid - resetting");
    totalFeeds = 0;
    lastFeedTime = "-";
    return;
  }

  totalFeeds = data.totalFeeds;
  lastFeedTime = String(data.lastFeedTime);

  Serial.println("📥 Loaded from RTC memory:");
  Serial.println(" - Total Feeds: " + String(totalFeeds));
  Serial.println(" - Last Feed: " + lastFeedTime);
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