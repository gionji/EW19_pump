#include "DHT.h"

// Included libraries
#include <Wire.h>
#include <math.h>
#include <EmonLib.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <SoftwareSerial.h>


//Contants
#define EMON_CALIB      111.0
#define EMON_IRMS_CALIB  1480

// Device address
#define I2C_ADDR        0x30

#define DATA_TYPE uint8_t

// Pins declaration
#define FLOODING_PIN     A2
#define RELAY_PIN        4
#define CURRENT_PIN      A1
#define FLUX_PIN         2 
#define ONE_WIRE_BUS     6 
#define DHT11_PIN        5 

// I2C registers descriptions
#define EVENT_GET_PUMP_RPM      0x30
#define EVENT_GET_PUMP_NOVELTY  0x31
#define EVENT_GET_PUMP_TEMP     0x32
#define EVENT_GET_PUMP_CURRENT  0x33
#define EVENT_GET_PUMP_FLUX     0x34
#define EVENT_GET_ENV_TEMP      0x35
#define EVENT_GET_ENV_HUM       0x36
#define EVENT_GET_FLOODING      0x37

DATA_TYPE VALUE_PUMP_RPM      = 30;
DATA_TYPE VALUE_PUMP_NOVELTY  = 31;
DATA_TYPE VALUE_PUMP_TEMP     = 32;
DATA_TYPE VALUE_PUMP_CURRENT  = 33;
DATA_TYPE VALUE_PUMP_FLUX     = 34;
DATA_TYPE VALUE_ENV_TEMP      = 35;
DATA_TYPE VALUE_ENV_HUM       = 36;
DATA_TYPE VALUE_FLOODING      = 37;


volatile int int0count = 0;
uint8_t EVENT = 0;
OneWire  ds(ONE_WIRE_BUS);
DallasTemperature sensors(&ds);
float pumpTemp;
int pumpRpm, pumpNovelty, pumpCurrent, pumpFlux, extTemp, extHum, flooding;
EnergyMonitor pumpSensor; 
DHT dht(DHT11_PIN, DHT11);

void setup() {
  Serial.begin(115200);
  attachInterrupt(0, interrupt0Handler, RISING);
  pinMode(FLUX_PIN, INPUT);
  // Input pins
  sensors.begin();
  dht.begin();
  // Output pin muxing
  pinMode(13, OUTPUT);

  pumpSensor.current(CURRENT_PIN, EMON_CALIB);   
  
  // I2c slave mode enabling
  Wire.begin(I2C_ADDR);
  Wire.onRequest(requestEvent); // data request to slave
  Wire.onReceive(receiveEvent); // data slave received


}

void loop() {
  sensors.requestTemperatures();
  double Irms = pumpSensor.calcIrms(EMON_IRMS_CALIB);
  int0count = 0;
  
  sei();
  delay(500);
  cli();

  pumpFlux  = int0count;

  pumpRpm = 123;
  pumpNovelty = 10;
  pumpCurrent = (DATA_TYPE) Irms;
  pumpTemp = sensors.getTempCByIndex(0);
  extTemp = dht.readTemperature();; // DHT11
  extHum = dht.readHumidity(); //  DHT11
  flooding = analogRead(FLOODING_PIN);

  VALUE_PUMP_RPM      = (DATA_TYPE) 0;
  VALUE_PUMP_NOVELTY  = (DATA_TYPE) 0;
  VALUE_PUMP_TEMP     = (DATA_TYPE) pumpTemp;
  VALUE_PUMP_CURRENT  = (DATA_TYPE) pumpCurrent;
  VALUE_PUMP_FLUX     = (DATA_TYPE) pumpFlux;
  VALUE_ENV_TEMP      = (DATA_TYPE) extTemp;
  VALUE_ENV_HUM       = (DATA_TYPE) extHum;
  VALUE_FLOODING      = (DATA_TYPE) (flooding >> 2);


//  Serial.print("rpm: ");
//  Serial.print(VALUE_PUMP_RPM);
//  Serial.print(" pn: ");
//  Serial.print(VALUE_PUMP_NOVELTY);
  if(Serial.available() > 0){
      if(Serial.read() == '1'){
        Serial.print("[");
        Serial.print(VALUE_PUMP_TEMP);
        Serial.print(",");
        Serial.print(VALUE_PUMP_CURRENT);
        Serial.print(",");
        Serial.print(VALUE_PUMP_FLUX);
        Serial.print(",");
        Serial.print(VALUE_ENV_TEMP);
        Serial.print(",");
        Serial.print(VALUE_ENV_HUM);
        Serial.print(",");
        Serial.print(VALUE_FLOODING);
        Serial.println("]");
     }
  }
  
}


void interrupt0Handler(){
  int0count++;
}


// I2C management
void receiveEvent(int countToRead) {
  byte x;
  while (0 < Wire.available()) {
    x = Wire.read();
    //Serial.println(x, HEX);
  }
  String message = "Receive event: ";
  String out = message + x;
  EVENT = x;
}

void requestEvent() {
  String event_s = "0xFF";
  switch (EVENT) {
    case EVENT_GET_PUMP_RPM: 
      Wire.write(VALUE_PUMP_RPM);
      break;
    case EVENT_GET_PUMP_NOVELTY: 
      Wire.write(VALUE_PUMP_NOVELTY);
      break;
    case EVENT_GET_PUMP_TEMP: 
      Wire.write(VALUE_PUMP_TEMP);
      break;
    case EVENT_GET_PUMP_CURRENT: 
      Wire.write(VALUE_PUMP_CURRENT);
      break;
    case EVENT_GET_PUMP_FLUX: 
      Wire.write(VALUE_PUMP_FLUX);
      break;
    case EVENT_GET_ENV_TEMP: 
      Wire.write(VALUE_ENV_TEMP);
      break;
    case EVENT_GET_ENV_HUM: 
      Wire.write(VALUE_ENV_HUM);
      break;
    case EVENT_GET_FLOODING: 
      Wire.write(VALUE_FLOODING);
      break;
    default:
      Wire.write(0xFF);
      break;
    }
}


