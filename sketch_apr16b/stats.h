#pragma once

#include <Arduino_GFX_Library.h>
#include <stdint.h>

class PetStats {
 public:
  static constexpr uint8_t STAT_MAX = 5;

  uint8_t hunger() const { return hunger_; }
  uint8_t fun() const { return fun_; }

  void feed();
  void play();
  void tickDecay(unsigned long now);

 private:
  uint8_t hunger_ = 3;
  uint8_t fun_ = 3;
  unsigned long hunger_interval_   = 600000;
  unsigned long fun_interval_      = 300000;
  unsigned long last_hunger_decay_ = 0;
  unsigned long last_fun_decay_    = 0;
};

void drawStatBank(Arduino_GFX* gfx, int16_t cx, int16_t labelY,
                  const char* label, uint8_t level);
void drawStats(Arduino_GFX* gfx, const PetStats& stats);
