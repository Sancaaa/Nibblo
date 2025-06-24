#ifndef ALERT_MANAGER_H
#define ALERT_MANAGER_H

#include "config.h"

//foward declaration
class Hardware;
class TelegramHandler;

struct AlertState {
  bool foodWarning = false;
  bool foodCritical = false;
  bool waterWarning = false;
  bool waterCritical = false;
  bool batteryWarning = false;
  bool batteryCritical = false;
  unsigned long lastAlertTime = 0;
};

class AlertManager {
private:
  static AlertState state;
  static unsigned long alertCooldown;

public:
  static void init();
  static void checkAlerts();
  static void checkFoodAlerts(int level);
  static void checkWaterAlerts(int level);
  static void checkBatteryAlerts(float percent);
  static int checkLevelState(int level, int warning, int critical);
};

#endif
