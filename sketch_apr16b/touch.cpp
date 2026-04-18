#include "touch.h"

#include <Arduino.h>

bool TouchInput::begin(TwoWire* wire, uint8_t addr) {
  if (focal_.begin(0, wire, addr)) {
    Serial.println("Focal Touchscreen found");
    ok_ = true;
    is_focal_ = true;
    return true;
  }
  if (cst_.begin(wire, addr)) {
    Serial.println("CST826 Touchscreen found");
    ok_ = true;
    is_focal_ = false;
    return true;
  }
  Serial.print("No Touchscreen found at address 0x");
  Serial.println(addr, HEX);
  ok_ = false;
  return false;
}

bool TouchInput::getPoint(int16_t& x, int16_t& y) {
  if (!ok_) return false;
  if (is_focal_) {
    if (!focal_.touched()) return false;
    TS_Point p = focal_.getPoint(0);
    x = p.x; y = p.y;
  } else {
    if (!cst_.touched()) return false;
    CST_TS_Point p = cst_.getPoint(0);
    x = p.x; y = p.y;
  }
  return true;
}
