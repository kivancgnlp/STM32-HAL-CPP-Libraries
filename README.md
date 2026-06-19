# STM32 HAL CPP Libraries

> **Experimental — Under Active Construction**
> This project is in a very early stage. APIs are unstable, incomplete, and subject to breaking changes without notice. Not suitable for production use.

A modern C++20 hardware abstraction library for **STM32F429** microcontrollers, developed as an alternative to the official STM32 HAL. The goal is to leverage C++ template metaprogramming to provide a zero-cost, type-safe peripheral API with built-in simulation support.

Register layouts, bit positions, and peripheral addresses are derived from the **STM32F429 Reference Manual (RM0090)** and cross-checked against the official STM32F4xx HAL driver source.

### Zero-Cost Simulation

The library is designed so that peripherals can be compiled in simulation mode on a host machine (x86/x64) without any runtime overhead on the target. Simulation behavior is selected at compile time via a boolean template parameter — when targeting real hardware, the simulation code is completely compiled away with no performance or size penalty. In simulation mode, register reads/writes go to an internal array and hardware ready-flag polling is skipped, so the full configuration logic runs and can be inspected on the host.

## Target Hardware

| Parameter | Value |
|---|---|
| MCU family | STM32F4xx |
| Tested device | **STM32F429** |
| Core | ARM Cortex-M4 with FPU |
| Max SYSCLK | 180 MHz |
| Reference manual | RM0090 |

## Status

| Peripheral | Module | Status |
|---|---|---|
| GPIO | `hal/gpio/` | In Progress |
| RCC / Clock | `hal/rcc/` | In Progress |

## Project Structure

```
Hal_driver_prj/
└── src/
    ├── hal/
    │   ├── gpio/               # GPIO peripheral abstraction
    │   │   ├── Gpio.h              # GPIO_Bank<BANK, EMULATION> class template
    │   │   ├── GpioDefinitions.h   # Enums, register offsets, consteval helpers
    │   │   ├── EmulationChecks.h   # Simulation-mode register decoder
    │   │   └── AlternateFunctions_Info.h
    │   └── rcc/                # Reset and Clock Control abstraction
    │       ├── ClockConfiguration.h  # RCC_Controller<EMULATION> class template
    │       └── RccDefinitions.h      # Enums, register offsets, consteval helpers
    ├── utils/
    │   └── BitUtils.h          # Type-safe bit field get/set/mask templates
    └── main.cpp                # Usage example
```

## Requirements

- C++20 compiler (`arm-none-eabi-g++` for hardware, or any host compiler for simulation)
- CMake 4.0+

## Build

```sh
cmake -S Hal_driver_prj -B build
cmake --build build
```

## Design Patterns

### `write_hw_register` / `read_hw_register`

Every peripheral class exposes a private pair of register-access methods templated on a register enum. On real hardware they dereference the computed peripheral address; in simulation mode they redirect to an internal `std::array`. The address is computed by adding the integer byte offset to the base address before casting to a pointer — avoiding C++ pointer-arithmetic scaling issues.

### Bit field operations (`BitUtils.h`)

All bit manipulation uses the `kiv::utils` helpers:

| Function | Purpose |
|---|---|
| `get_bit_mask<T, start, count>()` | Compile-time mask |
| `get_bit_field<T, start, count>(value)` | Extract a field |
| `update_bit_field<T, start, count>(reg, val)` | Insert a field (read-modify-write safe) |

### Aggregate configure + commit (GPIO)

GPIO pins are staged through cached arrays (`agg_configure_pin_as_*`) then written to hardware in one call (`agg_commit_cached_config_to_hw`), minimising the number of register writes and making the intent explicit.

## Examples

### GPIO

```cpp
#include "hal/gpio/Gpio.h"

static constexpr bool SIMULATION = true; // false → real hardware

using namespace kiv::hal::gpio;

GPIO_Bank<GPIO_BANK::BANK_A, SIMULATION> gpio;

gpio.agg_configure_pin_as_GP_output({GPIO_PIN::PIN_1, GPIO_PIN::PIN_5});
gpio.agg_configure_pin_as_GP_input({GPIO_PIN::PIN_3});
gpio.agg_commit_cached_config_to_hw();

gpio.set_pin<GPIO_PIN::PIN_1>();
gpio.reset_pin<GPIO_PIN::PIN_1>();
```

### RCC — Clock Configuration

```cpp
#include "hal/rcc/ClockConfiguration.h"

static constexpr bool SIMULATION = true;

using namespace kiv::hal::rcc;

RCC_Controller<SIMULATION> rcc;

// HSE = 8 MHz crystal, PLL: M=8 N=360 P=2 → SYSCLK = 180 MHz
// AHB = 180 MHz, APB1 = 45 MHz (÷4), APB2 = 90 MHz (÷2)
rcc.configure_hse_pll_sysclk<8, 8, 360, 2>();

// Or from the internal 16 MHz HSI oscillator: SYSCLK = (16/8)*180/2 = 180 MHz
// rcc.configure_hsi_pll_sysclk<8, 180, 2>();

// Enable peripheral clocks
rcc.enable_ahb1_clock<AHB1_Peripheral::GPIOA>();
rcc.enable_apb1_clock<APB1_Peripheral::USART2>();

// Print decoded register state (simulation only)
rcc.print_emulation_state();
```

Compile-time frequency validation — this triggers a `static_assert` at build time:

```cpp
// Error: SYSCLK cannot exceed 180 MHz
rcc.configure_hse_pll_sysclk<8, 4, 400, 2>(); // would be 200 MHz
```

## Contributing

The project is in its infancy — contributions, ideas, and feedback are welcome.
