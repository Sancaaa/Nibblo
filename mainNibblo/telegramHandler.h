#ifndef TELEGRAM_HANDLER_H
#define TELEGRAM_HANDLER_H

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "config.h"
#include "credential.h"

// Forward declarations
class Hardware;
class TimeManager;
class DataLogger;
class PowerManager;

class TelegramHandler {
private:
  static WiFiClientSecure secured_client;
  static UniversalTelegramBot bot;
  static unsigned long lastCheckTime;
  static bool waitingForTimeInput;
  static String pendingChatId;
  
  // Internal methods
  static void handleNewMessages(int numNewMessages);
  static void sendMenuKeyboard(String chat_id);
  static void sendTimeMenuKeyboard(String chat_id);
  static void sendSystemMenuKeyboard(String chat_id);
  static void processTimeInput(String chat_id, String text);
  static void processCommand(String chat_id, String text);
  static String formatStatusMessage();
  static String formatSystemInfo();
  static void sendMessage(String chat_id, String message, String parseMode = "");
  static void sendMessageWithKeyboard(String chat_id, String message, String keyboard, String parseMode = "Markdown");

public:
  static void init();
  static void checkMessages();
  
  // Notification methods
  static void sendStartupNotification();
  static void sendAutoFeedNotification(String time);
  static void sendFeedingResult(bool success, String reason = "");
  static void sendFoodAlert(int level, bool critical);
  static void sendWaterAlert(int level, bool critical);
  static void sendBatteryAlert(float percent, bool critical);
  static void sendSystemAlert(String message);
  
  // Utility methods
  static void sendLogs(String logs);
  static void sendDebugInfo(String info);
  static bool isAuthorizedUser(String chat_id);
};

#endif