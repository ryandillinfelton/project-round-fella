#include "animator.h"

#include <Arduino.h>
#include <LittleFS.h>

namespace {
constexpr size_t kFramePixels = 480 * 480;
}

bool Animator::load(const char* const* files, size_t count) {
  if (count > MAX_FRAMES) count = MAX_FRAMES;
  count_ = count;
  for (size_t i = 0; i < count_; i++) {
    buffers_[i] = (uint16_t*)ps_malloc(kFramePixels * sizeof(uint16_t));
    if (!buffers_[i]) {
      Serial.printf("Failed to allocate frame %u\n", (unsigned)i);
      return false;
    }
    File f = LittleFS.open(files[i]);
    if (!f) {
      Serial.printf("Failed to open %s\n", files[i]);
      return false;
    }
    f.read((uint8_t*)buffers_[i], kFramePixels * sizeof(uint16_t));
    f.close();
    Serial.printf("Loaded %s\n", files[i]);
  }
  return true;
}

void Animator::setInterval(unsigned long ms) {
  interval_ = ms;
}

bool Animator::tick(unsigned long now) {
  if (count_ == 0) return false;
  if (now - last_ < interval_) return false;
  current_ = (current_ + 1) % count_;
  last_ = now;
  return true;
}

void Animator::render(Arduino_GFX* gfx, int16_t x, int16_t y) const {
  if (count_ == 0) return;
  uint16_t* buf = buffers_[current_];
  if (buf) gfx->draw16bitRGBBitmap(x, y, buf, 480, 480);
}
