// SPDX-FileCopyrightText: 2023 Limor Fried for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#include <Arduino_GFX_Library.h>
#include <Adafruit_FT6206.h>
#include <Adafruit_CST8XX.h>
#include <LittleFS.h>

Arduino_XCA9554SWSPI *expander = new Arduino_XCA9554SWSPI(
    PCA_TFT_RESET, PCA_TFT_CS, PCA_TFT_SCK, PCA_TFT_MOSI,
    &Wire, 0x3F);

Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    TFT_DE, TFT_VSYNC, TFT_HSYNC, TFT_PCLK,
    TFT_R1, TFT_R2, TFT_R3, TFT_R4, TFT_R5,
    TFT_G0, TFT_G1, TFT_G2, TFT_G3, TFT_G4, TFT_G5,
    TFT_B1, TFT_B2, TFT_B3, TFT_B4, TFT_B5,
    1 /* hsync_polarity */, 50 /* hsync_front_porch */, 2 /* hsync_pulse_width */, 44 /* hsync_back_porch */,
    1 /* vsync_polarity */, 16 /* vsync_front_porch */, 2 /* vsync_pulse_width */, 18 /* vsync_back_porch */
//    ,1, 30000000
    );

Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
// 2.1" 480x480 round display
    480 /* width */, 480 /* height */, rgbpanel, 0 /* rotation */, false /* auto_flush */,
    expander, GFX_NOT_DEFINED /* RST */, TL021WVC02_init_operations, sizeof(TL021WVC02_init_operations));

// The Capacitive touchscreen overlays uses hardware I2C (SCL/SDA)

// Most touchscreens use FocalTouch with I2C Address often but not always 0x48!
// #define I2C_TOUCH_ADDR 0x48

// 2.1" 480x480 round display use CST826 touchscreen with I2C Address at 0x15
#define I2C_TOUCH_ADDR 0x15

Adafruit_FT6206 focal_ctp = Adafruit_FT6206();  // this library also supports FT5336U!
Adafruit_CST8XX cst_ctp = Adafruit_CST8XX();
bool touchOK = false;
bool isFocalTouch = false;

const char *frameFiles[] = { "/Frame1.bin", "/Frame2.bin" };
uint16_t *frameBuffers[2] = { nullptr, nullptr };
uint8_t currentFrame = 0;
unsigned long lastFrameTime = 0;
const unsigned long FRAME_INTERVAL_MS = 200;

#define FRAME_PIXELS (480 * 480)

void loadFrames() {
  for (int i = 0; i < 2; i++) {
    frameBuffers[i] = (uint16_t *)ps_malloc(FRAME_PIXELS * sizeof(uint16_t));
    if (!frameBuffers[i]) {
      Serial.printf("Failed to allocate frame %d\n", i);
      return;
    }
    File f = LittleFS.open(frameFiles[i]);
    if (!f) {
      Serial.printf("Failed to open %s\n", frameFiles[i]);
      return;
    }
    f.read((uint8_t *)frameBuffers[i], FRAME_PIXELS * sizeof(uint16_t));
    f.close();
    Serial.printf("Loaded %s\n", frameFiles[i]);
  }
}

void displayFrame(uint8_t index) {
  if (frameBuffers[index]) {
    gfx->draw16bitRGBBitmap(0, 0, frameBuffers[index], 480, 480);
  }
}

void setup(void)
{
  Serial.begin(115200);

#ifdef GFX_EXTRA_PRE_INIT
  GFX_EXTRA_PRE_INIT();
#endif

  Wire.setClock(1000000);
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  }

  gfx->fillScreen(RGB565_BLACK);

  expander->pinMode(PCA_TFT_BACKLIGHT, OUTPUT);
  expander->digitalWrite(PCA_TFT_BACKLIGHT, HIGH);

  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed!");
  }
  loadFrames();

  if (!focal_ctp.begin(0, &Wire, I2C_TOUCH_ADDR)) {
    if (!cst_ctp.begin(&Wire, I2C_TOUCH_ADDR)) {
      Serial.print("No Touchscreen found at address 0x");
      Serial.println(I2C_TOUCH_ADDR, HEX);
      touchOK = false;
    } else {
      Serial.println("CST826 Touchscreen found");
      touchOK = true;
      isFocalTouch = false;
    }
  } else {
    Serial.println("Focal Touchscreen found");
    touchOK = true;
    isFocalTouch = true;
  }
}

void loop()
{
  if (touchOK) {
    if (isFocalTouch && focal_ctp.touched()) {
      TS_Point p = focal_ctp.getPoint(0);
      gfx->fillRect(p.x, p.y, 5, 5, RGB565_WHITE);
    } else if (!isFocalTouch && cst_ctp.touched()) {
      CST_TS_Point p = cst_ctp.getPoint(0);
      gfx->fillRect(p.x, p.y, 5, 5, RGB565_WHITE);
    }
  }

  unsigned long now = millis();
  if (now - lastFrameTime >= FRAME_INTERVAL_MS) {
    displayFrame(currentFrame);
    currentFrame = (currentFrame + 1) % 2;
    lastFrameTime = now;
  }

  if (!expander->digitalRead(PCA_BUTTON_DOWN)) {
    expander->digitalWrite(PCA_TFT_BACKLIGHT, LOW);
  }
  if (!expander->digitalRead(PCA_BUTTON_UP)) {
    expander->digitalWrite(PCA_TFT_BACKLIGHT, HIGH);
  }
}
