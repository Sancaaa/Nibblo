#include "telegramHandler.h"
#include "hardware.h"
#include "timeManager.h"
#include "dataLogger.h"
#include "powerManager.h"

// Static variables
WiFiClientSecure TelegramHandler::secured_client;
UniversalTelegramBot TelegramHandler::bot(BOT_TOKEN, secured_client);
unsigned long TelegramHandler::lastCheckTime = 0;
bool TelegramHandler::waitingForTimeInput = false;
String TelegramHandler::pendingChatId = "";

void TelegramHandler::init() {
  secured_client.setInsecure();
  Serial.println("✅ Telegram Handler initialized");
}

void TelegramHandler::checkMessages() {
  if (WiFi.status() != WL_CONNECTED) return;
  
  if (millis() - lastCheckTime > BOT_CHECK_INTERVAL) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastCheckTime = millis();
  }
}

void TelegramHandler::handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    
    // Security check
    if (!isAuthorizedUser(chat_id)) {
      sendMessage(chat_id, "❌ Unauthorized access");
      continue;
    }
    
    text.trim();
    text.toLowerCase();
    
    Serial.println("📩 Telegram: [" + text + "] from " + chat_id);
    PowerManager::updateActivity(); // Update activity for power management
    
    if (waitingForTimeInput && chat_id == pendingChatId) {
      processTimeInput(chat_id, text);
    } else {
      processCommand(chat_id, text);
    }
  }
}

void TelegramHandler::processCommand(String chat_id, String text) {
  if (text == "/start" || text == "/menu") {
    sendMenuKeyboard(chat_id);
    
  } else if (text == "/status" || text == "📊 status") {
    sendMessage(chat_id, formatStatusMessage());
    
  } else if (text == "/makan" || text == "🍽 feed now") {
    Hardware::readAllSensors(); // Update sensor readings
    bool success = Hardware::feedHamster();
    if (success) {
      DataLogger::logFeeding("MANUAL", TimeManager::getCurrentTime());
      sendFeedingResult(true);
    } else {
      sendFeedingResult(false, "Food level too low or battery critical");
    }
    
  } else if (text == "/info makan" || text == "🍽 food info") {
    Hardware::readAllSensors();
    String msg = "📦 Food Status\n";
    msg += "Level: " + String(Hardware::getFoodLevel()) + "%\n";
    msg += "Total feeds: " + String(DataLogger::getTotalFeeds()) + "\n";
    if (Hardware::getFoodLevel() < FOOD_CRITICAL_THRESHOLD) {
      msg += "⚠ CRITICAL - Refill needed!";
    } else if (Hardware::getFoodLevel() < FOOD_WARNING_THRESHOLD) {
      msg += "⚠ LOW - Consider refilling";
    } else {
      msg += "✅ Level OK";
    }
    sendMessage(chat_id, msg, "Markdown");
    
  } else if (text == "/info minum" || text == "💧 water info") {
    Hardware::readAllSensors();
    String msg = "💧 Water Status\n";
    msg += "Level: " + String(Hardware::getWaterLevel()) + "%\n";
    if (Hardware::getWaterLevel() < WATER_CRITICAL_THRESHOLD) {
      msg += "⚠ CRITICAL - Refill needed!";
    } else if (Hardware::getWaterLevel() < WATER_WARNING_THRESHOLD) {
      msg += "⚠ LOW - Consider refilling";
    } else {
      msg += "✅ Level OK";
    }
    sendMessage(chat_id, msg, "Markdown");
    
  } else if (text == "/setwaktu" || text == "⏰ schedule") {
    sendTimeMenuKeyboard(chat_id);
    
  } else if (text == "/tambah jadwal" || text == "➕ add schedule") {
    waitingForTimeInput = true;
    pendingChatId = chat_id;
    sendMessage(chat_id, "⏰ Send time in HH:MM format (24 hour)\nExample: 08:30 or 15:45", "Markdown");
    
  } else if (text == "/lihat jadwal" || text == "📋 view schedule") {
    sendMessage(chat_id, TimeManager::getScheduleList());
    
  } else if (text == "/hapus jadwal" || text == "🗑 clear schedule") {
    TimeManager::clearAllSchedules();
    sendMessage(chat_id, "🗑 All schedules cleared successfully!");
    
  } else if (text == "/system" || text == "⚙ system") {
    sendSystemMenuKeyboard(chat_id);
    
  } else if (text == "/logs" || text == "📝 logs") {
    sendMessage(chat_id, DataLogger::getDataSummary());
    
  } else if (text == "/sysinfo" || text == "ℹ system info") {
    sendMessage(chat_id, formatSystemInfo());
    
  } else if (text == "/reboot" || text == "🔄 reboot") {
    sendMessage(chat_id, "🔄 Rebooting system...");
    delay(1000);
    ESP.restart();
    
  } else if (text == "/kembali" || text == "🔙 back") {
    waitingForTimeInput = false;
    pendingChatId = "";
    sendMenuKeyboard(chat_id);
    
  } else {
    sendMessage(chat_id, "❓ Unknown command. Use /menu to see available options.");
  }
}

void TelegramHandler::processTimeInput(String chat_id, String text) {
  waitingForTimeInput = false;
  pendingChatId = "";
  
  text.trim();
  if (text.length() == 4 && text.indexOf(':') == 1) {
    text = "0" + text; // Convert H:MM to HH:MM
  }
  
  if (TimeManager::isValidTimeFormat(text)) {
    TimeManager::addSchedule(text, true);
    sendMessage(chat_id, "✅ Schedule " + text + " added successfully!", "Markdown");
    sendDebugInfo("Schedule added: " + text);
  } else {
    sendMessage(chat_id, "❌ Invalid time format. Use HH:MM (24 hour format)\nExample: 08:30 or 15:45");
  }
}

void TelegramHandler::sendMenuKeyboard(String chat_id) {
  // Format keyboard seperti contoh yang berhasil - TANPA wrapper "keyboard"
  String keyboardJson = "[[{\"text\":\"📊 Status\"},{\"text\":\"🍽 Feed Now\"}],"
                       "[{\"text\":\"🍽 Food Info\"},{\"text\":\"💧 Water Info\"}],"
                       "[{\"text\":\"⏰ Schedule\"},{\"text\":\"⚙ System\"}]]";
  
  bot.sendMessageWithReplyKeyboard(chat_id, 
    "🐹 *HAMSTER FEEDER CONTROL*\nChoose an option below:", 
    "Markdown", 
    keyboardJson, 
    true);
}

void TelegramHandler::sendTimeMenuKeyboard(String chat_id) {
  // Format keyboard seperti contoh yang berhasil
  String keyboardJson = "[[{\"text\":\"➕ Add Schedule\"}],"
                       "[{\"text\":\"📋 View Schedule\"}],"
                       "[{\"text\":\"🗑 Clear Schedule\"}],"
                       "[{\"text\":\"🔙 Back\"}]]";
  
  bot.sendMessageWithReplyKeyboard(chat_id, 
    "⏰ *SCHEDULE MANAGEMENT*\nChoose an option:", 
    "Markdown", 
    keyboardJson, 
    true);
}

void TelegramHandler::sendSystemMenuKeyboard(String chat_id) {
  // Format keyboard seperti contoh yang berhasil
  String keyboardJson = "[[{\"text\":\"📝 Logs\"},{\"text\":\"ℹ System Info\"}],"
                       "[{\"text\":\"🔄 Reboot\"}],"
                       "[{\"text\":\"🔙 Back\"}]]";
  
  bot.sendMessageWithReplyKeyboard(chat_id, 
    "⚙ *SYSTEM MANAGEMENT*\nAdvanced system options:", 
    "Markdown", 
    keyboardJson, 
    true);
}

String TelegramHandler::formatStatusMessage() {
  Hardware::readAllSensors(); // Ensure fresh data
  
  String status = "📊 SYSTEM STATUS\n\n";
  
  // Time info
  status += "🕐 Time: " + TimeManager::getCurrentTimeString() + "\n";
  status += "⚡ WiFi: " + String(WiFi.status() == WL_CONNECTED ? "Connected" : "❌ Disconnected") + "\n\n";
  
  // Hardware status
  status += "🔋 Battery: " + String(Hardware::getBatteryVolt(), 1) + "V (" + String(Hardware::getBatteryPercent(), 0) + "%)\n";
  status += "🍽 Food: " + String(Hardware::getFoodLevel()) + "%\n";
  status += "💧 Water: " + String(Hardware::getWaterLevel()) + "%\n\n";
  
  // Feed info
  status += "📈 Total feeds: " + String(DataLogger::getTotalFeeds()) + "\n";
  
  // Alerts
  if (Hardware::isCriticalBattery()) {
    status += "\n⚠ CRITICAL: Battery very low!";
  } else if (Hardware::isLowBattery()) {
    status += "\n⚠ WARNING: Battery low";
  }
  
  if (Hardware::getFoodLevel() < FOOD_CRITICAL_THRESHOLD) {
    status += "\n⚠ CRITICAL: Food very low!";
  }
  
  if (Hardware::getWaterLevel() < WATER_CRITICAL_THRESHOLD) {
    status += "\n⚠ CRITICAL: Water very low!";
  }
  
  return status;
}

String TelegramHandler::formatSystemInfo() {
  String info = "ℹ SYSTEM INFORMATION\n\n";
  
  // info += "💾 Free Heap: " + String(ESP.getFreeHeap()) + " bytes\n";
  // info += "⚡ Chip ID: " + String(ESP.getChipId()) + "\n";
  info += "🔄 Uptime: " + String(millis() / 1000 / 60) + " minutes\n";
  // info += "📶 RSSI: " + String(WiFi.RSSI()) + " dBm\n";
  info += "🌐 IP: " + WiFi.localIP().toString() + "\n";
  // info += "🔧 SDK: " + String(ESP.getSdkVersion()) + "\n";
  
  return info;
}

// Notification methods
void TelegramHandler::sendStartupNotification() {
  String message = "🟢 System Started\n";
  message += "Hamster Feeder is now online!\n";
  message += "Time: " + TimeManager::getCurrentTimeString();
  sendMessage(CHAT_ID, message, "Markdown");
}

void TelegramHandler::sendAutoFeedNotification(String time) {
  String message = "🍽 Auto Feed Executed\n";
  message += "Time: " + time + "\n";
  message += "Food level: " + String(Hardware::getFoodLevel()) + "%";
  sendMessage(CHAT_ID, message, "Markdown");
  sendDebugInfo("Auto feed at " + time);
}

void TelegramHandler::sendFeedingResult(bool success, String reason) {
  String message;
  if (success) {
    message = "✅ Feeding Successful\n";
    message += "Hamster has been fed!\n";
    message += "Food level: " + String(Hardware::getFoodLevel()) + "%";
  } else {
    message = "❌ Feeding Failed\n";
    message += "Reason: " + reason + "\n";
    message += "Food level: " + String(Hardware::getFoodLevel()) + "%";
  }
  sendMessage(CHAT_ID, message, "Markdown");
}

void TelegramHandler::sendFoodAlert(int level, bool critical) {
  String message = critical ? "🚨 CRITICAL FOOD ALERT\n" : "⚠ Food Warning\n";
  message += "Food level: " + String(level) + "%\n";
  message += critical ? "Immediate refill required!" : "Consider refilling soon";
  sendMessage(CHAT_ID, message, "Markdown");
}

void TelegramHandler::sendWaterAlert(int level, bool critical) {
  String message = critical ? "🚨 CRITICAL WATER ALERT\n" : "⚠ Water Warning\n";
  message += "Water level: " + String(level) + "%\n";
  message += critical ? "Immediate refill required!" : "Consider refilling soon";
  sendMessage(CHAT_ID, message, "Markdown");
}

void TelegramHandler::sendBatteryAlert(float percent, bool critical) {
  String message = critical ? "🚨 CRITICAL BATTERY ALERT\n" : "⚠ Battery Warning\n";
  message += "Battery: " + String(percent, 0) + "%\n";
  message += critical ? "System may shut down soon!" : "Consider charging";
  sendMessage(CHAT_ID, message, "Markdown");
}

void TelegramHandler::sendSystemAlert(String message) {
  sendMessage(CHAT_ID, "⚠ System Alert\n" + message, "Markdown");
}

void TelegramHandler::sendLogs(String logs) {
  sendMessage(CHAT_ID, "📝 System Logs\n" + logs, "Markdown");
}

void TelegramHandler::sendDebugInfo(String info) {
  #ifdef DEBUG_MODE
  sendMessage(CHAT_ID, "🔧 DEBUG: " + info, "Markdown");
  #endif
  Serial.println("🔧 " + info);
}

// Utility methods
void TelegramHandler::sendMessage(String chat_id, String message, String parseMode) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("🔄 WiFi disconnected, attempting reconnect...");
    WiFi.reconnect();
    
    // Wait with timeout
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
      delay(500); // 0.5 detik per attempt
      attempts++;
      Serial.print(".");
    }
    Serial.println();
  }

  if (WiFi.status() == WL_CONNECTED) {
    bot.sendMessage(chat_id, message, parseMode);
    Serial.println("✅ Message sent successfully");
  } else {
    Serial.println("❌ WiFi not connected - message not sent");
  }
}

void TelegramHandler::sendMessageWithKeyboard(String chat_id, String message, String keyboard, String parseMode) {
  if (WiFi.status() == WL_CONNECTED) {
    bot.sendMessageWithReplyKeyboard(chat_id, message, parseMode, keyboard, true);
  } else {
    Serial.println("❌ WiFi not connected - message not sent");
  }
}

bool TelegramHandler::isAuthorizedUser(String chat_id) {
  return chat_id == CHAT_ID; // Simple authorization - can be expanded
}