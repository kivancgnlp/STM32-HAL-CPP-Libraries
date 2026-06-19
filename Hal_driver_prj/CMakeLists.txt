cmake_minimum_required(VERSION 4.0)
project(Project)

set(CMAKE_CXX_STANDARD 20)

add_executable(Project
        src/hal/gpio/Gpio.cpp
        src/hal/gpio/Gpio.h
        src/main.cpp
        src/hal/gpio/EmulationChecks.cpp
        src/hal/gpio/EmulationChecks.h
        src/hal/gpio/GpioDefinitions.h
        src/hal/gpio/AlternateFunctions_Info.h
        src/hal/rcc/ClockConfiguration.cpp
        src/hal/rcc/ClockConfiguration.h
        src/hal/rcc/RccDefinitions.h
        src/hal/systick/SysTickController.cpp
        src/hal/systick/SysTickController.h
        src/hal/systick/SysTickEmulationHelper.h
        src/hal/systick/SysTickDefinitions.h)
