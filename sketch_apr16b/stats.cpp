#include "stats.h"

#include <Arduino.h>
#include <string.h>

void PetStats::feed() {
  if (hunger_ < STAT_MAX) hunger_++;
}

void PetStats::play() {
  if (fun_ < STAT_MAX) fun_++;
}

void PetStats::tickDecay(unsigned long now) {
  if (now - last_hunger_decay_ >= hunger_interval_) {
    if (hunger_ > 0) hunger_--;
    last_hunger_decay_ = now;
  }
  if (now - last_fun_decay_ >= fun_interval_) {
    if (fun_ > 0) fun_--;
    last_fun_decay_ = now;
  }
}

void drawStatBank(Arduino_GFX* gfx, int16_t cx, int16_t labelY,
                  const char* label, uint8_t level) {
  const uint8_t textSize = 3;
  int16_t labelW = strlen(label) * 6 * textSize;
  gfx->setTextColor(RGB565_BLACK);
  gfx->setTextSize(textSize);
  gfx->setCursor(cx - labelW / 2, labelY);
  gfx->print(label);

  const int16_t r = 11, gap = 6, spacing = 2 * r + gap;
  int16_t startX = cx - 2 * spacing;
  int16_t circleY = labelY + 8 * textSize + r + 6;
  for (int i = 0; i < PetStats::STAT_MAX; i++) {
    int16_t x = startX + i * spacing;
    if (i < level) gfx->fillCircle(x, circleY, r, RGB565_BLACK);
    else           gfx->drawCircle(x, circleY, r, RGB565_BLACK);
  }
}

void drawStats(Arduino_GFX* gfx, const PetStats& stats) {
  drawStatBank(gfx, 170, 40, "Hunger", stats.hunger());
  drawStatBank(gfx, 310, 40, "Fun",    stats.fun());
}
