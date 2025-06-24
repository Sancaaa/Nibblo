#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <ESP8266WiFi.h>
#include "config.h"
#include "credential.h"

class Hardware;
class TelegramHandler;

class PowerManager {
private:
  static unsigned long lastActivity;
  static unsigned long sleepTimeout;
  static bool lowPowerMode;
  static bool idleMode;

public:
  static void init();
  static void checkPowerStatus();
  static void updateActivity();
  static void enterLowPowerMode();
  static void exitLowPowerMode();
  static void enterIdleMode();
};

#endif
