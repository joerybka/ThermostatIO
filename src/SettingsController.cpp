#include "Modes.h"
#include "SettingsController.h"

float SettingsController::SetHeatTempC() { return _setHeatTempC; }

/// @brief Getter for the current temperature target in cooling mode in celcius mode
/// @return The current temperature target in celcius
float SettingsController::SetCoolTempC() { return _setCoolTempC; }

/// @brief Getter for the current temperature mode
/// @return Farenheit or celcius
ThermostatTemperatureMode SettingsController::CurrentTempMode() { return _tempMode; }

/// @brief Getter for the current heating mode
/// @return Off, Heat, or Cool
ThermostatHvacMode SettingsController::CurrentHeatMode() { return _heatMode; }

/// @brief Accessor for a string representation of the current heat mode
/// @return The string value of the heat mode
const char* SettingsController::GetHeatModeString() {
  switch (_heatMode) {
    case Off: return "Off";
    case Heat: return "Heat";
    case Cool: return "Cool";
    default: return "Unknown State";
  }
}

/// @brief Increment the set temperature of the current HVAC mode
void SettingsController::IncrementSetTempC() {
  auto wrapper = [this]() { _incrementSetTempC(); };
  _incrementBouncer.Bounce(wrapper);
}

/// @brief Decrement the set temperature of the current HVAC mode
void SettingsController::DecrementSetTempC() {
  auto wrapper = [this]() { _decrementSetTempC(); };
  _decrementBouncer.Bounce(wrapper);
}

/// @brief Toggle between heat modes: Off -> Heat -> Cool -> Off
void SettingsController::ToggleHeatMode() {
  auto wrapper = [this]() { _heatModeToggle(); };
  _setHeatModeBouncer.PermaBounce(wrapper);
}

/// @brief Method to call to execute looping behavior
void SettingsController::LoopHandler() {
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