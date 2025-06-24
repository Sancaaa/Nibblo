#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include "config.h"

class Hardware;

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
  static void updateRTC();
  static void loadFromRTC();
  static String getDataSummary();
  static int getTotalFeeds();
};

#endif