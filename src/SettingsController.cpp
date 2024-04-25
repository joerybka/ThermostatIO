#include "ThermostatModes.h"
#include "Debouncer.h"
#include "StableDebouncer.h"
#include "SettingsController.h"

float SettingsController::SetHeatTempC() const { return _setHeatTempC; }

float SettingsController::SetCoolTempC() const { return _setCoolTempC; }

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

SettingsController::SettingsController(Debouncer incrementBouncer, Debouncer decrementBouncer,
                                       PinController upButtonController, PinController downButtonController,
                                       PinController modeButtonController)
 : _incrementBouncer(incrementBouncer), _decrementBouncer(decrementBouncer), _upButton(upButtonController),
   _downButton(downButtonController), _modeButton(modeButtonController) {
    _setHeatModeBouncer.SetStickyBounce(true);
    _setHeatModeBouncer.SetStartBounceDelay(10);
    _setHeatModeBouncer.SetBounceResetCooldown(10);
}

void SettingsController::Initialize() {
    _upButton.Initialize();
    _downButton.Initialize();
    _modeButton.Initialize();
}

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
  _setHeatModeBouncer.Bounce(wrapper);
}

void SettingsController::LoopHandler() {
  if(_upButton.IsOn()){
    IncrementSetTempC();
  } 
  else {
    _incrementBouncer.Reset();
  }
  
  if(_downButton.IsOn()) {
    DecrementSetTempC();
  }
  else {
    _decrementBouncer.Reset();
  }

  if(_modeButton.IsOn()) {
    ToggleHeatMode();
  }
  else {
    _setHeatModeBouncer.Reset();
  }
}
