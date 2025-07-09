#include "reset_button.hpp"

void reset_all_voters(Voter *voters, uint active_voters)
{
  // Reset every voter
  // The voter object will update the LED desired state and update the OLED.
  for (int i = 0; i < active_voters; i++)
  {
    voters[i].reset();
  }

  xTaskNotifyGive(vote_monitor_task_handle);
}

void deinit_all_voters(Voter *voters)
{
  for (int i = 0; i < 6; i++)
  {
    gpio_put(voters[i].get_led_pin(), false);
    gpio_disable_pulls(voters[i].get_button_pin());
    gpio_deinit(voters[i].get_led_pin());
    gpio_deinit(voters[i].get_button_pin());
  }
}