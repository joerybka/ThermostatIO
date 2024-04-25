#include "ThermostatModes.h"
#include "Debouncer.h"
#include "SettingsController.h"
#include "SensorController.h"

#ifndef HVAC_CONTROLLER_H
#define HVAC_CONTROLLER_H

/// @brief Controller for the HVAC relays
class HvacController {
  /// @brief HVAC change debouncer to prevent damage to HVAC equipment
  Debouncer _hvacChangeDebouncer;

  /// @brief Flag for if the cooling system is on
  bool _isCoolOn = false;

  /// @brief Flag for if the heating system is on
  bool _isHeatOn = false;

  /// @brief Flag for if the fan is on
  bool _isFanOn = false;

  int _pinCool;
  int _pinHeat;
  int _pinFan;

  const float _hvacOnBufferC = 0.5;

  /// @brief private setter for the cooling system relay
  void _setCoolRelay() {
    digitalWrite(_pinCool, _isCoolOn ? HIGH : LOW);
  }

  /// @brief Private setter for the heating system relay
  void _setHeatRelay() {
    digitalWrite(_pinHeat, _isHeatOn ? HIGH : LOW);
  }

  /// @brief Private setter for the fan
  void _setFanRelay() {
    digitalWrite(_pinFan, _isFanOn ? HIGH : LOW);
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

    if(sensorController.CurrentTempC() >= (settingsController.SetHeatTempC() + _hvacOnBufferC)) {
      _isHeatOn = false;
      _isFanOn = false;
    }
    else if(sensorController.CurrentTempC() <= (settingsController.SetHeatTempC() - _hvacOnBufferC)) {
      _isHeatOn = true;
      _isFanOn = true;
    }
  }

  /// @brief Private setter for flags in the cooling state
  /// @param sensorController The sensor controller to read from to get current external readings
  /// @param settingsController The settings controller to get current settings from
  void _setHvacCoolStates(SensorController & sensorController, SettingsController & settingsController) {
    _isHeatOn = false;

    if(sensorController.CurrentTempC() <= (settingsController.SetCoolTempC() - _hvacOnBufferC)) {
      _isCoolOn = false;
      _isFanOn = false;
    }
    else if(sensorController.CurrentTempC() >= (settingsController.SetCoolTempC() + _hvacOnBufferC)) {
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
    /// @brief Controller for the HVAC relays
    /// @param hvacChangeBounceMs The number of milliseconds between changes to the HVAC equipment, be careful not to set this too low
    HvacController(unsigned long hvacChangeBounceMs, int coolPin, int heatPin, int fanPin);

    /// @brief Loop handler for HVAC behaviors
    /// @param sensorController The sensor controller to read from to get current external readings
    /// @param settingsController The settings controller to get current settings from
    void LoopHandler(SensorController & sensorController, SettingsController & settingsController);
};

#endif