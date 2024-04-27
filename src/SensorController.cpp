#include "Wire.h"
#include "StableDebouncer.h"
#include "SensorController.h"

float SensorController::CurrentTempC() const { return _currentTempC; }

float SensorController::CurrentHumidityRel() const { return _currentHumdityRel; }

SHT31 & SensorController::Sensor() { return _sensor; }

SensorController::SensorController(unsigned long sensorReadBounceMs)
  : _readSensorDebouncer(StableDebouncer(sensorReadBounceMs)), _currentTempC(0.0), _currentHumdityRel(0.0) { }

void SensorController::Initialize() {
    _sensor.begin();
}
    
void SensorController::LoopHandler() {
  auto wrapper = [this]() { _readSensor(); };
  _readSensorDebouncer.Execute(wrapper);
}