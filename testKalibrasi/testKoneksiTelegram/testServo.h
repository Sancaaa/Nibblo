#include <ESP8266WiFi.h>
#include <Servo.h>

// Makro untuk konfigurasi
#define SERVO_PIN D8
#define SERVO_FEED_ANGLE 0
#define SERVO_CLOSE_ANGLE 180

Servo servoMotor;

void setup() {
  Serial.begin(9600);

  // Attach servo ke pin yang sudah didefinisikan
  servoMotor.attach(SERVO_PIN, 500, 2400);

}

void loop() {
  
  // Gerakkan ke posisi awal
  servoMotor.write(SERVO_FEED_ANGLE);
  Serial.println(F("Servo ke posisi awal (0 derajat)"));
  delay(1000); // tunggu 1 detik

  // Gerakkan ke posisi akhir
  servoMotor.write(SERVO_CLOSE_ANGLE);
  Serial.println(F("Servo ke posisi akhir (180 derajat)"));
  delay(1000); // tunggu 1 detik
}
