#include "voter.hpp"

Voter::Voter(const std::string &initials, uint button_pin, uint led_pin)
{
  this->initials_ = initials;
  this->button_pin_ = button_pin;
  this->led_pin_ = led_pin;

  this->led_desired_state_ = LEDDesiredState::REQUEST_OFF;
  this->voted_ = false;
  this->led_current_state_ = false;
  this->button_state_ = false;
}

bool Voter::has_voted() const
{
  return this->voted_;
};

void Voter::vote()
{
  this->voted_ = true;
  this->led_desired_state_ = LEDDesiredState::REQUEST_ON;
}

void Voter::reset()
{
  this->voted_ = false;

  // Only turn the LED off if it's "ON" (i.e., not flashing)
  if (this->led_desired_state_ == LEDDesiredState::REQUEST_ON)
  {
    this->led_desired_state_ = LEDDesiredState::REQUEST_OFF;
  }
}

bool Voter::get_led_state() const
{
  return this->led_current_state_;
}

LEDDesiredState Voter::get_desired_led_state() const
{
  return this->led_desired_state_;
}

void Voter::set_led_state(bool on)
{
  this->led_current_state_ = on;
}

uint Voter::get_button_pin() const
{
  return this->button_pin_;
}

uint Voter::get_led_pin() const
{
  return this->led_pin_;
}

std::string Voter::get_initials() const
{
  return this->initials_;
}

bool Voter::get_button_state() const
{
  return this->button_state_;
}

void Voter::set_button_state(bool pressed)
{
  this->button_state_ = pressed;
}