#include <Arduino.h>
#include "Wire.h"
#include "SHT31.h"

#define SHT31_ADDRESS 0x44

// inputs
#define PIN_BUTTON_UP 21
#define PIN_BUTTON_DOWN 20
#define PIN_TEMP_MODE_TOGGLE 19
#define PIN_HEAT_MODE_TOGGLE 18

// outputs
#define PIN_LED_HEAT 4
#define PIN_LED_COOL 5
#define PIN_LED_FAN 6

// global configuration
const float tempIncrementC = 0.5;
const float hvacOnBufferC = 0.5;
const unsigned long hvacChangeDebounce = 5000;  // 5 seconds
const unsigned long writeDebounce = 1000;  // 1 second
const unsigned long buttonDebounce = 1000;  // 1 second

enum TempMode {
  C = 0,
  F = 1
};

enum HeatMode {
  Heat = 0,
  Cool = 1,
  Off = 2
};

const char* getHeatModeString(HeatMode currentMode) {
  switch (currentMode) {
    case Off: return "Off";
    case Heat: return "Heat";
    case Cool: return "Cool";
  }
}

class ThermostatController {
    // internal timers for debouncing
    unsigned long _lastChangeStamp = 0;
    unsigned long _lastWrite = 0;
    unsigned long _lastHeatTempChange = 0;
    unsigned long _lastCoolTempChange = 0;
    unsigned long _lastTempModeToggle = 0;
    unsigned long _lastHeatModeToggle = 0;

    float _currentTempC;
    float _currentHumidityRel;
    bool _isHeatOn = false;
    bool _isCoolOn = false;
    bool _isFanOn = false;
    float _setHeatTempC = 21.0;
    float _setCoolTempC = 21.0;
    TempMode _tempMode = C;
    HeatMode _heatMode = Cool;

    // sensor
    SHT31 _environmentSensor;

    public:
      unsigned long LastChangeStamp() const { return _lastChangeStamp; }

      SHT31 & EnvironmentSensor() { return _environmentSensor; }

      float CurrentTempC() const {return _currentTempC;}
      void CurrentTempC(float currentTempC) { _currentTempC = currentTempC; }

      void Initialize() {
        Wire.begin();
        Wire.setClock(100000);
        _environmentSensor.begin();
      }


};

// configuration options
SHT31 sht;
ThermostatController thermostat;

// global state
float currentTempC;
float currentHumidityRel;
bool isHeatOn = false;
bool isCoolOn = false;
bool isFanOn = false;
float setHeatTempC = 21.0;
float setCoolTempC = 21.0;
TempMode tempMode = C;
HeatMode heatMode = Cool;

// timers


// state setters
void incrementHeat() {
  setHeatTempC += tempIncrementC;
}

void decrementHeat() {
  setHeatTempC -= tempIncrementC;
}

void incrementCool() {
  setCoolTempC += tempIncrementC;
}

void decrementCool() {
  setCoolTempC -= tempIncrementC;
}

// conversion functions
float convertCToF(float tempC) {
  return (tempC * 9 / 5) + 32;
}

// control functions
void buttonUp() {
  switch(heatMode) {
    case Off: 
      return;
    case Heat:
      incrementHeat();
      break;
    case Cool:
      incrementCool();
      break;
  };
}

void buttonDown() {
    switch(heatMode) {
    case Off: 
      return;
    case Heat:
      decrementHeat();
      break;
    case Cool:
      decrementCool();
      break;
  };
}

void tempModeToggle() {
  tempMode = tempMode == C ? F : C;
}

void heatModeToggle() {
  switch(heatMode) {
    case Off:
      heatMode = Heat;
      break;
    case Heat:
      heatMode = Cool;
      break;
    case Cool:
      heatMode = Off;
      break;
  }
}

void controlHandler() {
  if(digitalRead(PIN_BUTTON_UP)) {
    buttonUp();
  }
  
  if(digitalRead(PIN_TEMP_MODE_TOGGLE)){
    tempModeToggle();
  }

  if(digitalRead(PIN_HEAT_MODE_TOGGLE)) {
    heatModeToggle();
  }
  
  if(digitalRead(PIN_BUTTON_DOWN)) {
    buttonDown();
  }
}

// utilities
void setLastChangeStamp() {
  lastChangeStamp = millis();
}

bool isDebounced() {
  return (millis() - lastChangeStamp) < hvacChangeDebounce;
}

void setLeds() {
  digitalWrite(PIN_LED_HEAT, isHeatOn ? HIGH : LOW);
  digitalWrite(PIN_LED_COOL, isCoolOn ? HIGH : LOW);
  digitalWrite(PIN_LED_FAN, isFanOn ? HIGH : LOW);
}

void setHvacOffStates() {
  digitalWrite(PIN_LED_HEAT, LOW);
  digitalWrite(PIN_LED_COOL, LOW);
  digitalWrite(PIN_LED_FAN, LOW);
}

void setHvacHeatStates() {
  digitalWrite(PIN_LED_COOL, LOW);

  if(isHeatOn) {
    if(currentTempC >= (setHeatTempC + hvacOnBufferC)) {
      digitalWrite(PIN_LED_HEAT, LOW);
      digitalWrite(PIN_LED_FAN, LOW);
      setLastChangeStamp();
    }
  }
  else {
    if(currentTempC <= (setHeatTempC - hvacOnBufferC)) {
      digitalWrite(PIN_LED_HEAT, HIGH);
      digitalWrite(PIN_LED_FAN, HIGH);
      setLastChangeStamp();
    }
  }
}

void setHvacCoolStates() {
  digitalWrite(PIN_LED_HEAT, LOW);

  if(isCoolOn) {
    if(currentTempC <= (setCoolTempC - hvacOnBufferC)) {
      digitalWrite(PIN_LED_COOL, LOW);
      digitalWrite(PIN_LED_FAN, LOW);
      setLastChangeStamp();
    }
  }
  else {
    if(currentTempC >= (setCoolTempC + hvacOnBufferC)) {
      digitalWrite(PIN_LED_COOL, HIGH);
      digitalWrite(PIN_LED_FAN, HIGH);
      setLastChangeStamp();
    }
  }
}

void setup() {
  pinMode(PIN_BUTTON_UP, INPUT);
  pinMode(PIN_BUTTON_DOWN, INPUT);
  pinMode(PIN_TEMP_MODE_TOGGLE, INPUT);
  pinMode(PIN_HEAT_MODE_TOGGLE, INPUT);
  pinMode(PIN_LED_HEAT, OUTPUT);
  pinMode(PIN_LED_COOL, OUTPUT);
  pinMode(PIN_LED_FAN, OUTPUT);

  thermostat.Initialize();

  Serial.begin(9600);
  Serial.println(__FILE__);
  Serial.print("Library version: \t");
  Serial.println(SHT31_LIB_VERSION);

  

  Serial.print(sht.readStatus(), HEX);
  Serial.println();
}

void loop() {
  sht.read();
  thermostat.LastChangeStamp();

  currentTempC = sht.getTemperature();
  currentHumidityRel = sht.getHumidity();

  if ((millis() - lastChangeStamp) > hvacChangeDebounce){
    switch(heatMode) {
      case Off:
        setHvacOffStates();
        break;
      case Heat:
        setHvacHeatStates();
        break;
      case Cool:
        setHvacCoolStates();
        break;
    }
  }

  if((millis() - lastWrite) > writeDebounce) {
    Serial.print("\t");
    Serial.print(currentTempC, 1);
    Serial.print("\t");
    Serial.print(currentHumidityRel, 1);
    Serial.print("\t");
    Serial.print(getHeatModeString(heatMode));
    Serial.print("\t");
    Serial.print(setCoolTempC, 1);
    Serial.print("\t");
    Serial.println(setHeatTempC, 1);
    lastWrite = millis();
  }
}