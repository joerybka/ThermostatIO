#include <Arduino.h>
#include "Wire.h"
#include "SHT31.h"

#include "settings.h"
#include "Debouncer.h"
#include "SettingsController.h"
#include "SensorController.h"
#include "HvacController.h"

// controllers
SettingsController settingsController = SettingsController(buttonDebounceMs, buttonDebounceMs);
SensorController sensorController = SensorController(sensorReadBounceMs);
HvacController hvacController = HvacController(hvacChangeDebounceMs);

/// @brief debouncer to control the frequency of writing to the serial console
Debouncer writeDebouncer = Debouncer(writeDebounceMs);

// status writer
void statusWriter() {
  Serial.print("\t");
  Serial.print(sensorController.CurrentTempC(), 1);
  Serial.print("\t");
  Serial.print(sensorController.CurrentHumidityRel(), 1);
  Serial.print("\t");
  Serial.print(settingsController.GetHeatModeString());
  Serial.print("\t");
  Serial.print(settingsController.SetCoolTempC(), 1);
  Serial.print("\t");
  Serial.println(settingsController.SetHeatTempC(), 1);
}

void setup() {
  // set up all the pins
  pinMode(PIN_BUTTON_UP, INPUT);
  pinMode(PIN_BUTTON_DOWN, INPUT);
  pinMode(PIN_TEMP_MODE_TOGGLE, INPUT);
  pinMode(PIN_HEAT_MODE_TOGGLE, INPUT);
  pinMode(PIN_LED_HEAT, OUTPUT);
  pinMode(PIN_LED_COOL, OUTPUT);
  pinMode(PIN_LED_FAN, OUTPUT);

  // write headers to the serial console
  Serial.begin(9600);
  Serial.println(__FILE__);
  Serial.print("Library version: \t");
  Serial.println(SHT31_LIB_VERSION);

  // run any initializers
  sensorController.Initialize();

  // print starting status to the console
  Serial.print(sensorController.Sensor().readStatus(), HEX);
  Serial.println();
}

void loop() {
  // execute the behavior loops
  settingsController.LoopHandler();
  sensorController.LoopHandler();
  hvacController.LoopHandler(sensorController, settingsController);

  // write status on a debounced interval
  writeDebouncer.Bounce(statusWriter);
}