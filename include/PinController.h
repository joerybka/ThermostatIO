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
    uint8_t _pin;
    uint8_t _mode;
    bool _inverted;

    IoMode _ioMode;

    bool _setOn = false;

    uint8_t _onValue() {
        return _inverted ? LOW : HIGH;
    }

    uint8_t _offValue() {
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
     * Initialize the pin controller, this is a light wrapper around the digital functions for arduino.
     * @param pin The arduino pin number to control
     * @param mode The pin mode
     * @param inverted If inverted, all logic will be reversed, up will become down, left will become right, but most importantly, pin outputs are reversed
     */
    PinController(uint8_t pin, uint8_t mode, bool inverted);

    /**
     * You must call initialize to run the pin setup.  Output pins will be set to this controller's Off value.
     */
     void Initialize();

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
