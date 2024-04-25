#include "Debouncer.h"
#include "SensorController.h"

float SensorController::CurrentTempC() const { return _currentTempC; }

float SensorController::CurrentHumidityRel() const { return _currentHumdityRel; }

SHT31 & SensorController::Sensor() { return _sensor; }

SensorController::SensorController(unsigned long sensorReadBounceMs)
  : _readSensorDebouncer(Debouncer(sensorReadBounceMs)), _currentTempC(0.0), _currentHumdityRel(0.0) { }

void SensorController::Initialize(int sdaPin, int sclPin, uint32_t clock) {
    Wire.begin(sdaPin, sclPin, clock);
    _sensor.begin();
}
    
void SensorController::LoopHandler() {
  auto wrapper = [this]() { _readSensor(); };
  _readSensorDebouncer.Bounce(wrapper);
}