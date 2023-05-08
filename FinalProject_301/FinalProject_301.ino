#include <DHT.h>
#include <DHT_U.h>
#include <LiquidCrystal.h>

const char* empty_str = "        ";
enum STATE{OFF, IDL, ERR, RUN};
STATE state_cur = IDL;

#define lowWaterWarning 1
#define highTempWarning 10
#define waterSensorPower 12
#define waterSensorPin A0
#define humiditySensorPin 9
#define fanPowerPin1 40
#define fanPowerPin2 41
#define pumpPowerPin 42
#define disableButton 24
#define DHTPIN 11
#define DHTTYPE DHT11

#define LED_yel 2
#define LED_grn 3
#define LED_red 4
#define LED_blue 5

#define p_rs 6
#define p_en 7
#define p_d4 8
#define p_d5 9
#define p_d6 10
#define p_d7 11



DHT humiditySensor(humiditySensorPin, DHT11);
int humidity = (int)humiditySensor.readHumidity();
int temperature = (int)humiditySensor.readTemperature();

float readTemp(){
return humiditySensor.readTemperature();
}

DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

const int p_rs = 8, p_en = 7, p_d4 = 6, p_d5 = 5, p_d6 = 4, p_d7 = 3;
const int LED_yel = 11, LED_grn = 12, LED_red = 10, LED_blue = 13;

void loop(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Humidity: ");
  lcd.print(readHumidity());
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("Temperature in C: ");
  lcd.print(readTemperature());
  lcd.print((char)223);
  lcd.print("Celsius");
}

int readHumidity(){
  int chk = humiditySensor.read(humiditySensorPin);
  int humidity = humiditySensor.readHumidity(); 
  while (humidity < -15){
    delay(25);
    chk = humiditySensor.read(humiditySensorPin);
    humidity = humiditySensor.readHumidity();
  }
  return humidity;
}


int readTemperature(){
  int chk = humiditySensor.read(humiditySensorPin);
  int temperature = (int)humiditySensor.readTemperature();
  while (temperature < -15){
    delay(25);
    chk = humiditySensor.read(humiditySensorPin);
    temperature = (int)humiditySensor.readTemperature();
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
  pinMode(pumpPowerPin, OUTPUT);

  pinMode(disableButton, INPUT_PULLUP);

  Serial.begin(9600);

}

void LightsForState(STATE state) {
  digitalWrite(LED_blue, LOW);
  digitalWrite(LED_yel, LOW);
  digitalWrite(LED_grn, LOW);
  digitalWrite(LED_red, LOW);

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
      state_cur = RUN;
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
  break;
}

delay(1000);
}