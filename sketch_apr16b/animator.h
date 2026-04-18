#pragma once

#include <Arduino_GFX_Library.h>
#include <stddef.h>
#include <stdint.h>

class Animator {
 public:
  static constexpr size_t MAX_FRAMES = 2;

  bool load(const char* const* files, size_t count);
  void setInterval(unsigned long ms);
  bool tick(unsigned long now);
  void render(Arduino_GFX* gfx, int16_t x, int16_t y) const;

 private:
  uint16_t* buffers_[MAX_FRAMES] = {nullptr, nullptr};
  size_t count_ = 0;
  uint8_t current_ = 0;
  unsigned long interval_ = 500;
  unsigned long last_ = 0;
};
