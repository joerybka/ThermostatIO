#include "settings.h"
// global configuration

/// @brief The increment of an up/down button press in celcius mode
const float tempIncrementC = 0.5;

/// @brief The default temperature setting for heating in celcius mode
const float defaultHeatTempC = 21.0;

/// @brief the default temperature setting for cooling in celcius mode
const float defaultCoolTempC = 21.0;

/// @brief the amount to over cool or over heat in celcius mode, helps to prevent too many on/off events
const float hvacOnBufferC = 0.5;

/// @brief The time in milliseconds to wait between HVAC relay state changes, do not set this too low, or you could damage the equipment
const unsigned long hvacChangeDebounceMs = 5000;  // 5 seconds

/// @brief The time in milliseconds to wait between writing status information to the serial console
const unsigned long writeDebounceMs = 1000;  // 1 second

/// @brief The time in milliseconds to execute the button action on a continuous press
const unsigned long buttonDebounceMs = 1000;  // 1 second

/// @brief The time in milliseconds between reads of the temperature sensor
const unsigned long sensorReadBounceMs = 500;  // .5 seconds