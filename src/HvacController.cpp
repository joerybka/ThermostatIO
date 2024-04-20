#include "HvacController.h"

HvacController::HvacController(unsigned long hvacChangeDebounceMs) : _hvacChangeDebouncer(Debouncer(hvacChangeDebounceMs)) { }

void HvacController::LoopHandler(SensorController & sensorController, SettingsController & settingsController) {
  auto wrapper = [this, &sensorController, &settingsController]() { _setHvacStates(sensorController, settingsController); };
  _hvacChangeDebouncer.Bounce(wrapper);
}