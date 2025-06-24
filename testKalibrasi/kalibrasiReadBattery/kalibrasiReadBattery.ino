//lib 
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//pin
const int voltReadPin = A0; 

//debug
#define SDA_PIN 4 //D2
#define SCL_PIN 5 //D1 

// other macro
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

#define skalaVoltage 3.836538462 // (Vawal / Vakhir)
#define analogReadMaxBit 1023
#define analogReadMaxVolt 3.3
#define voltDividerVolt 3.12  
#define offsetAV 17 //tes a0 to gnd

//declare
int analogVal = 0; 
float voltVal = 0.0;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN); 

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Alamat I2C umum: 0x3C
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); 
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void loop() {
  analogVal = analogRead(voltReadPin);
  voltVal = (((analogVal - offsetAV)* (analogReadMaxVolt / analogReadMaxBit))) * skalaVoltage  ;
  // Serial.println(voltVal);
  Serial.println(analogVal);
  // if(voltVal < 0) voltVal = 0;

  display.clearDisplay();
  display.setCursor(0, 10);
  display.print("Voltage : ");
  display.print(voltVal);
  display.println("V");
  display.print("analog read : ");
  display.print(analogVal);
  display.display(); 

  delay(300);
}