#include "Debouncer.h"
#include "SensorController.h"

float SensorController::CurrentTempC() { return _currentTempC; }

float SensorController::CurrentHumidityRel() { return _currentHumdityRel; }  

SHT31 & SensorController::Sensor() { return _sensor; }

SensorController::SensorController(unsigned long sensorReadBounceMs) : _readSensorDebouncer(Debouncer(sensorReadBounceMs)) { }
    
void SensorController::Initialize() {
  Wire.begin();
  Wire.setClock(100000);
  _sensor.begin();
}
    
void SensorController::LoopHandler() {
  auto wrapper = [this]() { _readSensor(); };
  _readSensorDebouncer.Bounce(wrapper);
}