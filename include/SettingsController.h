#include "ThermostatModes.h"
#include "Debouncer.h"
#include "StableDebouncer.h"
#include "PinController.h"

#ifndef SETTINGSCONTROLLER_H
#define SETTINGSCONTROLLER_H

/// @brief Controller to manage tracking settings in the thermostat
class SettingsController {
  private: 
    StableDebouncer _incrementBouncer;
    StableDebouncer _decrementBouncer;
    StableDebouncer _setHeatModeBouncer = StableDebouncer();
    PinController _upButton;
    PinController _downButton;
    PinController _modeButton;

    /// @brief The temperature target for heating mode when the thermostat is in celcius mode
    float _setHeatTempC = 21.0;

    /// @brief The temperature target for cooling mode when the thermostat is in celcius mode
    float _setCoolTempC = 21.0;

    /// @brief The amount to increment temperature settings by when in celcius mode
    float _tempIncrementC = 0.5;

    /// @brief The current temperatur display mode
    ThermostatTemperatureMode _tempMode = C;

    /// @brief The current HVAC mode
    ThermostatHvacMode _heatMode = Off;

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
    float SetHeatTempC() const;

    /// @brief Getter for the current temperature target in cooling mode in celcius mode
    /// @return The current temperature target in celcius
    float SetCoolTempC() const;

    /// @brief Getter for the current temperature mode
    /// @return Farenheit or celcius
    ThermostatTemperatureMode CurrentTempMode();

    /// @brief Getter for the current heating mode
    /// @return Off, Heat, or Cool
    ThermostatHvacMode CurrentHeatMode();

    /// @brief Accessor for a string representation of the current heat mode
    /// @return The string value of the heat mode
    const char* GetHeatModeString();

    /**
     * Pass in the actual debouncers to be used instead of default bounce delays
     * @param incrementBouncer The debouncer for incrementing settings
     * @param decrementBouncer The debouncer for decrementing settings
     * @param upButtonController The controller for the up button
     * @param downButtonController The controller for the down button
     * @param modeButtonController The controller for the mode button
     */
    SettingsController(StableDebouncer incrementBouncer, StableDebouncer decrementBouncer, PinController upButtonController,
                       PinController downButtonController, PinController modeButtonController);

    /**
     * Initialize the settings of any internal states
     */
    void Initialize();

    /// @brief Increment the set temperature of the current HVAC mode
    void IncrementSetTempC();

    /// @brief Decrement the set temperature of the current HVAC mode
    void DecrementSetTempC();

    /// @brief Toggle between heat modes: Off -> Heat -> Cool -> Off
    void ToggleHeatMode();

    /// @brief Method to call to execute looping behavior
    void LoopHandler();
};

#endif