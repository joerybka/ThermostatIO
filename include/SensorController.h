#include "Debouncer.h"
#include "SHT31.h"

#ifndef SENSOR_CONTROLLER_H
#define SENSOR_CONTROLLER_H

/// @brief Controller for the temperature sensor
class SensorController {
  private:
    /// @brief Debouncer for reading the temperature sensor
    Debouncer _readSensorDebouncer;

    /// @brief The sensor object
    SHT31 _sensor;

    /// @brief The last read temperature value in celcius
    float _currentTempC;

    /// @brief The last read humidity in relative percent
    float _currentHumdityRel;
    
    /// @brief Execute a read of the sensor
    void _readSensor() {
      _sensor.read();

      _currentTempC = _sensor.getTemperature();
      _currentHumdityRel = _sensor.getHumidity();
    }

  public:
    /// @brief Getter of the current temperature
    /// @return The last read temperature in celcius
    float CurrentTempC() const;

    /// @brief Getter of the current relative humidity
    /// @return The last read humidity in relative percent
    float CurrentHumidityRel() const;

    /// @brief The current sensor object being managed by this object
    /// @return The SHT31 sensor
    SHT31 & Sensor();

    /// @brief Constructor for the SensorController
    /// @param sensorReadBounceMs The number of milliseconds to wait between reads of the sensor
    SensorController(unsigned long sensorReadBounceMs);

    /// Initializer with specified settings
    void Initialize(int sdaPin, int sclPin, uint32_t clock);

    /// @brief Handler for executing looping behavior
    void LoopHandler();
};

#endif