#ifndef TELEGRAM_HANDLER_H
#define TELEGRAM_HANDLER_H

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "config.h"

class TelegramHandler {
private:
  static WiFiClientSecure client;
  static UniversalTelegramBot bot;
  static unsigned long lastBotRan;

public:
  static void init();
  static void checkMessages();
  static void handleNewMessages(int numNewMessages);
  static void sendStartupNotification();
  static void sendFoodAlert(int level, bool critical);
  static void sendWaterAlert(int level, bool critical);
  static void sendBatteryAlert(float level, bool critical);
  static void sendAutoFeedNotification(String time);
  static String getMenuText();
  static String getStatusText();
};

WiFiClientSecure TelegramHandler::client;
UniversalTelegramBot TelegramHandler::bot(BOT_TOKEN, client);
unsigned long TelegramHandler::lastBotRan = 0;

void TelegramHandler::init() {
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  client.setInsecure();
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    Hardware::displayMessage("Connecting WiFi...");
  }
  
  Serial.println("WiFi connected: " + WiFi.localIP().toString());
  Hardware::displayMessage("WiFi Connected!");
  
  Serial.println("✅ Telegram Handler initialized");
}

void TelegramHandler::checkMessages() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  
  while (numNewMessages) {
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
  
  PowerManager::updateActivity();
}

void TelegramHandler::handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    String text = bot.messages[i].text;
    
    if (text == "/start") {
      bot.sendMessage(chat_id, "🐹 Hamster Feeder Bot Online!\nGunakan /menu untuk melihat perintah.", "");
    }
    else if (text == "/menu") {
      bot.sendMessage(chat_id, getMenuText(), "");
    }
    else if (text == "/status") {
      bot.sendMessage(chat_id, getStatusText(), "");
    }
    else if (text == "/info_makan") {
      int food = Hardware::getFoodLevel();
      String msg = "🍽️ Level makanan: " + String(food) + "%\n";
      msg += food > 30 ? "Status: OK ✅" : food > 15 ? "Status: Rendah ⚠️" : "Status: Kritis ❌";
      bot.sendMessage(chat_id, msg, "");
    }
    else if (text == "/info_minum") {
      int water = Hardware::getWaterLevel();
      String msg = "💧 Level air: " + String(water) + "%\n";
      msg += water > 25 ? "Status: OK ✅" : water > 10 ? "Status: Rendah ⚠️" : "Status: Kritis ❌";
      bot.sendMessage(chat_id, msg, "");
    }
    else if (text == "/makan") {
      bot.sendMessage(chat_id, "🍽️ Memberikan makanan...", "");
      Hardware::feedHamster();
      DataLogger::logFeeding("MANUAL", TimeManager::getCurrentTime());
      bot.sendMessage(chat_id, "✅ Makanan telah diberikan!", "");
    }
    else if (text == "/jadwal") {
      bot.sendMessage(chat_id, TimeManager::getScheduleList(), "");
    }
    else if (text.startsWith("/tambah_jadwal ")) {
      String newTime = text.substring(15);
      if (TimeManager::isValidTimeFormat(newTime)) {
        TimeManager::addSchedule(newTime, true);
        bot.sendMessage(chat_id, "✅ Jadwal " + newTime + " ditambahkan!", "");
      } else {
        bot.sendMessage(chat_id, "❌ Format salah! Gunakan HH:MM", "");
      }
    }
    else if (text == "/data") {
      bot.sendMessage(chat_id, DataLogger::getDataSummary(), "");
    }
    else {
      bot.sendMessage(chat_id, "Perintah tidak dikenali. Gunakan /menu", "");
    }
  }
}

String TelegramHandler::getMenuText() {
  String menu = "🐹 HAMSTER FEEDER MENU\n\n";
  menu += "/status - Status lengkap\n";
  menu += "/info_makan - Info makanan\n";
  menu += "/info_minum - Info minuman\n";
  menu += "/makan - Beri makan sekarang\n";
  menu += "/jadwal - Lihat jadwal auto feed\n";
  menu += "/tambah_jadwal HH:MM - Tambah jadwal\n";
  menu += "/data - Lihat data summary\n";
  menu += "/menu - Menu ini";
  return menu;
}

String TelegramHandler::getStatusText() {
  String status = "📊 STATUS DEVICE\n\n";
  status += "🔋 Baterai: " + String(Hardware::getBatteryVolt(), 1) + "V (";
  status += String(Hardware::getBatteryPercent(), 0) + "%)\n";
  status += "📶 WiFi: " + (WiFi.status() == WL_CONNECTED ? "OK ✅" : "Error ❌") + "\n";
  status += "🍽️ Makanan: " + String(Hardware::getFoodLevel()) + "%\n";
  status += "💧 Minuman: " + String(Hardware::getWaterLevel()) + "%\n";
  status += "⏰ Waktu: " + TimeManager::getCurrentTimeString() + "\n";
  status += "📈 Total feeds: " + String(DataLogger::getTotalFeeds());
  return status;
}

void TelegramHandler::sendStartupNotification() {
  String msg = "🚀 Hamster Feeder System Started!\n";
  msg += "📱 Bot online dan siap digunakan\n";
  msg += "⏰ " + TimeManager::getCurrentTimeString