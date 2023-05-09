#include <Adafruit_MCP23008.h>
#include <Adafruit_LiquidCrystal.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <RTClib.h>
#include <Stepper.h>
#include <stdint.h>

const char* empty_str = "        ";
enum STATE{OFF, IDL, ERR, RUN};
STATE state_cur = IDL;

#define setOutput(port, pin) (port) |= (1 << (pin))
#define setInput(port, pin) (port) &= ~(1 << (pin))

#define LED_yel_reg PORTC
#define LED_yel_pin 0
#define LED_grn_reg PORTC
#define LED_grn_pin 1
#define LED_red_reg PORTC
#define LED_red_pin 2
#define LED_blue_reg PORTC
#define LED_blue_pin 3

#define p_rs_reg PORTB
#define p_rs_pin 0
#define p_en_reg PORTB
#define p_en_pin 1
#define p_d4_reg PORTB
#define p_d4_pin 2
#define p_d5_reg PORTB
#define p_d5_pin 3
#define p_d6_reg PORTD
#define p_d6_pin 4
#define p_d7_reg PORTD
#define p_d7_pin 5

#define waterSensorPower_reg PORTD
#define waterSensorPower_pin 6

#define fanPowerPin1_reg PORTC
#define fanPowerPin1_pin 5
#define fanPowerPin2_reg PORTC
#define fanPowerPin2_pin 4
#define pumpPowerPin_reg PORTC
#define pumpPowerPin_pin 3

#define DHT11_PIN 3

#define disableButton_reg PORTC
#define disableButton_pin 6

LiquidCrystal lcd(&p_rs_reg, &p_en_reg, &p_d4_reg, &p_d5_reg, &p_d6_reg, &p_d7_reg);

Stepper stepper(200, 8, 10, 9, 11);

DHT dht(DHT11_PIN, DHT11);

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

unsigned long previousTime = 0;
const long interval = 1000;

void setup(){
  Serial.begin(9600);
  
  setOutput(LED_yel_reg, LED_yel_pin);
  setOutput(LED_grn_reg, LED_grn_pin);
  setOutput(LED_red_reg, LED_red_pin);
  setOutput(LED_blue_reg, LED_blue_pin);

  setOutput(p_rs_reg, p_rs_pin);
  setOutput(p_en_reg, p_en_pin);
  setOutput(p_d4_reg, p_d4_pin);
  setOutput(p_d5_reg, p_d5_pin);
  setOutput(p_d6_reg, p_d6_pin);
  setOutput(p_d7_reg, p_d7_pin);

  setOutput(fanPowerPin1_reg, fanPowerPin1_pin);
  setOutput(fanPowerPin2_reg, fanPowerPin2_pin);
  setOutput(pumpPowerPin_reg, pump_power_pin);

  setInput(disableButton_reg, disable_button_pin);

  digitalWrite(waterSensorPower_reg, HIGH);
}

void setOutput(uint8_t port, int pin){
  port |= (1 << (pin));
}
void setInput(uint8_t port, int pin){
  port &= ~(1 << (pin));
}

void updateWaterLevel(){
  int water_level = analogRead(water_sensor_pin);
  if(water_level > 500){
    water_level_high = true;
    PORTD &= ~(1 << waterSensorPower_pin);
  } else{
    water_level_high = false;
    PORTD |= (1 << waterSensorPower_pin);
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
  
  PORTB = fan_speed;
  analogWrite(pump_power_pin, pump_speed);

  if(fan_speed > 0 || pump_speed > 0){
    PORTC |= (1 << LED_red_pin);
  } else{
    PORTC &= ~(1 << LED_red_pin);
  }
  
  if(fan_speed > 0 && pump_speed > 0){
    PORTC |= (1 << LED_grn_pin);
  } else {
    PORTC &= ~(1 << LED_grn_pin);
  }

  if(fan_speed == 0 && pump_speed > 0){
    PORTC |= (1 << LED_yel_pin);
  } else {
    PORTC &= ~(1 << LED_yel_pin);
  }

  if(fan_speed == 0 && pump_speed == 0){
    PORTC |= (1 << LED_blue_pin);
  } else{
    PORTC &= ~(1 << LED_blue_pin);
  }
}  



void loop() {
  if (millis() - previousTime >= interval){
    previousTime = millis();
  }

  uint16_t water_level = 0;
  for(int i = 0; i < 16; i++){
    water_level += analogRead(A5);
  }
  water_level /= 16;
  if (water_level < 500){
    digitalWrite(waterSensorPower_pin, HIGH);
    lcd_set_cursor(0, 0);
    lcd_write_string("Water Level LOW");
    lcd_set_cursor(0, 1);
    lcd_write_string("Water Pump ON");
    analogWrite(pumpPowerPin_pin, 255);
  } else{
    digitalWrite(waterSensorPower_pin, LOW);
    lcd_set_cursor(0, 0);
    lcd_write_string("Water Level NOMINAL");
    lcd_set_cursor(0, 1);
    lcd_write_string("Water Pump OFF");
    analogWrite(pumpPowerPin_pin, 0);
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
    lcd_write_float(humi, 2);
    lcd_write_string("%");
  } else{
    lcd_set_cursor(13, 0);
    lcd_write_string("Temp: --.- C");
    lcd_set_cursor(13, 1);
    lcd_write_string("Humidity: --.- %");
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
    lcd_set_cursor(11, 3);
    lcd_write_string(steps >= 0 ? "FWD" : "REV");
  }
}