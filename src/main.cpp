#include <Arduino.h>
#include "Wire.h"
#include "SHT31.h"

#include "StableDebouncer.h"
#include "SettingsController.h"
#include "SensorController.h"
#include "HvacController.h"

/* **************************
 * Settings
 */

#ifdef ESP32_S2_DEV
#define PIN_I2C_SCL 18
#define PIN_I2C_SDA 21

#define PIN_BUTTON_UP 39
#define PIN_BUTTON_DOWN 40
#define PIN_TEMP_MODE_TOGGLE 37
#define PIN_HEAT_MODE_TOGGLE 38

// outputs
#define PIN_LED_HEAT 4
#define PIN_LED_COOL 5
#define PIN_LED_FAN 6

#elif SEEED
#define PIN_I2C_SCL 5
#define PIN_I2C_SDA 4

#define PIN_BUTTON_UP 10
#define PIN_BUTTON_DOWN 9
#define PIN_TEMP_MODE_TOGGLE 8
#define PIN_HEAT_MODE_TOGGLE 7

// outputs
#define PIN_LED_HEAT 0
#define PIN_LED_COOL 1
#define PIN_LED_FAN 2

#else
#define PIN_BUTTON_UP 21
#define PIN_BUTTON_DOWN 20
#define PIN_TEMP_MODE_TOGGLE 19
#define PIN_HEAT_MODE_TOGGLE 18

// outputs
#define PIN_LED_HEAT 4
#define PIN_LED_COOL 5
#define PIN_LED_FAN 6
#endif

/// @brief The increment of an up/down button press in celcius mode
const float tempIncrementC = 0.5;

/// @brief The default temperature setting for heating in celcius mode
const float defaultHeatTempC = 21.0;

/// @brief the default temperature setting for cooling in celcius mode
const float defaultCoolTempC = 21.0;

/// @brief the amount to over cool or over heat in celcius mode, helps to prevent too many on/off events
const float hvacOnBufferC = 0.5;

/// @brief The time in milliseconds to wait between HVAC relay state changes, do not set this too low, or you could damage the equipment
const unsigned long hvacChangeDebounceMs = 5000;  // 5 seconds

/// @brief The time in milliseconds to wait between writing status information to the serial console
const unsigned long writeDebounceMs = 1000;  // 1 second

/// @brief The time in milliseconds to execute the button action on a continuous press
const unsigned long buttonDebounceMs = 1000;  // 1 second

/// @brief The time in milliseconds between reads of the temperature sensor
const unsigned long sensorReadBounceMs = 500;  // .5 seconds

/* *************************************
 * End settings
 */

// controllers
SettingsController settingsController = SettingsController(
        StableDebouncer(buttonDebounceMs), StableDebouncer(buttonDebounceMs),
        PinController(PIN_BUTTON_UP, INPUT),PinController(PIN_BUTTON_DOWN, INPUT),
        PinController(PIN_HEAT_MODE_TOGGLE, INPUT));
SensorController sensorController = SensorController(sensorReadBounceMs);
HvacController hvacController = HvacController(hvacChangeDebounceMs, PIN_LED_COOL, PIN_LED_HEAT, PIN_LED_FAN);

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

#if defined(PIN_I2C_SCL) && defined(PIN_I2C_SDA)
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 100000);
#else
  Wire.begin()
#endif

  // run any initializers
  sensorController.Initialize();
  settingsController.Initialize();

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