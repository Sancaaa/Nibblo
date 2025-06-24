#ifndef TELEGRAM_HANDLER_H
#define TELEGRAM_HANDLER_H

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "config.h"
#include "credential.h"

// Forward declarations untuk dependencies
class Hardware;
class PowerManager;
class TimeManager;
class DataLogger;

class TelegramHandler {
private:
  static WiFiClientSecure client;
  static UniversalTelegramBot bot;
  static unsigned long lastBotRan;
  static const unsigned long BOT_INTERVAL = 1000; // Check messages every 1 second

public:
  static void init();
  static void checkMessages();1
  static void handleNewMessages(int numNewMessages);
  static void sendStartupNotification();
  static void sendFoodAlert(int level, bool critical);
  static void sendWaterAlert(int level, bool critical);
  static void sendBatteryAlert(float level, bool critical);
  static void sendAutoFeedNotification(String time);
  static String getMenuText();
  static String getStatusText();
  static void sendMessage(String message);
  static bool isTimeToCheck();
};

// Static member definitions
WiFiClientSecure TelegramHandler::client;
UniversalTelegramBot TelegramHandler::bot(BOT_TOKEN, client);
unsigned long TelegramHandler::lastBotRan = 0;

void TelegramHandler::init() {
  Serial.println("🔧 Initializing Telegram Handler...");
  
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  client.setInsecure();
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(1000);
    attempts++;
    Serial.println("Connecting to WiFi... Attempt " + String(attempts));
    Hardware::displayMessage("WiFi: " + String(attempts) + "/30");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
    Hardware::displayMessage("WiFi Connected!");
    Serial.println("✅ Telegram Handler initialized");
    
    // Send startup notification after a short delay
    delay(2000);
    sendStartupNotification();
  } else {
    Serial.println("❌ WiFi connection failed!");
    Hardware::displayMessage("WiFi Failed!");
  }
}

bool TelegramHandler::isTimeToCheck() {
  return (millis() - lastBotRan > BOT_INTERVAL);
}

void TelegramHandler::checkMessages() {
  if (!isTimeToCheck()) return;
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, skipping message check");
    return;
  }
  
  lastBotRan = millis();
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  
  while (numNewMessages) {
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
  
  PowerManager::updateActivity();
}

void TelegramHandler::handleNewMessages(int numNewMessages) {
  Serial.println("Handling " + String(numNewMessages) + " new messages");
  
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String from_name = bot.messages[i].from_name;
    
    // Security check - only authorized chat ID
    if (chat_id != CHAT_ID) {
      Serial.println("Unauthorized access attempt from: " + from_name + " (" + chat_id + ")");
      bot.sendMessage(chat_id, "❌ Unauthorized user. Access denied.", "");
      continue;
    }
    
    String text = bot.messages[i].text;
    Serial.println("Command received: " + text);
    
    // Command handling
    if (text == "/start") {
      String welcome = "🐹 Hamster Feeder Bot Online!\n\n";
      welcome += "Halo " + from_name + "! 👋\n";
      welcome += "Bot siap melayani hamster kesayangan Anda.\n\n";
      welcome += "Gunakan /menu untuk melihat semua perintah yang tersedia.";
      bot.sendMessage(chat_id, welcome, "");
    }
    else if (text == "/menu") {
      bot.sendMessage(chat_id, getMenuText(), "");
    }
    else if (text == "/status") {
      bot.sendMessage(chat_id, getStatusText(), "");
    }
    else if (text == "/info_makan") {
      int food = Hardware::getFoodLevel();
      String msg = "🍽️ INFORMASI MAKANAN\n\n";
      msg += "Level: " + String(food) + "%\n";
      if (food > 30) {
        msg += "Status: Cukup ✅\n";
        msg += "Kondisi: Normal";
      } else if (food > 15) {
        msg += "Status: Rendah ⚠️\n";
        msg += "Kondisi: Perlu diisi ulang segera";
      } else {
        msg += "Status: Kritis ❌\n";
        msg += "Kondisi: SEGERA ISI MAKANAN!";
      }
      bot.sendMessage(chat_id, msg, "");
    }
    else if (text == "/info_minum") {
      int water = Hardware::getWaterLevel();
      String msg = "💧 INFORMASI MINUMAN\n\n";
      msg += "Level: " + String(water) + "%\n";
      if (water > 25) {
        msg += "Status: Cukup ✅\n";
        msg += "Kondisi: Normal";
      } else if (water > 10) {
        msg += "Status: Rendah ⚠️\n";
        msg += "Kondisi: Perlu diisi ulang segera";
      } else {
        msg += "Status: Kritis ❌\n";
        msg += "Kondisi: SEGERA ISI AIR!";
      }
      bot.sendMessage(chat_id, msg, "");
    }
    else if (text == "/makan") {
      bot.sendMessage(chat_id, "🍽️ Sedang memberikan makanan...", "");
      bool success = Hardware::feedHamster();
      if (success) {
        DataLogger::logFeeding("MANUAL", TimeManager::getCurrentTime());
        bot.sendMessage(chat_id, "✅ Makanan telah diberikan!\n⏰ " + TimeManager::getCurrentTimeString(), "");
      } else {
        bot.sendMessage(chat_id, "❌ Gagal memberikan makanan. Periksa hardware!", "");
      }
    }
    else if (text == "/jadwal") {
      bot.sendMessage(chat_id, TimeManager::getScheduleList(), "");
    }
    else if (text.startsWith("/tambah_jadwal ")) {
      String newTime = text.substring(15);
      newTime.trim(); // Remove whitespace
      
      if (TimeManager::isValidTimeFormat(newTime)) {
        bool added = TimeManager::addSchedule(newTime, true);
        if (added) {
          bot.sendMessage(chat_id, "✅ Jadwal " + newTime + " berhasil ditambahkan!", "");
        } else {
          bot.sendMessage(chat_id, "❌ Gagal menambah jadwal. Mungkin sudah ada atau memori penuh.", "");
        }
      } else {
        bot.sendMessage(chat_id, "❌ Format waktu salah!\n\nGunakan format: HH:MM\nContoh: /tambah_jadwal 08:30", "");
      }
    }
    else if (text.startsWith("/hapus_jadwal ")) {
      String timeToDelete = text.substring(14);
      timeToDelete.trim();
      
      bool deleted = TimeManager::removeSchedule(timeToDelete);
      if (deleted) {
        bot.sendMessage(chat_id, "✅ Jadwal " + timeToDelete + " berhasil dihapus!", "");
      } else {
        bot.sendMessage(chat_id, "❌ Jadwal tidak ditemukan atau gagal dihapus.", "");
      }
    }
    else if (text == "/data") {
      bot.sendMessage(chat_id, DataLogger::getDataSummary(), "");
    }
    else if (text == "/reboot") {
      bot.sendMessage(chat_id, "🔄 Sistem akan restart dalam 5 detik...", "");
      delay(5000);
      ESP.restart();
    }
    else if (text == "/help") {
      String help = "🆘 BANTUAN PENGGUNAAN\n\n";
      help += "📋 Perintah dasar:\n";
      help += "• /start - Mulai bot\n";
      help += "• /menu - Tampilkan menu\n";
      help += "• /status - Status lengkap sistem\n\n";
      help += "🍽️ Makanan & Minuman:\n";
      help += "• /info_makan - Cek level makanan\n";
      help += "• /info_minum - Cek level air\n";
      help += "• /makan - Beri makan manual\n\n";
      help += "⏰ Jadwal:\n";
      help += "• /jadwal - Lihat semua jadwal\n";
      help += "• /tambah_jadwal HH:MM - Tambah jadwal baru\n";
      help += "• /hapus_jadwal HH:MM - Hapus jadwal\n\n";
      help += "📊 Data:\n";
      help += "• /data - Ringkasan data feeding\n";
      help += "• /reboot - Restart sistem";
      bot.sendMessage(chat_id, help, "");
    }
    else {
      String error = "❓ Perintah tidak dikenali: " + text + "\n\n";
      error += "Gunakan /menu untuk melihat perintah yang tersedia\n";
      error += "atau /help untuk bantuan lengkap.";
      bot.sendMessage(chat_id, error, "");
    }
  }
}

String TelegramHandler::getMenuText() {
  String menu = "🐹 HAMSTER FEEDER MENU\n";
  menu += "═══════════════════════\n\n";
  menu += "📊 /status - Status lengkap sistem\n";
  menu += "🍽️ /info_makan - Informasi makanan\n";
  menu += "💧 /info_minum - Informasi minuman\n";
  menu += "🥄 /makan - Beri makan sekarang\n";
  menu += "⏰ /jadwal - Lihat jadwal auto feed\n";
  menu += "➕ /tambah_jadwal HH:MM - Tambah jadwal\n";
  menu += "➖ /hapus_jadwal HH:MM - Hapus jadwal\n";
  menu += "📈 /data - Lihat ringkasan data\n";
  menu += "🔄 /reboot - Restart sistem\n";
  menu += "🆘 /help - Bantuan lengkap\n";
  menu += "📋 /menu - Menu ini\n\n";
  menu += "💡 Tip: Gunakan /help untuk panduan detail";
  return menu;
}

String TelegramHandler::getStatusText() {
  String status = "📊 STATUS SISTEM HAMSTER FEEDER\n";
  status += "════════════════════════════════\n\n";
  
  // Battery status
  float battVolt = Hardware::getBatteryVolt();
  int battPercent = Hardware::getBatteryPercent();
  status += "🔋 Baterai: " + String(battVolt, 1) + "V (" + String(battPercent) + "%)\n";
  if (battPercent > 50) status += "    Status: Baik ✅\n";
  else if (battPercent > 20) status += "    Status: Sedang ⚠️\n";
  else status += "    Status: Rendah ❌\n";
  
  // WiFi status
  status += "📶 WiFi: ";
  if (WiFi.status() == WL_CONNECTED) {
    status += "Terhubung ✅\n";
    status += "    IP: " + WiFi.localIP().toString() + "\n";
    status += "    RSSI: " + String(WiFi.RSSI()) + " dBm\n";
  } else {
    status += "Terputus ❌\n";
  }
  
  // Food and water levels
  int food = Hardware::getFoodLevel();
  int water = Hardware::getWaterLevel();
  
  status += "🍽️ Makanan: " + String(food) + "%\n";
  status += "💧 Minuman: " + String(water) + "%\n";
  
  // Time and feeding info
  status += "⏰ Waktu: " + TimeManager::getCurrentTimeString() + "\n";
  status += "🔢 Total feeding: " + String(DataLogger::getTotalFeeds()) + " kali\n";
  status += "📅 Feeding hari ini: " + String(DataLogger::getTodayFeeds()) + " kali\n";
  
  // System uptime
  unsigned long uptime = millis() / 1000;
  int hours = uptime / 3600;
  int minutes = (uptime % 3600) / 60;
  status += "⏱️ Uptime: " + String(hours) + "h " + String(minutes) + "m\n";
  
  // Next scheduled feeding
  String nextFeed = TimeManager::getNextScheduledTime();
  if (nextFeed != "") {
    status += "⏭️ Feeding berikutnya: " + nextFeed;
  }
  
  return status;
}

void TelegramHandler::sendStartupNotification() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Cannot send startup notification - WiFi not connected");
    return;
  }
  
  String msg = "🚀 HAMSTER FEEDER SYSTEM STARTED\n";
  msg += "═══════════════════════════════════\n\n";
  msg += "📱 Bot online dan siap digunakan\n";
  msg += "⏰ Waktu sistem: " + TimeManager::getCurrentTimeString() + "\n";
  msg += "🔋 Baterai: " + String(Hardware::getBatteryPercent()) + "%\n";
  msg += "📶 WiFi: " + WiFi.localIP().toString() + "\n\n";
  msg += "Sistem siap melayani hamster kesayangan Anda! 🐹\n";
  msg += "Gunakan /menu untuk melihat perintah yang tersedia.";
  
  bool sent = bot.sendMessage(CHAT_ID, msg, "");
  if (sent) {
    Serial.println("✅ Startup notification sent");
  } else {
    Serial.println("❌ Failed to send startup notification");
  }
}

void TelegramHandler::sendFoodAlert(int level, bool critical) {
  String msg = critical ? "🚨 PERINGATAN KRITIS - MAKANAN HABIS!\n" : "⚠️ Peringatan - Makanan Rendah\n";
  msg += "═══════════════════════════════\n\n";
  msg += "🍽️ Level makanan: " + String(level) + "%\n";
  
  if (critical) {
    msg += "❌ Status: KRITIS - Makanan hampir habis!\n";
    msg += "🔔 TINDAKAN SEGERA DIPERLUKAN:\n";
    msg += "• Isi ulang wadah makanan\n";
    msg += "• Periksa kondisi hamster\n";
    msg += "• Pastikan sistem feeding berfungsi";
  } else {
    msg += "⚠️ Status: Rendah - Perlu diisi ulang\n";
    msg += "💡 Saran:\n";
    msg += "• Siapkan makanan cadangan\n";
    msg += "• Isi ulang dalam waktu dekat";
  }
  
  msg += "\n\n⏰ " + TimeManager::getCurrentTimeString();
  sendMessage(msg);
}

void TelegramHandler::sendWaterAlert(int level, bool critical) {
  String msg = critical ? "🚨 PERINGATAN KRITIS - AIR HABIS!\n" : "⚠️ Peringatan - Air Rendah\n";
  msg += "═══════════════════════════════\n\n";
  msg += "💧 Level air: " + String(level) + "%\n";
  
  if (critical) {
    msg += "❌ Status: KRITIS - Air hampir habis!\n";
    msg += "🔔 TINDAKAN SEGERA DIPERLUKAN:\n";
    msg += "• Isi ulang wadah air\n";
    msg += "• Periksa kondisi hamster\n";
    msg += "• Pastikan sistem minum berfungsi";
  } else {
    msg += "⚠️ Status: Rendah - Perlu diisi ulang\n";
    msg += "💡 Saran:\n";
    msg += "• Siapkan air bersih\n";
    msg += "• Isi ulang dalam waktu dekat";
  }
  
  msg += "\n\n⏰ " + TimeManager::getCurrentTimeString();
  sendMessage(msg);
}

void TelegramHandler::sendBatteryAlert(float level, bool critical) {
  String msg = critical ? "🚨 PERINGATAN KRITIS - BATERAI LEMAH!\n" : "⚠️ Peringatan - Baterai Rendah\n";
  msg += "═══════════════════════════════\n\n";
  msg += "🔋 Level baterai: " + String(level, 1) + "V (" + String(Hardware::getBatteryPercent()) + "%)\n";
  
  if (critical) {
    msg += "❌ Status: KRITIS - Sistem akan mati!\n";
    msg += "🔔 TINDAKAN SEGERA:\n";
    msg += "• Sambungkan charger segera\n";
    msg += "• Sistem akan masuk mode hemat daya\n";
    msg += "• Feeding otomatis mungkin terganggu";
  } else {
    msg += "⚠️ Status: Rendah - Perlu dicharge\n";
    msg += "💡 Saran:\n";
    msg += "• Siapkan charger\n";
    msg += "• Charge dalam waktu dekat";
  }
  
  msg += "\n\n⏰ " + TimeManager::getCurrentTimeString();
  sendMessage(msg);
}

void TelegramHandler::sendAutoFeedNotification(String time) {
  String msg = "🍽️ FEEDING OTOMATIS\n";
  msg += "═══════════════════════\n\n";
  msg += "✅ Hamster telah diberi makan\n";
  msg += "⏰ Waktu: " + time + "\n";
  msg += "🔄 Jenis: Terjadwal otomatis\n";
  msg += "📊 Status: Berhasil\n\n";
  msg += "🐹 Selamat makan, hamster kesayangan!";
  
  sendMessage(msg);
  Serial.println("Auto feed notification sent for: " + time);
}

void TelegramHandler::sendMessage(String message) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Cannot send message - WiFi not connected");
    return;
  }
  
  bool sent = bot.sendMessage(CHAT_ID, message, "");
  if (!sent) {
    Serial.println("Failed to send message: " + message.substring(0, 50) + "...");
  }
}

#endif // TELEGRAM_HANDLER_H