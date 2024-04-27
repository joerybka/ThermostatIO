//
// Created by joerr on 20-Apr-24.
//

#include <Arduino.h>
#include "PinController.h"

PinController::PinController(uint8_t pin, uint8_t mode) : _pin(pin), _mode(mode) {
  // internals of Arduino default to write if the mode is bad
  if (_mode == INPUT || _mode == INPUT_PULLUP) _ioMode = In;
  else _ioMode = Out;
}

void PinController::Initialize() {
  pinMode(_pin, _mode);
  SetPinOff();  // ensure predictable start values
}

bool PinController::Invert() {
  _inverted = !_inverted;
  return _inverted;
}

bool PinController::IsOn() {
  return _ioMode == In ? digitalRead(_pin) == _onValue() : _setOn;
}

bool PinController::IsOff() {
  return _ioMode == In ? digitalRead(_pin) == _offValue() : !_setOn;
}

void PinController::SetPinOn() {
  if (_ioMode == In) return;

  _setOn = true;
  digitalWrite(_pin, _onValue());
}

void PinController::SetPinOff() {
  if (_ioMode == In) return;

  _setOn = false;
  digitalWrite(_pin, _offValue());
}