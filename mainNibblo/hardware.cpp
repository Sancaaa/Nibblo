#include "hardware.h"


// inisiasi objek
Adafruit_SSD1306 Hardware::display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Servo Hardware::feedServo;

//inisiasi variabel
int Hardware::currentFoodLevel = 0;
int Hardware::currentWaterLevel = 0;
float Hardware::currentBatteryVolt = 0.0;
float Hardware::currentBatteryPercent = 0.0;

//implementasi fungsi
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
  
  // Initialize sensors
  pinMode(TRIG_FOOD_PIN, OUTPUT);
  pinMode(ECHO_FOOD_PIN, INPUT);
  pinMode(TRIG_WATER_PIN, OUTPUT);
  pinMode(ECHO_WATER_PIN, INPUT);

  // Initialize servo
  feedServo.attach(SERVO_PIN, 500, 2400);
  feedServo.write(SERVO_CLOSE_ANGLE);
  
  //baca baterai awal
  pinMode(VOLT_READ_PIN, INPUT);
  readAnalogVoltage();
  
  Serial.println("✅ Hardware initialized");
}

int Hardware::getDistanceCM(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH, 30000); // timeout 30 ms (maks 5 meter)
  int distance = duration * 0.0343 / 2;
  return distance;
}

void Hardware::readAllSensors() {
  delay(50);
  readAnalogVoltage();
  readFoodSensor();
  readWaterSensor();
}

void Hardware::readAnalogVoltage(){
  // Read battery voltage
  // int analogVal = analogRead(VOLT_READ_PIN);

  WiFi.mode(WIFI_OFF);
  delay(100); // biar ADC stabil
  int analogVal = analogRead(VOLT_READ_PIN);
  WiFi.mode(WIFI_STA); // atau WIFI_ON
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

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
}

void Hardware::readFoodSensor(){
  // Read food level
  int foodDistance = getDistanceCM(TRIG_FOOD_PIN, ECHO_FOOD_PIN);
  if (foodDistance <= 0 || foodDistance > MAX_FOOD_DISTANCE) foodDistance = MAX_FOOD_DISTANCE;
  currentFoodLevel = map(foodDistance, MAX_FOOD_DISTANCE, MIN_DISTANCE, 0, 100); 
  currentFoodLevel = constrain(currentFoodLevel, 0, 100);
  //map merubah jarak jadi persen, map(val, fromLow, fromHigh, toLow, toHigh)
}

void Hardware::readWaterSensor(){
  // Read water level
  int waterDistance = getDistanceCM(TRIG_WATER_PIN, ECHO_WATER_PIN);
  if (waterDistance <= 0 || waterDistance > MAX_WATER_DISTANCE) waterDistance = MAX_WATER_DISTANCE;
  currentWaterLevel = map(waterDistance, MAX_WATER_DISTANCE, MIN_DISTANCE, 0, 100);
  currentWaterLevel = constrain(currentWaterLevel, 0, 100);
}

  // Feed
bool Hardware::feedHamster() {
  if (currentFoodLevel < 10) {
    Serial.println("Food too low!");
    displayMessage("Food too low!");
    return false;
  }

  Serial.println("Feeding hamster...");
  displayMessage("Feeding hamster...");
  feedServo.write(SERVO_FEED_ANGLE);
  delay(1000);
  feedServo.write(SERVO_CLOSE_ANGLE);
  delay(500);
  return true;
}

  //print message ke display oled
void Hardware::displayMessage(String message) { 
  int line = 10;  //debugging 10 line
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

  // getter
  int Hardware::getFoodLevel() { return currentFoodLevel; }
  int Hardware::getWaterLevel() { return currentWaterLevel; }
  float Hardware::getBatteryVolt() { return currentBatteryVolt; }
  float Hardware::getBatteryPercent() { return currentBatteryPercent; }
  bool Hardware::isLowBattery() { return currentBatteryPercent < LOW_BATTERY_THRESHOLD; }
  bool Hardware::isCriticalBattery() { return currentBatteryPercent < CRITICAL_BATTERY_THRESHOLD; }