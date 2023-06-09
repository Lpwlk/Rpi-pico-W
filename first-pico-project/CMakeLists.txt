# Generated Cmake Pico project file
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
# set(PICO_SDK_PATH "/Users/loicpwlk/Documents/Code/Rpi-pico/piconetwork/pico-sdk")
set(PICO_SDK_PATH "/Users/loicpwlk/Documents/Code/Rpi-pico/pico-sdk")
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull-in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(adc-picoW C CXX ASM)

add_executable(adc-picoW main.c)

# Add the standard library to the build
target_link_libraries(adc-picoW # Project & Cmake config name
        pico_cyw43_arch_none    # libs to link to our project source file(s)
        pico_stdlib
        hardware_adc
        hardware_dma
        )

        
# Add the standard include files to the build
target_include_directories(adc-picoW PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)
# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()
pico_set_program_name(adc-picoW "adc-picoW")
pico_set_program_version(adc-picoW "0.1")
pico_enable_stdio_usb(adc-picoW 1)
pico_enable_stdio_uart(adc-picoW 1)

# Add the standard include files to the build
target_include_directories(adc-picoW PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

pico_add_extra_outputs(adc-picoW)

