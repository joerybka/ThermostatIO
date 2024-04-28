//
// Created by joerr on 27-Apr-24.
//
#include <Arduino.h>
#include "Adafruit_SSD1306.h"

#include "StableDebouncer.h"

#ifndef THERMOSTATIO_DISPLAY_H
#define THERMOSTATIO_DISPLAY_H

static const unsigned char PROGMEM starBmp[] = {
    0b00000000, 0b11000000,
    0b00000001, 0b11000000,
    0b00000001, 0b11000000,
    0b00000011, 0b11100000,
    0b11110011, 0b11100000,
    0b11111110, 0b11111000,
    0b01111110, 0b11111111,
    0b00110011, 0b10011111,
    0b00011111, 0b11111100,
    0b00001101, 0b01110000,
    0b00011011, 0b10100000,
    0b00111111, 0b11100000,
    0b00111111, 0b11110000,
    0b01111100, 0b11110000,
    0b01110000, 0b01110000,
    0b00000000, 0b00110000
};

static const unsigned char PROGMEM squareBmp[] = {
    0b00000000, 0b00000000,
    0b01111111, 0b11111110,
    0b01000000, 0b00000010,
    0b01000000, 0b00000010,
    0b01000000, 0b00000010,
    0b01000000, 0b00000010,
    0b01000000, 0b00000010,
    0b01000000, 0b00000010,
    0b01000000, 0b00000010,
    0b01000000, 0b00000010,
    0b01000000, 0b00000010,
    0b01000000, 0b00000010,
    0b01000000, 0b00000010,
    0b01000000, 0b00000010,
    0b01111111, 0b11111110,
    0b00000000, 0b00000000,
};

#define X_POSITION 0
#define Y_POSITION 1
#define FALL_SPEED 2

class StarfallDriver {
private:
  Adafruit_SSD1306 *_display;
  StableDebouncer _redrawDebouncer;

  static const int8_t _starWidth = 16;
  static const int8_t _starHeight = 16;
  static const int8_t _numStars = 12;

  int8_t _positions[_numStars][3] = {{0}};

  void _drawAnimationFrame() {
    _display->clearDisplay();

    int8_t i;
    for(i = 0; i < _numStars; i++) {
      _display->drawBitmap(_positions[i][X_POSITION], _positions[i][Y_POSITION], starBmp, _starWidth, _starHeight, SSD1306_WHITE);

      // update positions for next iteration
      _positions[i][Y_POSITION] += _positions[i][FALL_SPEED];

      if(_positions[i][Y_POSITION] >= _display->height())
        _resetStarPosition(i);
    }

    _display->display();
  }

  void _resetStarPosition(int8_t positionIndex) {
    _positions[positionIndex][X_POSITION] = random(1 - _starWidth, _display->width());
    _positions[positionIndex][Y_POSITION] = -_starHeight;
    _positions[positionIndex][FALL_SPEED] = random(1, 6);
  }

public:
  StarfallDriver(Adafruit_SSD1306 *display, unsigned long millisecondsPerFrame)
    : _display(display), _redrawDebouncer(millisecondsPerFrame) {

  }

  void Initialize() {
    int8_t i;

    for(i = 0; i < _numStars; i++)
      _resetStarPosition(i);
  }

  void LoopHandler() {
    auto wrapper = [this]() { _drawAnimationFrame(); };
    _redrawDebouncer.Execute(wrapper);
  }
};

#endif //THERMOSTATIO_DISPLAY_H
