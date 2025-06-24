#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>
#include <ESP8266WiFi.h> 
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>  
#include "config.h" 

class Hardware {
private:
  //objek tiap hardware
  static Adafruit_SSD1306 display;
  static Servo feedServo;
  static int getDistanceCM(int trigPin, int echoPin);
  
  //variabel hardware dan baterai
  static int currentFoodLevel;
  static int currentWaterLevel;
  static float currentBatteryVolt;
  static float currentBatteryPercent;

public:
  static void init();
  static void readAllSensors();
  static void updateDisplay();
  static bool feedHamster();
  
  // Getters
  static int getFoodLevel();
  static int getWaterLevel();
  static float getBatteryVolt();
  static float getBatteryPercent();
  static bool isLowBattery();
  static bool isCriticalBattery();
  
  // Display functions
  static void displayMessage(String message);
  static void displayStatus();
};

#endif