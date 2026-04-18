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
    1 /* vsync_polarity */, 16 /* vsync_front_porch */, 2 /* vsync_pulse_width */, 18 /* vsync_back_porch */,
    0 /* pclk_active_neg */, GFX_NOT_DEFINED /* prefer_speed */, false /* useBigEndian */,
    0 /* de_idle_high */, 0 /* pclk_idle_high */, 480 * 10 /* bounce_buffer_size_px */
    );

Arduino_RGB_Display *display = new Arduino_RGB_Display(
// 2.1" 480x480 round display
    480 /* width */, 480 /* height */, rgbpanel, 0 /* rotation */, false /* auto_flush */,
    expander, GFX_NOT_DEFINED /* RST */, TL021WVC02_init_operations, sizeof(TL021WVC02_init_operations));

Arduino_Canvas *gfx = new Arduino_Canvas(480 /* width */, 480 /* height */, display);;

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
const unsigned long FRAME_INTERVAL_MS = 500;

unsigned long lastLoopTime = 0;
float deltaTime = 0.0f;

enum ButtonID { BTN_FEED, BTN_PLAY, BTN_SLEEP, BTN_COUNT };

struct Button { int16_t x, y, w, h; const char *label; };

const Button buttons[BTN_COUNT] = {
  { 110, 390, 80, 40, "Feed"  },
  { 200, 390, 80, 40, "Play"  },
  { 290, 390, 80, 40, "Sleep" },
};

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

void drawButtons() {
  for (int i = 0; i < BTN_COUNT; i++) {
    const Button &b = buttons[i];
    gfx->fillRoundRect(b.x, b.y, b.w, b.h, 8, 0x4A69);
    gfx->drawRoundRect(b.x, b.y, b.w, b.h, 8, RGB565_WHITE);
    gfx->setTextColor(RGB565_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(b.x + 8, b.y + 12);
    gfx->print(b.label);
  }
}

void render() {
  displayFrame(currentFrame);
  drawButtons();
  gfx->flush();
  display->flush();
}

bool getTouchPoint(int16_t &x, int16_t &y) {
  if (!touchOK) return false;
  if (isFocalTouch) {
    if (!focal_ctp.touched()) return false;
    TS_Point p = focal_ctp.getPoint(0);
    x = p.x; y = p.y;
  } else {
    if (!cst_ctp.touched()) return false;
    CST_TS_Point p = cst_ctp.getPoint(0);
    x = p.x; y = p.y;
  }
  return true;
}

void onButtonPressed(ButtonID id) {
  switch (id) {
    case BTN_FEED:  Serial.println("Feed");  break;
    case BTN_PLAY:  Serial.println("Play");  break;
    case BTN_SLEEP: Serial.println("Sleep"); break;
  }
}

void pollButtons() {
  static bool wasTouched = false;
  int16_t tx, ty;
  bool touched = getTouchPoint(tx, ty);
  if (touched && !wasTouched) {
    for (int i = 0; i < BTN_COUNT; i++) {
      const Button &b = buttons[i];
      if (tx >= b.x && tx < b.x + b.w && ty >= b.y && ty < b.y + b.h) {
        onButtonPressed((ButtonID)i);
        break;
      }
    }
  }
  wasTouched = touched;
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

  lastLoopTime = millis();
  render();
}

void loop()
{
  unsigned long now = millis();
  deltaTime = (now - lastLoopTime) / 1000.0f;
  lastLoopTime = now;

  pollButtons();
  
  if (now - lastFrameTime >= FRAME_INTERVAL_MS) {
    currentFrame = (currentFrame + 1) % 2;
    lastFrameTime = now;
    render();
  }


  if (!expander->digitalRead(PCA_BUTTON_DOWN)) {
    expander->digitalWrite(PCA_TFT_BACKLIGHT, LOW);
  }
  if (!expander->digitalRead(PCA_BUTTON_UP)) {
    expander->digitalWrite(PCA_TFT_BACKLIGHT, HIGH);
  }
}
