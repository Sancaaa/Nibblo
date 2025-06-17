#ifndef HARDWARE_H
#define HARDWARE_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>
#include <NewPing.h>
#include "config.h"

class Hardware {
private:
  //objek tiap hardware
  static Adafruit_SSD1306 display;
  static Servo feedServo;
  static NewPing sonarFood;
  static NewPing sonarWater;
  
  //variabel hardware dan baterai
  static int currentFoodLevel;
  static int currentWaterLevel;
  static float currentBatteryVolt;
  static float currentBatteryPercent;

public:
  static void init();
  static void readAllSensors();
  static void updateDisplay();
  static void feedHamster();
  
  // Getters
  static int getFoodLevel() { return currentFoodLevel; }
  static int getWaterLevel() { return currentWaterLevel; }
  static float getBatteryVolt() { return currentBatteryVolt; }
  static float getBatteryPercent() { return currentBatteryPercent; }
  static bool isLowBattery() { return currentBatteryPercent < LOW_BATTERY_THRESHOLD; }
  static bool isCriticalBattery() { return currentBatteryPercent < CRITICAL_BATTERY_THRESHOLD; }
  
  // Display functions
  static void displayMessage(String message);
  static void displayStatus();
};

// inisiasi objek
Adafruit_SSD1306 Hardware::display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Servo Hardware::feedServo;
NewPing Hardware::sonarFood(TRIG_FOOD_PIN, ECHO_FOOD_PIN, MAX_FOOD_DISTANCE);
NewPing Hardware::sonarWater(TRIG_WATER_PIN, ECHO_WATER_PIN, MAX_WATER_DISTANCE);

//inisiasi variabel
int Hardware::currentFoodLevel = 0;
int Hardware::currentWaterLevel = 0;
float Hardware::currentBatteryVolt = 0.0;
float Hardware::currentBatteryPercent = 0.0;

void Hardware::init() {
  // inisiasi I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // inisiasi oled
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  displayMessage("Initializing...");
  
  // Initialize servo
  feedServo.attach(SERVO_PIN, 500, 2400);
  feedServo.write(SERVO_CLOSE_ANGLE);
  
  Serial.println("✅ Hardware initialized");
}

void Hardware::readAllSensors() {
  // Read battery voltage
  int analogVal = analogRead(VOLT_READ_PIN);
  if(analogVal < OFFSET_ANALOG_VALUE) analogVal = OFFSET_ANALOG_VALUE;
  currentBatteryVolt = (((analogVal - OFFSET_ANALOG_VALUE) * //hilangkan offset analog read value 
                        (ANALOG_READ_MAX_VOLT / ANALOG_READ_MAX_BIT))) *
                        VOLTAGE_SCALE;
  if (currentBatteryVolt < 0) currentBatteryVolt = 0;
  
  // Calculate battery percentage
  currentBatteryPercent = ((currentBatteryVolt - BATTERY_MIN_VOLT) / 
                          (BATTERY_MAX_VOLT - BATTERY_MIN_VOLT)) * 100;
  currentBatteryPercent = constrain(currentBatteryPercent, 0, 100);
  //constraint ngebatasin di range 0-100
  
  // Read food level
  int foodDistance = sonarFood.ping_cm();
  if (foodDistance == 0) foodDistance = MAX_FOOD_DISTANCE;
  currentFoodLevel = map(foodDistance, MAX_FOOD_DISTANCE, MIN_DISTANCE, 0, 100); 
  currentFoodLevel = constrain(currentFoodLevel, 0, 100);
  //map merubah jarak jadi persen, map(val, fromLow, fromHigh, toLow, toHigh)
  
  // Read water level
  int waterDistance = sonarWater.ping_cm();
  if (waterDistance == 0) waterDistance = MAX_WATER_DISTANCE;
  currentWaterLevel = map(waterDistance, MAX_WATER_DISTANCE, MIN_DISTANCE, 0, 100);
  currentWaterLevel = constrain(currentWaterLevel, 0, 100);
}

  // Feed
void Hardware::feedHamster() {
  Serial.println("Feeding hamster...");
  feedServo.write(SERVO_FEED_ANGLE);
  delay(1000);
  feedServo.write(SERVO_CLOSE_ANGLE);
  delay(500);
}

  //print message ke display oled
void Hardware::displayMessage(String int line = 10) { //debugging 10 line
  display.clearDisplay();
  display.setCursor(0, line);
  display.print(message);
  display.display();
}

  //update semua status hardware 
void Hardware::updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Bat: ");
  display.print(currentBatteryVolt, 2);
  display.print("V (");
  display.print(currentBatteryPercent, 0);
  display.println("%)");
  
  display.setCursor(0, 16);
  display.print("Food: ");
  display.print(currentFoodLevel);
  display.println("%");
  
  display.setCursor(0, 32);
  display.print("Water: ");
  display.print(currentWaterLevel);
  display.println("%");
  
  display.setCursor(0, 48);
  display.print("WiFi: ");
  display.print(WiFi.status() == WL_CONNECTED ? "OK" : "ERROR");
  
  display.display();
}

#endif