# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (c) 2025 sevenlab engineering GmbH
#
cmake_minimum_required(VERSION 3.17)

set(LOG 1)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_SDK_PATH ${CMAKE_CURRENT_LIST_DIR}/pico-sdk)

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(picoports C CXX ASM)

pico_sdk_init()

add_executable(picoports)

target_sources(picoports PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pp_adc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pp_ctrl.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pp_gpio.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pp_i2c.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pp_uart.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/usb_descriptors.c
)

target_include_directories(picoports PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(picoports PUBLIC
  pico_stdlib
  hardware_adc
  hardware_i2c
  tinyusb_device
  tinyusb_board
)

option(LOG_ON_GP01 "Enable debug logging on GP0/GP1 (TX/RX resp.)")
if(LOG_ON_GP01)
target_compile_definitions(picoports PUBLIC PP_LOG_ON_GP01=1)
else()
# UART stdio is enabled by default, disabling it
pico_enable_stdio_uart(picoports 0)
endif()

option(BOOTSEL_BUTTON "Pressing the button resets the pico into BOOTSEL mode")
if(BOOTSEL_BUTTON)
target_link_libraries(picoports PUBLIC pico_bootrom)
target_compile_definitions(picoports PUBLIC PP_BTN_BOOTSEL=1)
endif()

# Create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(picoports)
