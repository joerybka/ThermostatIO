#include <Arduino.h>
#include "Wire.h"
#include "SHT31.h"

#include "settings.h"
#include "Debouncer.h"
#include "SettingsController.h"

/// @brief Controller for the temperature sensor
class SensorController {
  /// @brief Debouncer for reading the temperature sensor
  Debouncer _readSensorDebouncer = Debouncer(sensorReadBounceMs);

  /// @brief The sensor object
  SHT31 _sensor;

  /// @brief The last read temperature value in celcius
  float _currentTempC;

  /// @brief The last read humidity in relative percent
  float _currentHumdityRel;
  
  /// @brief Execute a read of the sensor
  void _readSensor() {
    _sensor.read();

    _currentTempC = _sensor.getTemperature();
    _currentHumdityRel = _sensor.getHumidity();
  }

  public:
    /// @brief Getter of the current temperature
    /// @return The last read temperature in celcius
    float CurrentTempC() { return _currentTempC; }

    /// @brief Getter of the current relative humidity
    /// @return The last read humidity in relative percent
    float CurrentHumidityRel() { return _currentHumdityRel; }  
    SHT31 & Sensor() { return _sensor; }

    /// @brief Initializer for the sensor hardware
    void Initialize() {
      Wire.begin();
      Wire.setClock(100000);
      _sensor.begin();
    }

    /// @brief Handler for executing looping behavior
    void LoopHandler() {
      auto wrapper = [this]() { _readSensor(); };
      _readSensorDebouncer.Bounce(wrapper);
    }
};

/// @brief Controller for the HVAC relays
class HvacController {
  /// @brief HVAC change debouncer to prevent damage to HVAC equipment
  Debouncer _hvacChangeDebouncer = Debouncer(hvacChangeDebounceMs);

  /// @brief Flag for if the cooling system is on
  bool _isCoolOn = false;

  /// @brief Flag for if the heating system is on
  bool _isHeatOn = false;

  /// @brief Flag for if the fan is on
  bool _isFanOn = false;

  /// @brief private setter for the cooling system relay
  void _setCoolRelay() {
    digitalWrite(PIN_LED_COOL, _isCoolOn ? HIGH : LOW);
  }

  /// @brief Private setter for the heating system relay
  void _setHeatRelay() {
    digitalWrite(PIN_LED_HEAT, _isHeatOn ? HIGH : LOW);
  }

  /// @brief Private setter for the fan
  void _setFanRelay() {
    digitalWrite(PIN_LED_FAN, _isFanOn ? HIGH : LOW);
  }

  /// @brief Private trigger for setting all relays at once
  void _setRelays() {
    _setCoolRelay();
    _setHeatRelay();
    _setFanRelay();
  }

  /// @brief Private setter to turn off all HVAC flags
  void _setHvacOffStates() {
    _isCoolOn = false;
    _isFanOn = false;
    _isHeatOn = false;
  }

  /// @brief Private setter for flags in the heating state
  /// @param sensorController The sensor controller to read from to get current external readings
  /// @param settingsController The settings controller to get current settings from
  void _setHvacHeatStates(SensorController & sensorController, SettingsController & settingsController) {
    _isCoolOn = false;

    if(sensorController.CurrentTempC() >= (settingsController.SetHeatTempC() + hvacOnBufferC)) {
      _isHeatOn = false;
      _isFanOn = false;
    }
    else if(sensorController.CurrentTempC() <= (settingsController.SetHeatTempC() - hvacOnBufferC)) {
      _isHeatOn = true;
      _isFanOn = true;
    }
  }

  /// @brief Private setter for flags in the cooling state
  /// @param sensorController The sensor controller to read from to get current external readings
  /// @param settingsController The settings controller to get current settings from
  void _setHvacCoolStates(SensorController & sensorController, SettingsController & settingsController) {
    _isHeatOn = false;

    if(sensorController.CurrentTempC() <= (settingsController.SetCoolTempC() - hvacOnBufferC)) {
      _isCoolOn = false;
      _isFanOn = false;
    }
    else if(sensorController.CurrentTempC() >= (settingsController.SetCoolTempC() + hvacOnBufferC)) {
      _isCoolOn = true;
      _isFanOn = true;
    }
  }

  /// @brief Dispatcher for the different HVAC states
  /// @param sensorController The sensor controller to read from to get current external readings
  /// @param settingsController The settings controller to get current settings from
  void _setHvacStates(SensorController & sensorController, SettingsController & settingsController) {
    switch(settingsController.CurrentHeatMode()) {
      case Heat:
        _setHvacHeatStates(sensorController, settingsController);
        break;
      case Cool:
        _setHvacCoolStates(sensorController, settingsController);
        break;
      case Off:
      default:
        _setHvacOffStates();
        break;
    }

    _setRelays();
  }

  public:
    /// @brief Loop handler for HVAC behaviors
    /// @param sensorController The sensor controller to read from to get current external readings
    /// @param settingsController The settings controller to get current settings from
    void LoopHandler(SensorController & sensorController, SettingsController & settingsController) {
      auto wrapper = [this, &sensorController, &settingsController]() { _setHvacStates(sensorController, settingsController); };
      _hvacChangeDebouncer.Bounce(wrapper);
    }
};

// controllers
SettingsController settingsController;
SensorController sensorController;
HvacController hvacController;

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