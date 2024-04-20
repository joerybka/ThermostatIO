#ifndef SETTINGS_H
#define SETTINGS_H

#define SHT31_ADDRESS 0x44

// inputs
#define PIN_BUTTON_UP 21
#define PIN_BUTTON_DOWN 20
#define PIN_TEMP_MODE_TOGGLE 19
#define PIN_HEAT_MODE_TOGGLE 18

// outputs
#define PIN_LED_HEAT 4
#define PIN_LED_COOL 5
#define PIN_LED_FAN 6

/// @brief The increment of an up/down button press in celcius mode
extern const float tempIncrementC;

/// @brief The default temperature setting for heating in celcius mode
extern const float defaultHeatTempC;

/// @brief the default temperature setting for cooling in celcius mode
extern const float defaultCoolTempC;

/// @brief the amount to over cool or over heat in celcius mode, helps to prevent too many on/off events
extern const float hvacOnBufferC;

/// @brief The time in milliseconds to wait between HVAC relay state changes, do not set this too low, or you could damage the equipment
extern const unsigned long hvacChangeDebounceMs; 

/// @brief The time in milliseconds to wait between writing status information to the serial console
extern const unsigned long writeDebounceMs; 

/// @brief The time in milliseconds to execute the button action on a continuous press
extern const unsigned long buttonDebounceMs; 

/// @brief The time in milliseconds between reads of the temperature sensor
extern const unsigned long sensorReadBounceMs; 

#endif