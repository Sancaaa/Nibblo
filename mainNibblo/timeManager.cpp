#include "timeManager.h"
#include "hardware.h"
#include "dataLogger.h"
#include "telegramHandler.h"

WiFiUDP TimeManager::ntpUDP;
NTPClient TimeManager::timeClient(ntpUDP, NTP_SERVER, TIME_ZONE*ONE_HOUR_SECOND, 60000); 
//konveri UTC ke WITA, sinkronisasi tiap 60 detik
FeedSchedule TimeManager::schedules[MAX_SCHEDULES];
int TimeManager::scheduleCount = 0;
unsigned long TimeManager::lastTimeSync = 0;

void TimeManager::init() {
  timeClient.begin();
  
  // Add default schedules
  addSchedule("08:00", true);
  
  syncTime();
  Serial.println("✅ Time Manager initialized");
}

void TimeManager::update() {
  // sinkronisasi per jam, PASTIKAN > agar millis() tidak overflow
  if (millis() - lastTimeSync > ONE_HOUR_MILLIS) {
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

  // EPOCHTIME dimulai dari 1 januari 1970. 
  // Sedangkan struct tm (dari lib c), dimulai dari 1 januari 1900

  String TimeManager::getCurrentTime() {
    time_t rawTime = timeClient.getEpochTime(); // ambil dari NTP
    struct tm * timeInfo = localtime(&rawTime); // ubah ke struktur sec min hour dst
    char timeStr[6];  // HH:MM
    sprintf(timeStr, "%02d:%02d", timeInfo->tm_hour, timeInfo->tm_min); //simpan HH:MM
    return String(timeStr);
  }

String TimeManager::getCurrentTimeString() {
  time_t rawTime = timeClient.getEpochTime();
  struct tm * timeInfo = localtime(&rawTime);
  char timeStr[20];
  sprintf(timeStr, "%02d:%02d:%02d %02d/%02d/%04d", 
          timeInfo->tm_hour, 
          timeInfo->tm_min, 
          timeInfo->tm_sec,
          timeInfo->tm_mday, 
          timeInfo->tm_mon + 1, 
          timeInfo->tm_year + 1900);
  return String(timeStr);
}

void TimeManager::checkAutoFeedSchedule() {
  String currentTime = getCurrentTime();
  time_t rawTime = timeClient.getEpochTime();
  struct tm * timeInfo = localtime(&rawTime);
  int currentDay = timeInfo->tm_mday;
  
  for (int i = 0; i < scheduleCount; i++) {
    //jadwal aktif, waktu saat itu, belum dieksekusi
    if (schedules[i].enabled && 
        schedules[i].time == currentTime &&
        (!schedules[i].executed || schedules[i].lastDay != currentDay)) {
      
      // Execute auto feed
      if (Hardware::getFoodLevel() > FOOD_CRITICAL_THRESHOLD && Hardware::getBatteryPercent() > LOW_BATTERY_THRESHOLD) {
        Hardware::feedHamster();
        DataLogger::logFeeding("AUTO", schedules[i].time);
        TelegramHandler::sendAutoFeedNotification(schedules[i].time);
        
        // Update last day feed
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
  time.trim();  // Hilangkan spasi di awal/akhir
  if (time.length() == 4) time = "0" + time;  // Pastikan format selalu HH:MM

  // Cek duplikat
  for (int i = 0; i < scheduleCount; i++) {
    if (schedules[i].time == time) {
      Serial.println("⚠️ Jadwal \"" + time + "\" sudah ada. Tidak ditambahkan.");
      return;
    }
  }

  // Validasi format dan batas maksimal
  if (scheduleCount >= MAX_SCHEDULES) {
    Serial.println("❌ Jumlah maksimum jadwal (" + String(MAX_SCHEDULES) + ") telah tercapai.");
    return;
  }

  if (!isValidTimeFormat(time)) {
    Serial.println("❌ Format waktu tidak valid: \"" + time + "\". Gunakan format HH:MM.");
    return;
  }

  // Tambahkan jadwal
  schedules[scheduleCount].time = time;
  schedules[scheduleCount].enabled = enabled;
  schedules[scheduleCount].executed = false;
  schedules[scheduleCount].lastDay = 0;
  scheduleCount++;

  Serial.println("✅ Jadwal \"" + time + "\" berhasil ditambahkan.");
}

bool TimeManager::isValidTimeFormat(String time) {
  if (time.length() != 5 || time.charAt(2) != ':') return false;  //validasi HH:MM
  int hour = time.substring(0, 2).toInt();  // ambil karakter 0,1
  int minute = time.substring(3, 5).toInt();  // ambil karakter 3,4
  return (hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59);
}

  //hapus semua schedule
void TimeManager::clearAllSchedules() {
  scheduleCount = 0;
}

String TimeManager::getScheduleList() {
  String result = "📆 Feed Schedule:\n";
  if (scheduleCount == 0) {
    result += "- No schedule -\n";
    return result;
  }

  for (int i = 0; i < scheduleCount; i++) {
    result += String(i + 1) + ". " + schedules[i].time;
    result += schedules[i].enabled ? " ✅\n" : " ❌\n";
  }
  return result;
}


void TimeManager::removeSchedule(int index) {
  if (index < 0 || index >= scheduleCount) return;  // Validasi batas index

  for (int i = index; i < scheduleCount - 1; i++) {
    schedules[i] = schedules[i + 1];  // Geser semua elemen ke kiri
  }
  scheduleCount--;  // Kurangi jumlah total jadwal
}
