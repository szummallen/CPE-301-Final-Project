---------------------------------------------------------------------------------
Steven Zummallen
CPE 301.1001 
Final Project: Swamp Cooler
---------------------------------------------------------------------------------

#include "LiquidCrystal.h"
#include <dht.h>


const int p_rs = 8, p_en = 7, p_d4 = 6, p_d5 = 5, p_d6 = 4, p_d7 = 3;
const int 1_yel = 11, 1_grn = 12, 1_red = 10, 1_blue = 13;

#define lowWaterWarning 1
#define highTempWarning 10

#define waterSensorPower 2
#define waterSensorPin A0

#define humiditySensorPin 9

#define fanPowerPin1 40
#define fanPowerPin2 41
#define pumpPowerPin 42

#define disableButton 24

const char* empty_str = "        ";
enum STATE(OFF, IDL, ERR, RUN);
STATE state_cur = IDL;

dht humiditySensor;

int readHumidity(){
  int chk = humiditySensor.read11(humiditySensorPin);
  int humidity = humiditySensor.humidity; 
  while (humidity < -15){
    delay(25);
    chk = humiditySensor.read11(humiditySensorPin);
    humidity = humiditySensor.temperature;
  }
  return humidity;
}


int readTemp(){
  int chk = humiditySensor.read11(humiditySensorPin);
  int temperature = humiditySensor.temperature;
  while (temperature < -15){
    delay(25);
    chk = humiditySensor.read11(humiditySensorPin);
    temperature = humiditySensor.temperature;
  }
  return temperature;
}

int readWaterSensor(){
  digitalWrite(waterSensorPower, HIGH);
  delay(10);
  int val = analogRead(waterSensorPin);
  digitalWrite(waterSensorPower, LOW);
  return val;
}

void enableFans(){
  digitalWrite(fanPowerPin1, HIGH);
  digitalWrite(fanPowerPin2, HIGH);
}

void disableFans(){
  digitalWrite(fanPowerPin1, LOW);
  digitalWrite(fanPowerPin2, LOW);
}

void enablePump(){
  digitalWrite(pumpPowerPin, HIGH);
}

void disablePump(){
  digitalWrite(pumpPowerPin, LOW);
}


void setup() {
  lcd.begin(16, 2);
  pinMode(1_yel, OUTPUT);
  pinMode(1_grn, OUTPUT);
  pinMode(1_red, OUTPUT);
  pinMode(1_blu, OUTPUT);

  pinMode(p_rs, OUTPUT);
  pinMode(p_en, OUTPUT);
  pinMode(p_d4, OUTPUT);
  pinMode(p_d5, OUTPUT);
  pinMode(p_d6, OUTPUT);
  pinMode(p_d7, OUTPUT);

  pinMode(waterSensorPower, OUTPUT);
  digitalWrite(waterSensorPower, LOW);

  pinMode(fanPowerPin1, OUTPUT);
  pinMode(fanPowerPin2, OUTPUT);
  pinMode(fanPowerPin, OUTPUT);

  pinMode(disableButton, INPUT_PULLUP);

  Serial.begin(9600);

}

void loop() {
  print_status(state_cur);
  handleLightsForState(state_cur);
}
