# STM32 HAL CPP Libraries

> **Experimental — Under Active Construction**
> This project is in a very early stage. APIs are unstable, incomplete, and subject to breaking changes without notice. Not suitable for production use.

A modern C++20 hardware abstraction library for **STM32F429** microcontrollers, developed as an alternative to the official STM32 HAL. The goal is to leverage C++ template metaprogramming to provide a zero-cost, type-safe peripheral API with built-in simulation support.

Register layouts, bit positions, and peripheral addresses are derived from the **STM32F429 Reference Manual (RM0090)** and cross-checked against the official STM32F4xx HAL driver source.

### Zero-Cost Simulation

The library is designed so that peripherals can be compiled in simulation mode on a host machine (x86/x64) without any runtime overhead on the target. Simulation behavior is selected at compile time via a boolean template parameter — when targeting real hardware, the simulation code is completely compiled away with no performance or size penalty. In simulation mode, register reads/writes go to an internal array and hardware ready-flag polling is skipped, so the full configuration logic runs and can be inspected on the host. SysTick simulation is an exception: instead of emulating a register, a `std::thread` drives the millisecond tick counter at real wall-clock time, so `delay_ms()` behaves correctly on the host.

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
| SysTick | `hal/systick/` | In Progress |
| UART (polling) | `hal/uart/` | In Progress |

## Project Structure

```
Hal_driver_prj/
└── src/
    ├── hal/
    │   ├── gpio/               # GPIO peripheral abstraction
    │   │   ├── Gpio.h              # GPIO_Bank<BANK, EMULATION> class template
    │   │   ├── GpioDefinitions.h   # Enums, register offsets, consteval helpers
    │   │   ├── EmulationChecks.h   # Simulation-mode register decoder
    │   │   └── AlternateFunctions_Info.h  # AF table, PeripheralFunction enum, compile-time checks
    │   ├── rcc/                # Reset and Clock Control abstraction
    │   │   ├── ClockConfiguration.h  # RCC_Controller<EMULATION> class template
    │   │   └── RccDefinitions.h      # Enums, register offsets, consteval helpers
    │   ├── systick/            # SysTick timer abstraction
    │   │   ├── SysTickController.h   # SysTick_Controller<EMULATION> class template
    │   │   ├── SysTickController.cpp # g_systick_ms_count definition + SysTick_Handler ISR
    │   │   ├── SysTickDefinitions.h  # Register addresses, bit positions
    │   │   └── SysTickEmulationHelper.h  # std::thread-based tick simulation
    │   └── uart/               # UART peripheral abstraction (polling)
    │       ├── UartController.h      # UART_Controller<ID, EMULATION> class template
    │       ├── UartController.cpp    # Translation unit
    │       └── UartDefinitions.h     # Base addresses, register offsets, bit positions
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

### Compile-time alternate function safety

`AlternateFunctions_Info.h` holds a 2D `constexpr` table (`AF_Table = array<array<string_view,16>,16>`) indexed as `[pin][af]` and sourced from the STM32F429 datasheet. Three layers of compile-time protection are built on top of it:

**1. Pin / AF availability check**

The low-level overload (by AF number) `static_assert`s that the requested AF is actually mapped to that pin in the table:

```cpp
// Compile error — AF5 is not USART2_TX on PA2:
gpio.agg_configure_pin_as_AF_Output<GPIO_PIN::PIN_2, AlternateFunction::AF_5>();
// error: Selected AF is not available on this pin.
```

**2. Select by peripheral name instead of AF number**

A `PeripheralFunction` enum lists every peripheral function that appears in the table. The high-level overload looks up the correct AF number automatically — no need to open the datasheet to find the AF number:

```cpp
// AF number resolved at compile time from the table
gpio.agg_configure_pin_as_AF_Output<GPIO_PIN::PIN_2, PeripheralFunction::USART2_TX>();

// Compile error — USART2_TX is not routable to PA5:
gpio.agg_configure_pin_as_AF_Output<GPIO_PIN::PIN_5, PeripheralFunction::USART2_TX>();
// error: Selected peripheral function is not available on this pin.
```

**3. Automatic output type and speed defaults**

`get_peripheral_defaults(PeripheralFunction)` returns the correct `GPIO_OUTPUT_TYPE` and `GPIO_OUTPUT_SPEED` for each peripheral family. These become the template default arguments — no manual configuration needed:

| Peripheral family | Output type | Output speed |
|---|---|---|
| I2C | `OPEN_DRAIN` | `LOW` |
| UART / USART | `PUSH_PULL` | `LOW` |
| SPI | `PUSH_PULL` | `HIGH` |
| CAN | `PUSH_PULL` | `MEDIUM` |
| USB OTG | `PUSH_PULL` | `HIGH` |
| Ethernet MII | `PUSH_PULL` | `VERY_HIGH` |
| MCO | `PUSH_PULL` | `VERY_HIGH` |

**4. Open-drain enforcement for I2C**

I2C lines must be open-drain (the protocol requires it for multi-master operation). If the output type is overridden to `PUSH_PULL` on an I2C pin, a `static_assert` fires:

```cpp
// Default is OPEN_DRAIN — works fine:
gpio.agg_configure_pin_as_AF_Output<GPIO_PIN::PIN_8, PeripheralFunction::I2C3_SCL>();

// Explicit override to PUSH_PULL — compile error:
gpio.agg_configure_pin_as_AF_Output<GPIO_PIN::PIN_8, PeripheralFunction::I2C3_SCL,
                                     GPIO_OUTPUT_TYPE::PUSH_PULL>();
// error: This peripheral requires OPEN_DRAIN output type (e.g. I2C).
```

Speed can still be overridden freely (e.g. I2C fast-mode 400 kHz → `MEDIUM`).

## Examples

### GPIO

```cpp
#include "hal/gpio/Gpio.h"

static constexpr bool SIMULATION = true; // false → real hardware

using namespace kiv::hal::gpio;

GPIO_Bank<GPIO_BANK::BANK_A, SIMULATION> gpio;

// General-purpose I/O
gpio.agg_configure_pin_as_GP_output({GPIO_PIN::PIN_1, GPIO_PIN::PIN_5});
gpio.agg_configure_pin_as_GP_input({GPIO_PIN::PIN_3});

// Alternate function — select by peripheral name (recommended)
// Output type and speed are set automatically from the peripheral defaults table.
gpio.agg_configure_pin_as_AF_Output<GPIO_PIN::PIN_2, PeripheralFunction::USART2_TX>();
gpio.agg_configure_pin_as_AF_Output<GPIO_PIN::PIN_8, PeripheralFunction::I2C3_SCL>(); // auto OPEN_DRAIN

// Alternate function — select by AF number (when exact AF is known)
gpio.agg_configure_pin_as_AF_Output<GPIO_PIN::PIN_2, AlternateFunction::AF_7>();

// Speed can be overridden while output type defaults are preserved
gpio.agg_configure_pin_as_AF_Output<GPIO_PIN::PIN_2, PeripheralFunction::USART2_TX,
                                     GPIO_OUTPUT_TYPE::PUSH_PULL,
                                     GPIO_OUTPUT_SPEED::MEDIUM>();

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

### SysTick — Millisecond Tick Counter

```cpp
#include "hal/systick/SysTickController.h"

static constexpr bool SIMULATION = true;

using namespace kiv::hal::systick;

SysTick_Controller<SIMULATION> systick;

// Must be called after RCC configures the system clock.
// HCLK_HZ must match the actual HCLK — used to compute the reload value at compile time.
systick.start<180'000'000>(); // 1 ms ticks at 180 MHz HCLK

systick.delay_ms(500);        // blocking delay

uint32_t t = systick.get_tick_ms(); // raw tick counter value
```

In simulation, `start()` spawns a `std::thread` that increments the tick counter every millisecond using `std::this_thread::sleep_for`. On real hardware, `start()` programs `STK_LOAD`/`STK_VAL`/`STK_CTRL` and the `SysTick_Handler` ISR drives the counter.

### UART — Polling Transmit / Receive

```cpp
#include "hal/uart/UartController.h"

static constexpr bool SIMULATION = true;

using namespace kiv::hal::uart;
using namespace kiv::hal::gpio;

// Configure PA2 (TX) and PA3 (RX) as USART2 alternate functions.
// Output type (PUSH_PULL) and speed (LOW) are applied automatically.
GPIO_Bank<GPIO_BANK::BANK_A, SIMULATION> gpio;
gpio.agg_configure_pin_as_AF_Output<GPIO_PIN::PIN_2, PeripheralFunction::USART2_TX>();
gpio.agg_configure_pin_as_AF_Output<GPIO_PIN::PIN_3, PeripheralFunction::USART2_RX>();
gpio.agg_commit_cached_config_to_hw();

// USART2 is on APB1 → pass the APB1 clock (45 MHz at 180 MHz SYSCLK with DIV4).
// BRR = PCLK / BAUD is validated at compile time (static_assert on range).
UART_Controller<USART_ID::USART_2, SIMULATION> uart;
uart.configure<115200, 45'000'000>();

uart.transmit("Hello\r\n");         // blocking transmit of a string
uart.transmit_byte(0xAB);           // single byte

if (uart.data_available()) {
    uint8_t b = uart.receive_byte_blocking();
}

uart.print_emulation_state();       // decode and print register state (simulation only)
```

BRR compile-time validation — triggers a `static_assert` at build time:

```cpp
// Error: BRR overflow — PCLK_HZ or BAUD_RATE is wrong
uart.configure<115200, 450>();      // PCLK suspiciously low
```

> **Bus clock note:** USART1 and USART6 are on APB2 (up to 90 MHz). USART2–UART5 are on APB1 (up to 45 MHz). Pass the correct bus clock to `configure<>()` — use `is_on_apb2(USART_ID)` from `UartDefinitions.h` to check at compile time.

> **Simulation behaviour:** `transmit` / `transmit_byte` write to `stdout`. `receive_byte_blocking` reads from `stdin`. `data_available()` returns `false` (non-blocking stdin polling is platform-specific).

### LED Blink — Full Example

```cpp
#include "hal/gpio/Gpio.h"
#include "hal/rcc/ClockConfiguration.h"
#include "hal/systick/SysTickController.h"

static constexpr bool SIMULATION = true;

int main() {
    using namespace kiv::hal::rcc;
    using namespace kiv::hal::gpio;
    using namespace kiv::hal::systick;

    RCC_Controller<SIMULATION> rcc;
    rcc.configure_hse_pll_sysclk<8, 8, 360, 2>(); // SYSCLK = 180 MHz
    rcc.enable_ahb1_clock<AHB1_Peripheral::GPIOA>();

    SysTick_Controller<SIMULATION> systick;
    systick.start<180'000'000>();                  // 1 ms ticks

    GPIO_Bank<GPIO_BANK::BANK_A, SIMULATION> gpio;
    gpio.agg_configure_pin_as_GP_output({GPIO_PIN::PIN_5}); // PA5 = LD2
    gpio.agg_commit_cached_config_to_hw();

    while (true) {
        gpio.set_pin<GPIO_PIN::PIN_5>();
        systick.delay_ms(500);
        gpio.reset_pin<GPIO_PIN::PIN_5>();
        systick.delay_ms(500);
    }
}
```

## Contributing

The project is in its infancy — contributions, ideas, and feedback are welcome.
