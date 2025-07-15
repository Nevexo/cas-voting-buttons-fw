#include "rtos_tasks.hpp"
#include "constants.hpp"
#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "pico/sleep.h"
#include <string>

extern TaskHandle_t voter_button_task_handle;
extern TaskHandle_t led_control_task_handle;
extern TaskHandle_t led_flash_task_handle;
extern TaskHandle_t reset_button_task_handle;
extern TaskHandle_t vote_monitor_task_handle;
extern TaskHandle_t oled_display_task_handle;

enum class OperationMode
{
  MODE_4_VOTERS,
  MODE_6_VOTERS
};

static OperationMode op_mode = OperationMode::MODE_6_VOTERS;
static bool all_flash = false;

// Toggled by flash task
static bool flash_state = false;

void voter_button_task(void *pvParameters)
{
  printf("voter_button_task: initalising...\n");
  Voter *voters = static_cast<Voter *>(pvParameters);

  // Setup the hardware
  for (int i = 0; i < 6; i++)
  {
    uint btn_pin = voters[i].get_button_pin();
    uint led_pin = voters[i].get_led_pin();

    gpio_init(btn_pin);
    gpio_set_dir(btn_pin, GPIO_IN);
    gpio_pull_up(btn_pin);

    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);
    gpio_put(led_pin, false);
  }

  vTaskDelay(pdMS_TO_TICKS(500));

  printf("voter_button_task: entering main loop\n");
  while (1)
  {
    int active_voters = op_mode == OperationMode::MODE_6_VOTERS ? 6 : 4;
    for (int i = 0; i < active_voters; i++)
    {
      bool btn_press = gpio_get(voters[i].get_button_pin());

      // Button pressed and debounced, user has not yet voted
      if (!btn_press && voters[i].get_button_state() && !voters[i].has_voted())
      {
        printf("%s has voted!\n", voters[i].get_initials().c_str());
        voters[i].vote();
        xTaskNotifyGive(vote_monitor_task_handle);
      }

      if (!btn_press != voters[i].get_button_state())
      {
        voters[i].set_button_state(!btn_press);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(SCAN_INTERVAL_MS));
  }
}

void led_control_task(void *pvParameters)
{
  printf("led_control_task: initalising...\n");
  Voter *voters = static_cast<Voter *>(pvParameters);
  bool pico_led_on = false;

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  printf("led_control_task: entering main loop\n");
  while (1)
  {
    // Blink on board LED
    if (pico_led_on != flash_state)
    {
      pico_led_on = flash_state;
      gpio_put(PICO_DEFAULT_LED_PIN, flash_state);
    }

    int active_voters = op_mode == OperationMode::MODE_6_VOTERS ? 6 : 4;
    for (int i = 0; i < active_voters; i++)
    {
      // Handle the system being in the "all flash" state, ignore the desired state of the voter.
      if (all_flash || voters[i].get_desired_led_state() == LEDDesiredState::REQUEST_FLASH)
      {
        if (voters[i].get_led_state() != flash_state)
        {
          voters[i].set_led_state(flash_state);
          gpio_put(voters[i].get_led_pin(), flash_state);
        }
      }

      // Drop out now if all are flashing.
      if (all_flash)
        continue;

      // Otherwise, get the desired state (will be on or off)
      if (voters[i].get_desired_led_state() == LEDDesiredState::REQUEST_ON)
      {
        if (!voters[i].get_led_state())
        {
          printf("led_control_task: turning on %s LED\n", voters[i].get_initials().c_str());
          // Turn on the LED
          voters[i].set_led_state(true);
          gpio_put(voters[i].get_led_pin(), true);
        }
      }
      else if (voters[i].get_desired_led_state() == LEDDesiredState::REQUEST_OFF)
      {
        if (voters[i].get_led_state())
        {
          // turn off the LED
          printf("led_control_task: turning off %s LED\n", voters[i].get_initials().c_str());
          voters[i].set_led_state(false);
          gpio_put(voters[i].get_led_pin(), false);
        }
      }
    }

    vTaskDelay(pdMS_TO_TICKS(LED_UPDATE_INTERVAL_MS));
  }
}

void led_flash_task(void *pvParameters)
{
  while (1)
  {
    flash_state = !flash_state;
    vTaskDelay(pdMS_TO_TICKS(LED_FLASH_INTERVAL_MS));
  }
}

void vote_monitor_task(void *pvParameters)
{
  Voter *voters = static_cast<Voter *>(pvParameters);

  while (1)
  {
    // Sit and wait for this task to be notified of a vote change.
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    uint active_voters = op_mode == OperationMode::MODE_6_VOTERS ? 6 : 4;

    bool l_all_voted = true;
    for (int i = 0; i < active_voters; i++)
    {
      if (!voters[i].has_voted())
        l_all_voted = false;
    }

    all_flash = l_all_voted;
    // Notify display task.
  }
}

void reset_button_task(void *pvParameters)
{
  // NOTE: THE RESET PIN IS PULLED DOWN BY THE MCU, AND UP BY THE BUTTON UNLIKE THE VOTER BUTTONS.
  printf("reset_button_task: initalising\n");
  Voter *voters = static_cast<Voter *>(pvParameters);

  // Setup hardware
  gpio_init(RESET_BTN_PIN);
  gpio_set_dir(RESET_BTN_PIN, GPIO_IN);
  gpio_pull_up(RESET_BTN_PIN);

  bool previous_state = false;
  uint32_t hold_start_time = 0;

  // Track the stage as we go, once the delta thresholds are crossed we'll increment this.
  uint stage = 0;

  // Enter a loop scanning the button
  while (1)
  {
    // Delay at the start to dea*l with early continue.
    vTaskDelay(pdMS_TO_TICKS(RESET_BUTTON_SCAN_INTERVAL_MS));

    // Invert because active low
    bool state = !gpio_get(RESET_BTN_PIN);

    // Got a press, record the time.
    if (state && !previous_state)
    {
      previous_state = true;
      hold_start_time = to_ms_since_boot(get_absolute_time());
      stage = 0;

      printf("reset_button_task: RESET BUTTON PRESSED\n");
      // Might as well continue, we won't be within the press
      // / debounce delay.
      continue;
    }

    if (!state && !previous_state)
      continue;

    uint32_t delta = to_ms_since_boot(get_absolute_time()) - hold_start_time;

    // Check the stage
    if (delta >= RESET_BUTTON_PRESS_MS && stage == 0)
    {
      stage = 1;
      printf("reset_button_task: Release for RESET\n");
    }

    if (delta >= RESET_BUTTON_HOLD_STAGE_1_MS && stage == 1)
    {
      stage = 2;
      printf("reset_button_task: Release for MODE CHANGE\n");
    }

    if (delta >= RESET_BUTTON_HOLD_STAGE_2_MS && stage == 2)
    {
      stage = 3;
      printf("reset_button_task: Release for POWER OFF\n");
    }

    // If we've just been released, perform the action
    if (!state && previous_state)
    {
      uint active_voters = op_mode == OperationMode::MODE_6_VOTERS ? 6 : 4;

      switch (stage)
      {
      case 0:
        break;

      case 1:
        reset_all_voters(voters, active_voters);
        break;

      case 2:
        // Change operational mode
        reset_all_voters(voters, 6);
        if (op_mode == OperationMode::MODE_4_VOTERS)
        {
          op_mode = OperationMode::MODE_6_VOTERS;
        }
        else if (op_mode == OperationMode::MODE_6_VOTERS)
        {
          op_mode = OperationMode::MODE_4_VOTERS;
        }
        break;

      case 3:
        printf("reset_button_task: stopping all tasks and preparing MCU for sleep\n");

        reset_all_voters(voters, 6);
        // Inform the OLED task
        xTaskNotifyGive(oled_display_task_handle);

        // Stop all tasks (except ourselves, we need to restore everything.)
        stop_tasks();

        // Deinit the voter GPIOs
        deinit_all_voters(voters);

        // Shut off the i2c bus
        deinit_hardware();

        // Shut off the reset button GPIO
        gpio_disable_pulls(RESET_BTN_PIN);
        gpio_deinit(RESET_BTN_PIN);

        // Go to sleep and wait for RESET to go low.
        printf("reset_button_task: switching clock source for sleep\n");
        uart_default_tx_wait_blocking();

        // Sleeping to wait for the button to stop bouncing.
        // This isn't the correct sleep under normal conditions, but we don't even want FreeRTOS to context switch us.
        sleep_ms(500);

        sleep_run_from_xosc();
        sleep_goto_dormant_until_edge_high(RESET_BTN_PIN);

        // CPU is coming awake again, power everything up and return to our clocks.
        sleep_power_up();
        printf("reset_button_task: Hello world! Resuming from sleep.");

        // Bring back the reset pin GPIO
        gpio_init(RESET_BTN_PIN);
        gpio_set_dir(RESET_BTN_PIN, GPIO_IN);
        gpio_pull_down(RESET_BTN_PIN);

        // Start the i2c bus
        init_hardware();

        // Relaunch the tasks, this will also setup hardware
        create_tasks(voters);

        printf("reset_button_task: !! Finished resume from sleep !!");
        break;
      }
    }

    previous_state = state;
  }
}

void oled_display_task(void *pvParameters)
{
  Voter *voters = static_cast<Voter *>(pvParameters);

  printf("oled_display_task: Display is starting.\n");
  OLEDDisplay display(I2C_PORT, I2C_SDA, I2C_SCL, 0x3C);
  if (!display.init())
  {
    printf("SSD1306 did not initalise!!");
    // Remove own task, the display isn't available.
    vTaskDelete(NULL);
  }

  printf("oled_display_task: Display is up, entering main loop.\n");

  while (1)
  {
    // Wait for task notifications.
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // TODO: Notification just turns it off lol
    display.deinit();
  }
}