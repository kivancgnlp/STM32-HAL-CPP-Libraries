# STM32-HAL-CPP-Libraries

> **Experimental — Under Active Construction**
> This project is in a very early stage. APIs are unstable, incomplete, and subject to breaking changes without notice. Not suitable for production use.

A modern C++20 hardware abstraction library for STM32 microcontrollers, developed as an alternative to the official STM32 HAL. The goal is to leverage C++ template metaprogramming to provide a zero-cost, type-safe peripheral API with built-in simulation support.

### Zero-Cost Simulation

The library is designed so that peripherals can be compiled in simulation mode on a host machine (x86/x64) without any runtime overhead on the target. Simulation behavior is selected at compile time via template parameters — when targeting real hardware, the simulation code is completely compiled away with no performance or size penalty.

## Status

| Peripheral | Status |
|---|---|
| GPIO | Skeleton / In Progress |

## Project Structure

```
Hal_driver_prj/
└── src/
    ├── hal/
    │   └── gpio/       # GPIO peripheral abstraction
    └── main.cpp        # Usage example
```

## Requirements

- C++20 compiler (arm-none-eabi-g++ or compatible)
- CMake 4.0+

## Build

```sh
cmake -S Hal_driver_prj -B build
cmake --build build
```

## Example

```cpp
#include "hal/gpio/Gpio.h"

using namespace kiv::hal::gpio;

GPIO_Bank<GPIO_BANK::BANK_A, true> gpio;
gpio.set_pin<GPIO_PIN::PIN_1>();
gpio.reset_pin<GPIO_PIN::PIN_1>();
```

## Contributing

The project is in its infancy — contributions, ideas, and feedback are welcome.