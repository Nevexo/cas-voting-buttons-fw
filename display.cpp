#include "display.hpp"
#include "string"

OLEDDisplay::OLEDDisplay(i2c_inst_t *i2c, uint sda_pin, uint scl_pin, uint8_t address)
    : i2c_(i2c), address_(address), display_(nullptr)
{
}

// Startup the display
bool OLEDDisplay::init()
{
  this->display_ = new pico_ssd1306::SSD1306(this->i2c_, this->address_, pico_ssd1306::Size::W128xH32);
  this->display_->setOrientation(0);
  this->display_->turnOn();
  pico_ssd1306::drawText(this->display_, font_8x8, "READY", 0, 0);
  this->display_->sendBuffer();

  return true;
}

void OLEDDisplay::deinit()
{
  this->display_->turnOff();
}

void OLEDDisplay::updateDisplay(float temperature)
{
  float rounded = (int)std::roundf(temperature * 10.0f) / 10.0f;

  // Clear the section where the text was
  pico_ssd1306::fillRect(this->display_, 46, 22, 128, 64, pico_ssd1306::WriteMode::SUBTRACT);

  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%.1f", rounded);

  pico_ssd1306::drawText(display_, font_16x32, buffer, 46, 22);

  this->display_->sendBuffer();
}