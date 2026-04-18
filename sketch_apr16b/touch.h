#pragma once

#include <Adafruit_CST8XX.h>
#include <Adafruit_FT6206.h>
#include <Wire.h>
#include <stdint.h>

class TouchInput {
 public:
  bool begin(TwoWire* wire, uint8_t addr);
  bool getPoint(int16_t& x, int16_t& y);

 private:
  Adafruit_FT6206 focal_;
  Adafruit_CST8XX cst_;
  bool ok_ = false;
  bool is_focal_ = false;
};
