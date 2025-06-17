#ifndef CONFIG_H
#define CONFIG_H

// Pin 
#define VOLT_READ_PIN A0
#define SERVO_PIN D8
#define TRIG_FOOD_PIN D5
#define ECHO_FOOD_PIN D6
#define TRIG_WATER_PIN D7
#define ECHO_WATER_PIN D4
#define SDA_PIN D2
#define SCL_PIN D1

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
#define MAX_FOOD_DISTANCE 13.6f
#define MAX_WATER_DISTANCE 6.3f
#define SERVO_FEED_ANGLE 0
#define SERVO_CLOSE_ANGLE 180

// Konstan dan treshold power
#define BATTERY_MIN_VOLT 9.0f
#define BATTERY_MAX_VOLT 12.3f
#define SLEEP_DURATION_SECONDS 300
#define LOW_BATTERY_THRESHOLD 15
#define CRITICAL_BATTERY_THRESHOLD 10

// Konstan read baterai
#define VOLTAGE_SCALE 3.836538462  //Voltage Awal / Voltage setelah voltage divider 
#define ANALOG_READ_MAX_BIT 1023
#define ANALOG_READ_MAX_VOLT 3.3f
#define VOLTAGE_DIVIDER_VOLT 3.12f 
#define OFFSET_ANALOG_VALUE 17  //tes a0 dengan groung

// Threshold hardware
#define FOOD_WARNING_THRESHOLD 30f
#define FOOD_CRITICAL_THRESHOLD 15f
#define WATER_WARNING_THRESHOLD 25f
#define WATER_CRITICAL_THRESHOLD 10.0f
#define ALERT_COOLDOWN_MINUTES 30

#endif