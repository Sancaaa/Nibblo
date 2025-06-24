#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// WiFi & Bot Credentials
const char* ssid = "Aquabubuk";
const char* password = "hidupaqua";
const char* botToken = "7607051762:AAHjqbONCvdKkwCaIuJPk9IWoCJllwYOpME";
const int botRequestDelay = 1000;
char* chat_id = "1844227079";

WiFiClientSecure secured_client;
UniversalTelegramBot bot(botToken, secured_client);

// Jadwal
String jadwal[5];
int jumlahJadwal = 0;
bool menungguInputWaktu = false;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  secured_client.setInsecure();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");
  sendMenuKeyboard(chat_id);
}

unsigned long lastCheckTime = 0;
void loop() {
  if (millis() - lastCheckTime > botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastCheckTime = millis();
  }
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    text.trim();
    text.toLowerCase();

    Serial.print("📩 Diterima: [");
    Serial.print(text);
    Serial.println("]");

    if (menungguInputWaktu) {
      if (text.length() == 5 && text.charAt(2) == ':' && jumlahJadwal < 5) {
        jadwal[jumlahJadwal++] = text;
        bot.sendMessage(chat_id, "✅ Jadwal " + text + " disimpan!", "");
      } else {
        bot.sendMessage(chat_id, "❌ Format salah atau jadwal penuh. Gunakan format HH:MM", "");
      }
      menungguInputWaktu = false;
    } else if (text == "/start" || text == "/menu") {
      sendMenuKeyboard(chat_id);
    } else if (text == "/status") {
      bot.sendMessage(chat_id, "📊 Sistem aktif dan terhubung.", "");
    } else if (text == "/makan") {
      bot.sendMessage(chat_id, "🍽️ Memberi makan hamster...", "");
    } else if (text == "/info makan") {
      bot.sendMessage(chat_id, "📦 Pakan tersisa: 75%", "");
    } else if (text == "/info minum") {
      bot.sendMessage(chat_id, "💧 Air minum: 60%", "");
    } else if (text == "/setwaktu") {
      sendTimeMenuKeyboard(chat_id);
    } else if (text == "/tambah jadwal") {
      menungguInputWaktu = true;
      bot.sendMessage(chat_id, "Kirim waktu dalam format *HH:MM* (24 jam)", "Markdown");
    } else if (text == "/lihat jadwal") {
      String msg = "🕒 Jadwal saat ini:\n";
      for (int j = 0; j < jumlahJadwal; j++) {
        msg += "- " + jadwal[j] + "\n";
      }
      if (jumlahJadwal == 0) msg += "(belum ada)";
      bot.sendMessage(chat_id, msg, "");
    } else if (text == "/hapus jadwal") {
      jumlahJadwal = 0;
      bot.sendMessage(chat_id, "🗑️ Semua jadwal berhasil dihapus.", "");
    } else if (text == "/kembali") {
      sendMenuKeyboard(chat_id);
    } else {
      bot.sendMessage(chat_id, "❓ Perintah tidak dikenal. Gunakan /menu untuk melihat opsi.", "");
    }
  }
}

void sendMenuKeyboard(String chat_id) {
  // Gunakan method built-in UniversalTelegramBot untuk keyboard
  String keyboardJson = "[[{\"text\":\"/status\"},{\"text\":\"/makan\"}],"
                       "[{\"text\":\"/info makan\"},{\"text\":\"/info minum\"}],"
                       "[{\"text\":\"/hihi\"}]]";
  
  bot.sendMessageWithReplyKeyboard(chat_id, 
    "🐹 *HAMSTER FEEDER MENU*\nGunakan tombol di bawah ini:", 
    "Markdown", 
    keyboardJson, 
    true);
}

void sendTimeMenuKeyboard(String chat_id) {
  // Method yang lebih sederhana dan stabil
  String keyboardJson = "[[{\"text\":\"/tambah jadwal\"}],"
                       "[{\"text\":\"/lihat jadwal\"}],"
                       "[{\"text\":\"/hapus jadwal\"}],"
                       "[{\"text\":\"/kembali\"}]]";
  
  bot.sendMessageWithReplyKeyboard(chat_id, 
    "⏰ *Menu Pengaturan Waktu*\nSilakan pilih:", 
    "Markdown", 
    keyboardJson, 
    true);
}

// Backup method jika ingin tetap menggunakan custom payload
void sendTimeMenuKeyboardCustom(String chat_id) {
  StaticJsonDocument<1024> doc; // Gunakan StaticJsonDocument untuk stabilitas
  doc["chat_id"] = chat_id;
  doc["text"] = "⏰ *Menu Pengaturan Waktu*\nSilakan pilih:";
  doc["parse_mode"] = "Markdown";

  JsonObject replyMarkup = doc.createNestedObject("reply_markup");
  JsonArray keyboard = replyMarkup.createNestedArray("keyboard");

  JsonArray row1 = keyboard.createNestedArray();
  row1.add("/tambah jadwal");

  JsonArray row2 = keyboard.createNestedArray();
  row2.add("/lihat jadwal");

  JsonArray row3 = keyboard.createNestedArray();
  row3.add("/hapus jadwal");

  JsonArray row4 = keyboard.createNestedArray();
  row4.add("/kembali");

  replyMarkup["resize_keyboard"] = true;
  replyMarkup["one_time_keyboard"] = false;

  String payload;
  serializeJson(doc, payload);
  
  // Debug: Print payload sebelum dikirim
  Serial.println("📤 Payload yang akan dikirim:");
  Serial.println(payload);
  
  kirimKeTelegramImproved(payload);
}

// Versi yang lebih robust untuk kirim ke Telegram
void kirimKeTelegramImproved(String payload) {
  WiFiClientSecure client;
  client.setInsecure();
  
  // Timeout connection
  client.setTimeout(10000);
  
  Serial.println("🔄 Mencoba koneksi ke Telegram...");
  
  if (!client.connect("api.telegram.org", 443)) {
    Serial.println("🔴 Gagal konek ke Telegram");
    return;
  }
  
  Serial.println("🟢 Terhubung ke Telegram");

  String url = "/bot" + String(botToken) + "/sendMessage";
  
  // HTTP Request
  client.println("POST " + url + " HTTP/1.1");
  client.println("Host: api.telegram.org");
  client.println("User-Agent: ESP8266");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.print("Content-Length: ");
  client.println(payload.length());
  client.println();
  client.print(payload);

  // Tunggu response
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 10000) {
      Serial.println("🔴 Timeout menunggu response");
      client.stop();
      return;
    }
  }

  // Baca response
  String response = "";
  while (client.available()) {
    response += client.readStringUntil('\n');
  }
  
  Serial.println("🟢 Response dari Telegram:");
  Serial.println(response);
  
  client.stop();
}

// Method alternatif menggunakan fungsi built-in bot
void kirimKeTelegram(String payload) {
  kirimKeTelegramImproved(payload);
}