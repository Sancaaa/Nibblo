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
  static Adafruit_SSD1306 display;
  static Servo feedServo;
  static NewPing sonarFood;
  static NewPing sonarWater;
  
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
  
  // Display functions
  static void displayMessage(String message);
  static void displayStatus();
};

// Hardware Implementation
Adafruit_SSD1306 Hardware::display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Servo Hardware::feedServo;
NewPing Hardware::sonarFood(TRIG_FOOD_PIN, ECHO_FOOD_PIN, MAX_DISTANCE);
NewPing Hardware::sonarWater(TRIG_WATER_PIN, ECHO_WATER_PIN, MAX_DISTANCE);

int Hardware::currentFoodLevel = 0;
int Hardware::currentWaterLevel = 0;
float Hardware::currentBatteryVolt = 0.0;
float Hardware::currentBatteryPercent = 0.0;

void Hardware::init() {
  // Initialize I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Initialize display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  displayMessage("Initializing...");
  
  // Initialize servo
  feedServo.attach(SERVO_PIN);
  feedServo.write(SERVO_CLOSE_ANGLE);
  
  Serial.println("✅ Hardware initialized");
}

void Hardware::readAllSensors() {
  // Read battery voltage
  int analogVal = analogRead(VOLT_READ_PIN);
  currentBatteryVolt = ((analogVal * (ANALOG_MAX_VOLT / ANALOG_MAX_BIT)) * 
                       (REFERENCE_VOLT / VOLTAGE_DIVIDER_VOLT) - VOLTAGE_OFFSET) * 
                       CALIBRATION_FACTOR;
  if (currentBatteryVolt < 0) currentBatteryVolt = 0;
  
  // Calculate battery percentage
  currentBatteryPercent = ((currentBatteryVolt - BATTERY_MIN_VOLT) / 
                          (BATTERY_MAX_VOLT - BATTERY_MIN_VOLT)) * 100;
  if (currentBatteryPercent > 100) currentBatteryPercent = 100;
  if (currentBatteryPercent < 0) currentBatteryPercent = 0;
  
  // Read food level
  int foodDistance = sonarFood.ping_cm();
  if (foodDistance == 0) foodDistance = MAX_DISTANCE;
  currentFoodLevel = map(foodDistance, 20, 5, 0, 100);
  if (currentFoodLevel > 100) currentFoodLevel = 100;
  if (currentFoodLevel < 0) currentFoodLevel = 0;
  
  // Read water level
  int waterDistance = sonarWater.ping_cm();
  if (waterDistance == 0) waterDistance = MAX_DISTANCE;
  currentWaterLevel = map(waterDistance, 15, 3, 0, 100);
  if (currentWaterLevel > 100) currentWaterLevel = 100;
  if (currentWaterLevel < 0) currentWaterLevel = 0;
}

void Hardware::feedHamster() {
  Serial.println("Feeding hamster...");
  feedServo.write(SERVO_FEED_ANGLE);
  delay(1000);
  feedServo.write(SERVO_CLOSE_ANGLE);
  delay(500);
}

void Hardware::displayMessage(String message) {
  display.clearDisplay();
  display.setCursor(0, 10);
  display.print(message);
  display.display();
}

void Hardware::updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Bat: ");
  display.print(currentBatteryVolt, 1);
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
  display.print(WiFi.status() == WL_CONNECTED ? "OK" : "ERR");
  
  display.display();
}

#endif