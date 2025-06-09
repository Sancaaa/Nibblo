//lib 
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//pin
const int voltReadPin = A0; 


// other macro
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

#define faktorKalibrasi 1 // (Vmultimeter / Va0)
#define volDividerVolt 3.12
#define reffV 12
#define analogReadMaxBit 1023
#define analogReadMaxVolt 3.3 
#define offsetVolt 0.76 //tes a0 to gnd

//declare
int analogVal = 0; 
float voltVal = 0.0;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);

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
  voltVal = ((analogVal * (analogReadMaxVolt / analogReadMaxBit)) * (reffV / volDividerVolt) - offsetVolt) * faktorKalibrasi;
  // Serial.println(voltVal);
  if(voltval < 0) voltval = 0;

  display.clearDisplay();
  display.setCursor(0, 10);
  display.print("Voltage : ");
  display.print(voltVal, 2);
  display.print("V");
  display.display(); 

  delay(300);
}