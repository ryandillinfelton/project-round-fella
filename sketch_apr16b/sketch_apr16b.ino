#include <Arduino_GFX_Library.h>
#include <LittleFS.h>
#include <Wire.h>

#include "animator.h"
#include "stats.h"
#include "touch.h"
#include "ui.h"

Arduino_XCA9554SWSPI *expander = new Arduino_XCA9554SWSPI(
    PCA_TFT_RESET, PCA_TFT_CS, PCA_TFT_SCK, PCA_TFT_MOSI,
    &Wire, 0x3F);

Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    TFT_DE, TFT_VSYNC, TFT_HSYNC, TFT_PCLK,
    TFT_R1, TFT_R2, TFT_R3, TFT_R4, TFT_R5,
    TFT_G0, TFT_G1, TFT_G2, TFT_G3, TFT_G4, TFT_G5,
    TFT_B1, TFT_B2, TFT_B3, TFT_B4, TFT_B5,
    1 /* hsync_polarity */, 50 /* hsync_front_porch */, 2 /* hsync_pulse_width */, 44 /* hsync_back_porch */,
    1 /* vsync_polarity */, 16 /* vsync_front_porch */, 2 /* vsync_pulse_width */, 18 /* vsync_back_porch */,
    0 /* pclk_active_neg */, GFX_NOT_DEFINED /* prefer_speed */, false /* useBigEndian */,
    0 /* de_idle_high */, 0 /* pclk_idle_high */, 480 * 10 /* bounce_buffer_size_px */
    );

Arduino_RGB_Display *display = new Arduino_RGB_Display(
    480 /* width */, 480 /* height */, rgbpanel, 0 /* rotation */, false /* auto_flush */,
    expander, GFX_NOT_DEFINED /* RST */, TL021WVC02_init_operations, sizeof(TL021WVC02_init_operations));

Arduino_Canvas *gfx = new Arduino_Canvas(480, 480, display);

#define I2C_TOUCH_ADDR 0x15

Animator  idleAnimator;
Animator  sadAnimator;
PetStats  stats;
TouchInput touch;

void render(bool sad) {
  if (sad) sadAnimator.render(gfx, 0, 0);
  else     idleAnimator.render(gfx, 0, 0);
  drawButtons(gfx);
  drawStats(gfx, stats);
  gfx->flush();
  display->flush();
}

void setup() {
  Serial.begin(115200);

#ifdef GFX_EXTRA_PRE_INIT
  GFX_EXTRA_PRE_INIT();
#endif

  Wire.setClock(1000000);
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  }

  expander->pinMode(PCA_TFT_BACKLIGHT, OUTPUT);
  expander->digitalWrite(PCA_TFT_BACKLIGHT, HIGH);

  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed!");
  }

  static const char* kIdleFrames[] = { "/idleFrame1.bin", "/idleFrame2.bin" };
  idleAnimator.load(kIdleFrames, 2);

  static const char* kSadFrames[] = { "/SadFrame1.bin", "/SadFrame2.bin" };
  sadAnimator.load(kSadFrames, 2);

  touch.begin(&Wire, I2C_TOUCH_ADDR);

  render(false);
}

void loop() {
  unsigned long now = millis();

  static bool wasTouched = false;
  int16_t tx, ty;
  bool touched = touch.getPoint(tx, ty);
  if (touched && !wasTouched) {
    Serial.printf("Touch at %d,%d\n", tx, ty);
    ButtonID id = hitTest(tx, ty);
    if (id < BTN_COUNT) {
      Serial.printf("  -> hit button %d (%s)\n", id, buttons[id].label);
      switch (id) {
        case BTN_FEED: stats.feed(); break;
        case BTN_PLAY: stats.play(); break;
        default: break;
      }
    } else {
      Serial.println("  -> no button hit");
    }
  }
  wasTouched = touched;

  stats.tickDecay(now);

  bool isSad = (stats.hunger() == 0 || stats.fun() == 0);
  static bool wasSad = false;
  bool ticked = isSad ? sadAnimator.tick(now) : idleAnimator.tick(now);
  if (ticked || isSad != wasSad) render(isSad);
  wasSad = isSad;

  if (!expander->digitalRead(PCA_BUTTON_DOWN)) {
    expander->digitalWrite(PCA_TFT_BACKLIGHT, LOW);
  }
  if (!expander->digitalRead(PCA_BUTTON_UP)) {
    expander->digitalWrite(PCA_TFT_BACKLIGHT, HIGH);
  }
}
