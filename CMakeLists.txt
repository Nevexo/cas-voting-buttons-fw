# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")
set(PICO_EXTRAS_FETCH_FROM_GIT true)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)

project(cas-voting-buttons C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

set(FREERTOS_KERNEL_PATH ${CMAKE_SOURCE_DIR}/extern/FreeRTOS-Kernel)
include(FreeRTOS_Kernel_import.cmake)

add_executable(cas-voting-buttons main.cpp voter.cpp rtos_tasks.cpp reset_button.cpp display.cpp)

pico_set_program_name(cas-voting-buttons "cas-voting-buttons")
pico_set_program_version(cas-voting-buttons "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(cas-voting-buttons 1)
pico_enable_stdio_usb(cas-voting-buttons 1)

# Include libraries
add_subdirectory(extern/pico-ssd1306)

# Add the standard library to the build
target_link_libraries(cas-voting-buttons
    pico_ssd1306
    hardware_sleep
    FreeRTOS-Kernel-Heap4
    pico_stdlib)

# Add the standard include files to the build
target_include_directories(cas-voting-buttons PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(cas-voting-buttons 
        hardware_i2c
        )

pico_add_extra_outputs(cas-voting-buttons)

