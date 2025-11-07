# This file is derived from debugprobe/CMakeLists.txt It's kept close to it's original content for
# diff-ability.
#
# The main changes are:
# - Move project definition and initialization to the top level.
# - Change the target to a library.
# - Replace relative paths.
# - Disable usb_descriptors.c, we're using the file from picoports.
# - Disable extra output formats.
cmake_minimum_required(VERSION 3.12)

# include(pico_sdk_import.cmake)
#
# set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_LIST_DIR}/freertos)
# include(FreeRTOS_Kernel_import.cmake)
#
# project(debugprobe)
#
# pico_sdk_init()

if (${PICO_SDK_VERSION_MAJOR} LESS 2)
	message(SEND_ERROR "Version 2 of the Pico SDK is required to compile this project. Please update your installation at ${PICO_SDK_PATH}")
endif ()

add_library(debugprobe
        ${DEBUGPROBE_PATH}/src/probe_config.c
        ${DEBUGPROBE_PATH}/src/main.c
        # src/usb_descriptors.c
        ${DEBUGPROBE_PATH}/src/probe.c
        ${DEBUGPROBE_PATH}/src/cdc_uart.c
        ${DEBUGPROBE_PATH}/src/get_serial.c
        ${DEBUGPROBE_PATH}/src/sw_dp_pio.c
        ${DEBUGPROBE_PATH}/src/tusb_edpt_handler.c
)

target_sources(debugprobe PRIVATE
        ${DEBUGPROBE_PATH}/CMSIS_DAP/CMSIS/DAP/Firmware/Source/DAP.c
        ${DEBUGPROBE_PATH}/CMSIS_DAP/CMSIS/DAP/Firmware/Source/JTAG_DP.c
        ${DEBUGPROBE_PATH}/CMSIS_DAP/CMSIS/DAP/Firmware/Source/DAP_vendor.c
        ${DEBUGPROBE_PATH}/CMSIS_DAP/CMSIS/DAP/Firmware/Source/SWO.c
        #CMSIS_DAP/CMSIS/DAP/Firmware/Source/SW_DP.c
        )

target_include_directories(debugprobe PRIVATE
        ${DEBUGPROBE_PATH}/CMSIS_DAP/CMSIS/DAP/Firmware/Include/
        ${DEBUGPROBE_PATH}/CMSIS_DAP/CMSIS/Core/Include/
        ${DEBUGPROBE_PATH}/include/
        )

target_compile_options(debugprobe PRIVATE -Wall)

pico_generate_pio_header(debugprobe ${DEBUGPROBE_PATH}/src/probe.pio)
pico_generate_pio_header(debugprobe ${DEBUGPROBE_PATH}/src/probe_oen.pio)

target_include_directories(debugprobe PRIVATE ${DEBUGPROBE_PATH}/src)

target_compile_definitions (debugprobe PRIVATE
	PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1
)

option (DEBUG_ON_PICO "Compile firmware for the Pico instead of Debug Probe" OFF)
if (DEBUG_ON_PICO)
    target_compile_definitions (debugprobe PRIVATE
	DEBUG_ON_PICO=1
    )
    if (PICO_BOARD STREQUAL "pico")
        set_target_properties(debugprobe PROPERTIES
            OUTPUT_NAME "debugprobe_on_pico"
        )
    elseif (PICO_BOARD STREQUAL "pico2")
        set_target_properties(debugprobe PROPERTIES
            OUTPUT_NAME "debugprobe_on_pico2"
        )
    else ()
        message(SEND_ERROR "Unsupported board ${PICO_BOARD}")
    endif ()
endif ()


target_link_libraries(debugprobe PRIVATE
        pico_multicore
        pico_stdlib
        pico_unique_id
        tinyusb_device
        tinyusb_board
        hardware_pio
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap1
)

pico_set_binary_type(debugprobe copy_to_ram)

# pico_add_extra_outputs(debugprobe)
