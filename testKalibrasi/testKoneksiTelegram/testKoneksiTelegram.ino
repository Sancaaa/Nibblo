// TelegramTest.ino
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "credential.h"  // berisi WIFI_SSID, WIFI_PASSWORD, BOT_TOKEN, CHAT_ID

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

const unsigned long BOT_INTERVAL = 10000;  // cek sekali setiap 10 detik
unsigned long lastCheck = 0;

void setup() {
  Serial.begin(115200);
  delay(10);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
  client.setInsecure();
  bot.sendMessage(CHAT_ID, "✅ Telegram bot test: device online!", "");
}

void loop() {
  if (millis() - lastCheck > BOT_INTERVAL) {
    lastCheck = millis();
    int msgs = bot.getUpdates(bot.last_message_received + 1);
    if (msgs > 0) {
      Serial.println("Received " + String(msgs) + " messages!");
      for (int i = 0; i < msgs; i++) {
        String from = bot.messages[i].from_name;
        String text = bot.messages[i].text;
        String chat = String(bot.messages[i].chat_id);
        Serial.println("From: " + from + " (" + chat + ") >> " + text);
        if (chat == CHAT_ID) {
          bot.sendMessage(CHAT_ID, "Got: " + text, "");
        } else {
          bot.sendMessage(chat, "❌ Unauthorized", "");
        }
      }
    } else {
      Serial.println("No new messages");
    }
  }
}
