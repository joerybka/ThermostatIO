//
// Created by joerr on 20-Apr-24.
//

#include <Arduino.h>
#include "PinController.h"

PinController::PinController(uint8_t pin, uint8_t mode) : PinController(pin, mode, false) { }

PinController::PinController(uint8_t pin, uint8_t mode, bool inverted) : _pin(pin), _mode(mode), _inverted(inverted) {
    // internals of Arduino default to write if the mode is bad
    if(_mode == INPUT || _mode == INPUT_PULLUP) _ioMode = In;
    else _ioMode = Out;
}

void PinController::Initialize() {
    pinMode(_pin, _mode);
    if (_ioMode == Out) digitalWrite(_pin, _offValue());
}

bool PinController::IsOn() {
    return _ioMode == In ? digitalRead(_pin) == _onValue() : _setOn;
}

bool PinController::IsOff() {
    return _ioMode == In ? digitalRead(_pin) == _offValue() : !_setOn;
}

void PinController::SetPinOn() {
    if(_ioMode == In) return;

    digitalWrite(_pin, _onValue());
}

void PinController::SetPinOff() {
    if(_ioMode == In) return;

    digitalWrite(_pin, _offValue());
}