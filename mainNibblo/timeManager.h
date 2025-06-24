#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include "config.h"

#define ONE_HOUR_MILLIS 3600000
#define ONE_HOUR_SECOND 3600

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
  static FeedSchedule schedules[MAX_SCHEDULES];  //cuma bisa 10 waktu dalam 1 jadwal
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
  static void clearAllSchedules();
};

#endif