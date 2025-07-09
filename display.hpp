#pragma once

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "extern/pico-ssd1306/textRenderer/TextRenderer.h"
#include "extern/pico-ssd1306/shapeRenderer/ShapeRenderer.h"

class OLEDDisplay
{
public:
  OLEDDisplay(i2c_inst_t *i2c, uint sda_pin, uint scl_pin, uint8_t address);
  bool init();
  void deinit();
  void updateDisplay(float temperature);

private:
  i2c_inst_t *i2c_;
  uint8_t address_;
  pico_ssd1306::SSD1306 *display_;
};
