#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

#define SDA_PIN D2  
#define SCL_PIN D1  

#define TRIG_FOOD_PIN D5
#define ECHO_FOOD_PIN D6
#define TRIG_WATER_PIN D7
#define ECHO_WATER_PIN D4

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

float readUltrasonicCM(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // timeout 30 ms
  if (duration == 0) return -1; // error baca

  float distanceCM = duration * 0.0343 / 2; // konversi ke cm
  return distanceCM;
}

void setup() {
  Serial.begin(115200);

  // Inisialisasi I2C manual ke pin yang kamu tentukan
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); 
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  pinMode(TRIG_FOOD_PIN, OUTPUT);
  pinMode(ECHO_FOOD_PIN, INPUT);
  pinMode(TRIG_WATER_PIN, OUTPUT);
  pinMode(ECHO_WATER_PIN, INPUT);

  Serial.println("Kalibrasi Sensor Ultrasonik");
}

void loop() {
  float foodDistance = readUltrasonicCM(TRIG_FOOD_PIN, ECHO_FOOD_PIN);
  float waterDistance = readUltrasonicCM(TRIG_WATER_PIN, ECHO_WATER_PIN);

  Serial.print("Jarak Makanan: ");
  // if (foodDistance >= 0)
    Serial.print(foodDistance, 2);
  // else
    // Serial.print("Error");
  Serial.print(" cm\t");

  Serial.print("Jarak Air: ");
  // if (waterDistance >= 0)
    Serial.print(waterDistance, 2);
  // else
    // Serial.print("Error");
  Serial.println(" cm");

  // OLED Display
  display.clearDisplay();
  display.setCursor(0, 10);
  if (foodDistance >= 0) {
    display.print("Food: ");
    display.print(foodDistance, 2);
    display.println(" cm");
  } else {
    display.println("Food: Error");
  }

  display.setCursor(0, 30);
  if (waterDistance >= 0) {
    display.print("Water: ");
    display.print(waterDistance, 2);
    display.println(" cm");
  } else {
    display.println("Water: Error");
  }

  display.display(); 

  delay(1000); // Baca setiap 1 detik
}
