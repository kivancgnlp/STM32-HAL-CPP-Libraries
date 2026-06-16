cmake_minimum_required(VERSION 4.0)
project(Project)

set(CMAKE_CXX_STANDARD 20)

add_executable(Project
        src/hal/gpio/Gpio.cpp
        src/hal/gpio/Gpio.h
        src/main.cpp
        src/hal/gpio/EmulationChecks.cpp
        src/hal/gpio/EmulationChecks.h
        src/hal/gpio/GpioDefinitions.h)
