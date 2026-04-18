#pragma once

#include <Arduino_GFX_Library.h>
#include <stdint.h>

enum ButtonID { BTN_FEED, BTN_PLAY, BTN_COUNT };

struct Button {
  int16_t x, y, w, h;
  const char* label;
};

extern const Button buttons[BTN_COUNT];

void     drawButtons(Arduino_GFX* gfx);
ButtonID hitTest(int16_t x, int16_t y);
