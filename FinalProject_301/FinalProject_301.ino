#include <DHT.h>
#include <DHT_U.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
DHT humiditySensor(humiditySensorPin, DHT11);

const int p_rs = 8, p_en = 7, p_d4 = 6, p_d5 = 5, p_d6 = 4, p_d7 = 3;
const int LED_yel = 11, LED_grn = 12, LED_red = 10, LED_blue = 13;

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
enum STATE{OFF, IDL, ERR, RUN};
STATE state_cur = IDL;

dht humiditySensor;

int readHumidity(){
  int chk = humiditySensor.read(humiditySensorPin);
  int humidity = humiditySensor.humidity; 
  while (humidity < -15){
    delay(25);
    chk = humiditySensor.read(humiditySensorPin);
    humidity = humiditySensor.temperature;
  }
  return humidity;
}


int readTemperature(){
  int chk = humiditySensor.read(humiditySensorPin);
  int temperature = humiditySensor.temperature;
  while (temperature < -15){
    delay(25);
    chk = humiditySensor.read(humiditySensorPin);
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
  pinMode(LED_yel, OUTPUT);
  pinMode(LED_grn, OUTPUT);
  pinMode(LED_red, OUTPUT);
  pinMode(LED_blue, OUTPUT);

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

  bool b_red = digitalRead(disableButton);

  switch(state_cur){
    case OFF:
    if (b_red){
      if(readWaterSensor() < lowWaterWarning){
        state_cur = ERR;
        disablePump();
        disableFans();        
      }      
    }
      else if(readTemp() > highTempWarning){
        state_cur = RUN;
        enableFans();
        enablePump();
      }
      else{
        state_cur = IDL;
        disablePump();
        disableFans();
      }     
    }
    break;    

    case IDL:
    if(readWaterSensor() < lowWaterWarning){
      state_cur = ERR;
      disablePump();
      disableFans();
    }
    else if(b_red){
      state_cur = OFF;
      disablePump();
      disableFans();
    }
    else if(readTemp() > highTempWarning){
      state_cur = RUN
      enableFans();
      enablePump();
    }
  break;

  case ERR:
  if(readWaterSensor() > lowWaterWarning){
    state_cur = IDL;
    disablePump();
    disableFans();
  }
  break;

  case RUN:
  if(readWaterSensor() < lowWaterWarning){
    state_cur = ERR;
    disablePump();
    disableFans();
  }
  else if(b_red == LOW){
    state_cur = OFF;
    disablePump();
    disableFans();
  }
  else if(readTemp() < highTempWarning){
    state_cur = IDL;
    disablePump();
    disableFans();
  }
}

delay(1000);
}