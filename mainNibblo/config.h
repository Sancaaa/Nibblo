#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h> 

// Pin 
const int VOLT_READ_PIN = A0;
#define SERVO_PIN 15    //D8
#define TRIG_FOOD_PIN 14    //D5
#define ECHO_FOOD_PIN 12    //D6
#define TRIG_WATER_PIN 13   //D7
#define ECHO_WATER_PIN 2    //D4
#define SDA_PIN 4   //D2 
#define SCL_PIN 5   //D1 

// Schedule
#define MAX_SCHEDULES 10

// NTP
#define TIME_ZONE 8
#define NTP_SERVER "pool.ntp.org"
#define NTP_UPDATE_INTERVAL 60000

// Timing & Interval
#define BOT_CHECK_INTERVAL 5000
#define SENSOR_READ_INTERVAL 5000
#define DISPLAY_UPDATE_INTERVAL 2000
#define ALERT_CHECK_INTERVAL 30000
#define DATA_LOG_INTERVAL 300000 

// Hardware Config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define MIN_DISTANCE 2
#define MAX_FOOD_DISTANCE 13.6
#define MAX_WATER_DISTANCE 6.3
#define SERVO_FEED_ANGLE 0
#define SERVO_CLOSE_ANGLE 180

// Konstan dan treshold power
#define BATTERY_MIN_VOLT 6.0
#define BATTERY_MAX_VOLT 8.4
#define SLEEP_DURATION_SECONDS 300
#define LOW_BATTERY_THRESHOLD 15
#define CRITICAL_BATTERY_THRESHOLD 10

// Konstan read baterai
#define VOLTAGE_SCALE 3.836538462  //Voltage Awal / Voltage setelah voltage divider 
#define ANALOG_READ_MAX_BIT 1023.0
#define ANALOG_READ_MAX_VOLT 3.3
#define VOLTAGE_DIVIDER_VOLT 3.12 
#define OFFSET_ANALOG_VALUE 17  //tes a0 dengan groung

// Threshold hardware
#define FOOD_WARNING_THRESHOLD 30.0
#define FOOD_CRITICAL_THRESHOLD 15.0
#define WATER_WARNING_THRESHOLD 25.0
#define WATER_CRITICAL_THRESHOLD 10.0
#define ALERT_COOLDOWN_MINUTES 30

#endif