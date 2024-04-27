//
// Created by joerr on 20-Apr-24.
//
#include <Arduino.h>

#ifndef THERMOSTATIO_PINCONTROLLER_H
#define THERMOSTATIO_PINCONTROLLER_H

enum IoMode {
  In = 0,
  Out = 1,
};

class PinController {
private:
  /// The pin number that this controller represents
  uint8_t _pin;

  /// The mode of the pin to set during initialization
  uint8_t _mode;

  /// Whether the logic of this pin is inverted, set this if you want "on" to represent LOW and "off" to represent HIGH
  bool _inverted = false;

  /// A higher-level in/out enum so that we can not care about comparing with mode
  IoMode _ioMode;

  /// If this is an "Out" controller, this will hold the last value we wrote to the pin
  bool _setOn = false;

  /// The value to use in this controller to represent "On"
  uint8_t _onValue() const {
    return _inverted ? LOW : HIGH;
  }

  /// The value to use in this controller to represent "Off"
  uint8_t _offValue() const {
    return _inverted ? HIGH : LOW;
  }

public:
  /**
   * Initialize the pin controller, this is a light wrapper around the digital functions for arduino.
   * @param pin The arduino pin number to control
   * @param mode The pin mode
   */
  PinController(uint8_t pin, uint8_t mode);

  /**
   * You must call initialize to run the pin setup.  Output pins will be set to this controller's Off value.
   */
  void Initialize();

  /**
   * Flip the current inversion setting
   * @return The new inversion setting
   */
  bool Invert();

  /**
   * Checks if the pin set to high in write mode or if it is high in read mode
   * @return True if the pin reads on, or if the pin state is set to high in write mode
   */
  bool IsOn();

  /**
   * Checks if the pin is set to low in write mode or if it is low in read mode
   * @return True if the pin reads off, or if the pin state is set to low in write mode
   */
  bool IsOff();

  /**
   * Sets the pin to high in write mode, otherwise no action
   */
  void SetPinOn();

  /**
   * Sets the pin to low in write mode, otherwise no action
   */
  void SetPinOff();
};

#endif //THERMOSTATIO_PINCONTROLLER_H
