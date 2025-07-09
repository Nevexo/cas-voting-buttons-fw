// Helper functions for the reset button task (rtos_tasks.cpp)
#pragma once

#include "voter.hpp"
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t vote_monitor_task_handle;

void reset_all_voters(Voter *voters, uint active_voters);
void deinit_all_voters(Voter *voters);