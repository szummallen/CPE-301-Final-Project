#include <LiquidCrystal.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <RTClib.h>
#include <Stepper.h>

const char* empty_str = "        ";
enum STATE{OFF, IDL, ERR, RUN};
STATE state_cur = IDL;
#define setOutput(pin) pinMode(pin, OUTPUT)
#define setInput(pin) pinMode(pin, INPUT_PULLUP)

#define LED_yel 12
#define LED_grn 3
#define LED_red 4
#define LED_blue 5

#define p_rs 6
#define p_en 7
#define p_d4 8
#define p_d5 9
#define p_d6 10
#define p_d7 11

#define waterSensorPower 12

#define fanPowerPin1 A0
#define fanPowerPin2 A1
#define pumpPowerPin A2

#define disableButton A3

void setOutput(int pin){ pinMode(pin, OUTPUT);}
void setInput(int pin){ pinMode(pin, INPUT_PULLUP);}

setOutput(LED_yel);
setOutput(LED_grn);
setOutput(LED_red);
setOutput(LED_blue);

setOutput(p_rs);
setOutput(p_en);
setOutput(p_d4);
setOutput(p_d5);
setOutput(p_d6);
setOutput(p_d7);

setOutput(fanPowerPin1);
setOutput(fanPowerPin2);
setOutput(pumpPowerPin);

setInput(disableButton);

LiquidCrystal lcd(p_rs, p_en, p_d4, p_d5, p_d6, p_d7);

Stepper stepper(200, 8, 10, 9, 11);

DHT dht(2, DHT11);

RTC_DS3231 rtc;

int water_sensor_pin = A0;
volatile bool water_level_high = false;

int fan_power_pin1 = A1;
int fan_power_pin2 = A2;
int pump_power_pin = A3;
int fan_speed = 0;
int pump_speed = 0;

int stepper_speed = 100;

int disable_button_pin = 4;
volatile bool disable_button_pressed = false;

void setup(){
  Serial.begin(9600);
  
  setOutput(LED_yel);
  setOutput(LED_grn);
  setOutput(LED_red);
  setOutput(LED_blue);

  setOutput(p_rs);
  setOutput(p_en);
  setOutput(p_d4);
  setOutput(p_d5);
  setOutput(p_d6);
  setOutput(p_d7);

  setOutput(fanPowerPin1);
  setOutput(fanPowerPin2);
  setOutput(pumpPowerPin);

  setInput(disableButton);

  digitalWrite(waterSensorPower, HIGH);
}



void updateWaterLevel(){
  int water_level = analogRead(water_sensor_pin);
  if(water_level > 500){
    water_level_high = true;
    digitalWrite(waterSensorPower, LOW);
  } else{
    water_level_high = false;
    digitalWrite(waterSensorPower, HIGH);
  }
}

void readDHT11(float &temperature, float &humidity){
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
}

void updateFanAndPump(float temperature, float humidity){
  if(temperature > 25 || humidity > 60){
    fan_speed = 255;
    pump_speed = 255;
  } else if(temperature > 23 || humidity > 50){
    fan_speed = 150;
    pump_speed = 150;
  } else{
    fan_speed = 0;
    pump_speed = 0;
  }
  
  analogWrite(fan_power_pin1, fan_speed);
  analogWrite(fan_power_pin2, fan_speed);
  analogWrite(pump_power_pin, pump_speed);

  digitalWrite(LED_red, fan_speed > 0 || pump_speed > 0);
  digitalWrite(LED_grn, fan_speed > 0 && pump_speed > 0);
  digitalWrite(LED_yel, fan_speed == 0 && pump_speed > 0);
  digitalWrite(LED_blue, fan_speed == 0 && pump_speed == 0);

void loop() {
  uint16_t water_level = 0;
  for(int i = 0; i < 16; i++){
    water_level += analogRead(A5);
  }
  water_level /= 16;
  if (water_level < WATER_LEV_THRESHOLD){
    digitalWrite(waterSensorPower, HIGH);
    lcd_set_cursor(0, 0);
    lcd_write_string("Water Level LOW");
    lcd_set_cursor(0, 1);
    lcd_write_string("Water Pump ON");
    analogWrite(pumpPowerPin, 255);
  } else{
    digitalWrite(waterSensorPower, LOW);
    lcd_set_cursor(0, 0);
    lcd_write_string("Water Level NOMINAL");
    lcd_set_cursor(0, 1);
    lcd_write_string("Water Pump OFF");
    analogWrite(pumpPowerPin, 0);
  }


  int chk = DHT.read11(DHT11_PIN);
  if (chk == DHTLIB_OK){
    float temp = DHT.temperature;
    float humi = DHT.humidity;
    lcd_set_cursor(13, 0);
    lcd_write_string("Temp:");
    lcd_set_cursor(18, 0);
    lcd_write_float(temp, 2);
    lcd_write_string("C");
    lcd_set_cursor(13, 1);
    lcd_write_string("Humidity:");
    lcd_set_cursor(22, 1);
    lcd_write_float(hum, 2);
    lcd_write_string("%");
  } else{
    lcd_set_cursor(13, 0);
    lcd_write_string("Temp: --.- C");
    lcd_set_cursor(13, 1);
    lcd_write_string("Humidity: --.- %")
  }

  if(digitalRead(disableButton) == LOW){
    lcd_set_cursor(0, 2);
    lcd_write_string("Vent Control OFF");
  } else {
    int pot_value = analogRead(A4);
    int steps = map(pot_value, 0, 1023, -MAX_STEPS, MAX_STEPS);
    stepper.step(steps);
    lcd_set_cursor(0, 2);
    lcd_write_string("Vent Control ON");
    lcd_set_cursor(0, 3);
    lcd_write_string("Direction:");
    lcd_set_cursor(11, 0);
    lcd_write_string(steps >= 0 ? "FWD" : "REV");
  }
  delay(1000);
  }
}