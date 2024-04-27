#include "HvacController.h"

HvacController::HvacController(unsigned long hvacChangeDebounceMs, int coolPin, int heatPin, int fanPin)
  : _hvacChangeDebouncer(StableDebouncer(hvacChangeDebounceMs)) {
  _pinCool = coolPin;
  _pinHeat = heatPin;
  _pinFan = fanPin;
}

void HvacController::LoopHandler(SensorController & sensorController, SettingsController & settingsController) {
  auto wrapper = [this, &sensorController, &settingsController]() { _setHvacStates(sensorController, settingsController); };
  _hvacChangeDebouncer.Execute(wrapper);
}