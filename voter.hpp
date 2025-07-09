#pragma once

#include <string>
#include "pico/stdlib.h"

enum class LEDDesiredState
{
  REQUEST_OFF,
  REQUEST_ON,
  REQUEST_FLASH
};

class Voter
{
private:
  std::string initials_;
  uint button_pin_;
  uint led_pin_;
  bool button_state_;
  bool voted_;
  LEDDesiredState led_desired_state_;
  bool led_current_state_;

public:
  Voter(const std::string &initials, uint button_pin, uint led_pin);

  bool has_voted() const;
  void vote();
  void reset();

  bool get_led_state() const;
  LEDDesiredState get_desired_led_state() const;
  void set_led_state(bool on);

  uint get_led_pin() const;
  uint get_button_pin() const;

  bool get_button_state() const;
  void set_button_state(bool pressed);

  std::string get_initials() const;
};