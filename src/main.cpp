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

// temperature display mode
enum TempMode {
  C = 0,  // celcius
  F = 1  // farenheit
};

enum HeatMode {
  Heat = 0,
  Cool = 1,
  Off = 2
};

/// @brief Class that will only execute the function that is passed to it on the frequncy set by the bounce time.
class Debouncer {
  /// @brief The last bounce time in milliseconds, populated with the millis() function
  unsigned long _lastBounce = 0;

  /// @brief True if this bouncer has invoked any functions and not been reset
  bool _permaBounced = false;

  /// @brief The configured bounce time in this instance
  unsigned long _bounceTimeMs;

  /// @brief Set the last bounce timer to the current millis()
  void _setLastBounce() {
    _lastBounce = millis();
    _permaBounced = true;
  }

  public:
    /// @brief The default value for the bounce time if no configuration is passed
    const unsigned long DefaultBounceTimeMs = 1000;  // 1 second

    /// @brief Instantiate the debouncer with the default bounce time of 1000 milliseconds
    Debouncer() {
      _bounceTimeMs = DefaultBounceTimeMs;
    }

    /// @brief Instantiate the debouncer with a custom bounce timer
    /// @param bounceTimeMs The time in milliseconds to wait between function invocations
    Debouncer(unsigned long bounceTimeMs) {
      _bounceTimeMs = bounceTimeMs;
    }

    /// @brief Pass this method the function that you would like to run, if you need to run a method, wrap it in a lambda
    /// @tparam F The type of the function pointer, be sure this requires no arguments
    /// @param debounceFunction The parameterless function to run if the debouncer is unbounced
    template<typename F>
    void Bounce(F debounceFunction) { 
      if(!IsBounced()) {
        debounceFunction();
        _setLastBounce();
      }
    }

    /// @brief Bounce the running of any functions with this debouncer until it is manually reset
    /// @tparam F The type of the function pointer, be sure this requires no arguments
    /// @param debounceFunction The parameterless function to run if the debouncer has been reset or is in its initial state
    template<typename F>
    void PermaBounce(F debounceFunction) {
      if(!IsPermaBounced()) {
        debounceFunction();
        _setLastBounce();
      }
    }

    /// @brief Accessor to check if this instance will execute functions
    /// @return False if this debouncer would execute the function passed to Bounce, otherwise true
    bool IsBounced() { return (millis() - _lastBounce) < _bounceTimeMs; }

    /// @brief Accessor to check if this instance has executed any functions and not been reset
    /// @return False if no function has been run since instantiation or the last reset, otherwise true
    bool IsPermaBounced() { return _permaBounced; }
    
    /// @brief Reset the last bounce and perma-bounce to their initial values
    void Reset() { 
      _lastBounce = 0; 
      _permaBounced = false;
    }
};

/// @brief Controller to manage tracking settings in the thermostat
class SettingsController {
  Debouncer _incrementBouncer = Debouncer(buttonDebounceMs);
  Debouncer _decrementBouncer = Debouncer(buttonDebounceMs);
  Debouncer _setHeatModeBouncer = Debouncer(buttonDebounceMs);

  /// @brief The temperature target for heating mode when the thermostat is in celcius mode
  float _setHeatTempC = defaultHeatTempC;

  /// @brief The temperature target for cooling mode when the thermostat is in celcius mode
  float _setCoolTempC = defaultCoolTempC;

  /// @brief The amount to increment temperature settings by when in celcius mode
  float _tempIncrementC = tempIncrementC;

  /// @brief The current temperatur display mode
  TempMode _tempMode = C;

  /// @brief The current HVAC mode
  HeatMode _heatMode = Off;

  /// @brief Increment the correct temperature setting in celcius mode
  void _incrementSetTempC() {
    switch(_heatMode) {
      case Heat:
        _setHeatTempC += _tempIncrementC;
        break;
      case Cool: 
        _setCoolTempC += _tempIncrementC;
        break;
      case Off:
      default:
        break;
    }
  }

  /// @brief decrement the correct temperature setting in celcius mode
  void _decrementSetTempC() {
    switch(_heatMode) {
      case Heat:
        _setHeatTempC -= _tempIncrementC;
        break;
      case Cool: 
        _setCoolTempC -= _tempIncrementC;
        break;
      case Off:
      default:
        break;
    }
  }

  /// @brief Toggle the current heat mode: Off -> Heat -> Cool -> Off
  void _heatModeToggle() {
    switch(_heatMode) {
      case Off:
        _heatMode = Heat;
        break;
      case Heat:
        _heatMode = Cool;
        break;
      case Cool:
      default:
        _heatMode = Off;
        break;
    }
  }

  public:
    /// @brief Getter for the current temperature target in heat mode in celcius mode
    /// @return The current temperature target in celcius
    float SetHeatTempC() { return _setHeatTempC; }

    /// @brief Getter for the current temperature target in cooling mode in celcius mode
    /// @return The current temperature target in celcius
    float SetCoolTempC() { return _setCoolTempC; }

    /// @brief Getter for the current temperature mode
    /// @return Farenheit or celcius
    TempMode CurrentTempMode() { return _tempMode; }

    /// @brief Getter for the current heating mode
    /// @return Off, Heat, or Cool
    HeatMode CurrentHeatMode() { return _heatMode; }

    /// @brief Accessor for a string representation of the current heat mode
    /// @return The string value of the heat mode
    const char* GetHeatModeString() {
      switch (_heatMode) {
        case Off: return "Off";
        case Heat: return "Heat";
        case Cool: return "Cool";
        default: return "Unknown State";
      }
    }

    /// @brief Increment the set temperature of the current HVAC mode
    void IncrementSetTempC() {
      auto wrapper = [this]() { _incrementSetTempC(); };
      _incrementBouncer.Bounce(wrapper);
    }

    /// @brief Decrement the set temperature of the current HVAC mode
    void DecrementSetTempC() {
      auto wrapper = [this]() { _decrementSetTempC(); };
      _decrementBouncer.Bounce(wrapper);
    }

    /// @brief Toggle between heat modes: Off -> Heat -> Cool -> Off
    void ToggleHeatMode() {
      auto wrapper = [this]() { _heatModeToggle(); };
      _setHeatModeBouncer.PermaBounce(wrapper);
    }

    /// @brief Method to call to execute looping behavior
    void LoopHandler() {
      if(digitalRead(PIN_BUTTON_UP)){
        IncrementSetTempC();
      } 
      else {
        _incrementBouncer.Reset();
      }
      
      if(digitalRead(PIN_BUTTON_DOWN)) {
        DecrementSetTempC();
      }
      else {
        _decrementBouncer.Reset();
      }

      if(digitalRead(PIN_HEAT_MODE_TOGGLE)) {
        ToggleHeatMode();
      }
      else {
        _setHeatModeBouncer.Reset();
      }
    }
};

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
    float CurrentHumidityRel() { return _currentHumdityRel; }    SHT31 & Sensor() { return _sensor; }

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