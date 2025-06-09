#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include "config.h"

struct FeedSchedule {
  String time;
  bool enabled;
  bool executed;
  int lastDay;
};

class TimeManager {
private:
  static WiFiUDP ntpUDP;
  static NTPClient timeClient;
  static FeedSchedule schedules[5];
  static int scheduleCount;
  static unsigned long lastTimeSync;

public:
  static void init();
  static void update();
  static void syncTime();
  static String getCurrentTime();
  static String getCurrentTimeString();
  static void checkAutoFeedSchedule();
  static void addSchedule(String time, bool enabled = true);
  static void removeSchedule(int index);
  static String getScheduleList();
  static bool isValidTimeFormat(String time);
};

WiFiUDP TimeManager::ntpUDP;
NTPClient TimeManager::timeClient(ntpUDP, "pool.ntp.org", 7*3600, 60000);
FeedSchedule TimeManager::schedules[5];
int TimeManager::scheduleCount = 0;
unsigned long TimeManager::lastTimeSync = 0;

void TimeManager::init() {
  timeClient.begin();
  
  // Add default schedules
  addSchedule("08:00", true);
  addSchedule("18:00", true);
  
  syncTime();
  Serial.println("✅ Time Manager initialized");
}

void TimeManager::update() {
  // Sync time every hour
  if (millis() - lastTimeSync > 3600000) {
    syncTime();
  }
}

void TimeManager::syncTime() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Syncing time...");
    if (timeClient.update()) {
      lastTimeSync = millis();
      Serial.println("Time synced: " + getCurrentTimeString());
    }
  }
}

String TimeManager::getCurrentTime() {
  time_t rawTime = timeClient.getEpochTime();
  struct tm * timeInfo = localtime(&rawTime);
  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", timeInfo->tm_hour, timeInfo->tm_min);
  return String(timeStr);
}

String TimeManager::getCurrentTimeString() {
  time_t rawTime = timeClient.getEpochTime();
  struct tm * timeInfo = localtime(&rawTime);
  char timeStr[20];
  sprintf(timeStr, "%02d:%02d:%02d %02d/%02d/%04d", 
          timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec,
          timeInfo->tm_mday, timeInfo->tm_mon + 1, timeInfo->tm_year + 1900);
  return String(timeStr);
}

void TimeManager::checkAutoFeedSchedule() {
  String currentTime = getCurrentTime();
  time_t rawTime = timeClient.getEpochTime();
  struct tm * timeInfo = localtime(&rawTime);
  int currentDay = timeInfo->tm_mday;
  
  for (int i = 0; i < scheduleCount; i++) {
    if (schedules[i].enabled && 
        schedules[i].time == currentTime &&
        (!schedules[i].executed || schedules[i].lastDay != currentDay)) {
      
      // Execute auto feed
      if (Hardware::getFoodLevel() > 10 && Hardware::getBatteryPercent() > 15) {
        Hardware::feedHamster();
        DataLogger::logFeeding("AUTO", schedules[i].time);
        TelegramHandler::sendAutoFeedNotification(schedules[i].time);
        
        schedules[i].executed = true;
        schedules[i].lastDay = currentDay;
      }
    }
    
    // Reset execution flag for new day
    if (schedules[i].lastDay != currentDay) {
      schedules[i].executed = false;
    }
  }
}

void TimeManager::addSchedule(String time, bool enabled) {
  if (scheduleCount < 5 && isValidTimeFormat(time)) {
    schedules[scheduleCount].time = time;
    schedules[scheduleCount].enabled = enabled;
    schedules[scheduleCount].executed = false;
    schedules[scheduleCount].lastDay = 0;
    scheduleCount++;
  }
}

bool TimeManager::isValidTimeFormat(String time) {
  if (time.length() != 5 || time.charAt(2) != ':') return false;
  int hour = time.substring(0, 2).toInt();
  int minute = time.substring(3, 5).toInt();
  return (hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59);
}

#endif