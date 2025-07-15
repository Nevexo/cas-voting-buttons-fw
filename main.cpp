#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"

#include "constants.hpp"
#include "rtos_tasks.hpp"
#include "voter.hpp"

TaskHandle_t voter_button_task_handle = nullptr;
TaskHandle_t led_control_task_handle = nullptr;
TaskHandle_t led_flash_task_handle = nullptr;
TaskHandle_t reset_button_task_handle = nullptr;
TaskHandle_t vote_monitor_task_handle = nullptr;
TaskHandle_t oled_display_task_handle = nullptr;

/**
 * Startup the hardware, configure the mux etc.
 */
void init_hardware()
{
  stdio_init_all();

  // Startup i2c
  i2c_init(I2C_PORT, 400 * 1000);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);
}

void deinit_hardware()
{
  // Bring down the I2C bus
  i2c_deinit(I2C_PORT);
  gpio_deinit(I2C_SDA);
  gpio_deinit(I2C_SCL);
  gpio_disable_pulls(I2C_SDA);
  gpio_disable_pulls(I2C_SCL);
}

void create_tasks(Voter *voters)
{
  printf("create_tasks: starting all tasks\n");
  xTaskCreate(voter_button_task, "VoterButtonTask", 256, voters, 1, &voter_button_task_handle);
  xTaskCreate(led_control_task, "LedControlTask", 256, voters, 1, &led_control_task_handle);
  xTaskCreate(led_flash_task, "LedFlashTask", 256, nullptr, 2, &led_flash_task_handle);
  xTaskCreate(vote_monitor_task, "VoteMonitorTask", 256, voters, 2, &vote_monitor_task_handle);
  // xTaskCreate(oled_display_task, "OLEDDisplayTask", 256, voters, 2, &oled_display_task_handle);
}

void stop_tasks()
{
  printf("stop_tasks: stopping all tasks.\n");
  vTaskDelete(voter_button_task_handle);
  vTaskDelete(led_control_task_handle);
  vTaskDelete(led_flash_task_handle);
  vTaskDelete(vote_monitor_task_handle);
  // vTaskDelete(oled_display_task_handle);

  // Do not stop the reset_button_task here as it'll stop itself.
}

int main()
{
  init_hardware();
  sleep_ms(1000);
  printf("CAS Voting Buttons - Cameron Fleming (c) 2025\n");

  Voter voters[] = {
      Voter("GJ", 5, 4),
      Voter("FK", 7, 6),
      Voter("AB", 9, 8),
      Voter("CF", 11, 10),
      Voter("AH", 13, 12),
      Voter("GD", 18, 14)};

  xTaskCreate(reset_button_task, "ResetButtonTask", 256, voters, 1, &reset_button_task_handle);
  create_tasks(voters);

  printf("Scheduler is starting!\n");
  vTaskStartScheduler();

  // Shouldn't reach here as the kernel has taken control
  while (1)
  {
  }
}
