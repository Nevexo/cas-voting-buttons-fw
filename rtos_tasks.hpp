#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include "voter.hpp"
#include "reset_button.hpp"
#include "display.hpp"

void voter_button_task(void *pvParameters);
void led_control_task(void *pvParameters);
void led_flash_task(void *pvParameters);
void reset_button_task(void *pvParameters);
void vote_monitor_task(void *pvParameters);
void oled_display_task(void *pvParameters);

void create_tasks(Voter *voters);
void stop_tasks();
void init_hardware();
void deinit_hardware();