#include "ThermostatModes.h"
#include "Debouncer.h"
#include "SettingsController.h"

float SettingsController::SetHeatTempC() { return _setHeatTempC; }

float SettingsController::SetCoolTempC() { return _setCoolTempC; }

ThermostatTemperatureMode SettingsController::CurrentTempMode() { return _tempMode; }

ThermostatHvacMode SettingsController::CurrentHeatMode() { return _heatMode; }

const char* SettingsController::GetHeatModeString() {
  switch (_heatMode) {
    case Off: return "Off";
    case Heat: return "Heat";
    case Cool: return "Cool";
    default: return "Unknown State";
  }
}

SettingsController::SettingsController(unsigned long incrementBounceMs, unsigned long decrementBounceMs) 
: _incrementBouncer(Debouncer(incrementBounceMs)), _decrementBouncer(Debouncer(decrementBounceMs)) { }

void SettingsController::IncrementSetTempC() {
  auto wrapper = [this]() { _incrementSetTempC(); };
  _incrementBouncer.Bounce(wrapper);
}

void SettingsController::DecrementSetTempC() {
  auto wrapper = [this]() { _decrementSetTempC(); };
  _decrementBouncer.Bounce(wrapper);
}

void SettingsController::ToggleHeatMode() {
  auto wrapper = [this]() { _heatModeToggle(); };
  _setHeatModeBouncer.PermaBounce(wrapper);
}

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