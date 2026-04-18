#include "ui.h"

const Button buttons[BTN_COUNT] = {
  { 155, 390, 80, 40, "Feed" },
  { 245, 390, 80, 40, "Play" },
};

void drawButtons(Arduino_GFX* gfx) {
  for (int i = 0; i < BTN_COUNT; i++) {
    const Button& b = buttons[i];
    gfx->fillRoundRect(b.x, b.y, b.w, b.h, 8, 0x4A69);
    gfx->drawRoundRect(b.x, b.y, b.w, b.h, 8, RGB565_WHITE);
    gfx->setTextColor(RGB565_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(b.x + 8, b.y + 12);
    gfx->print(b.label);
  }
}

ButtonID hitTest(int16_t x, int16_t y) {
  for (int i = 0; i < BTN_COUNT; i++) {
    const Button& b = buttons[i];
    if (x >= b.x && x < b.x + b.w && y >= b.y && y < b.y + b.h) {
      return (ButtonID)i;
    }
  }
  return BTN_COUNT;
}
