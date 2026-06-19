#include <iostream>

#include "hal/gpio/Gpio.h"
#include "hal/rcc/ClockConfiguration.h"
#include "hal/systick/SysTickController.h"

static constexpr bool SIMULATION = true;

int main() {
    using namespace kiv::hal::rcc;
    using namespace kiv::hal::gpio;
    using namespace kiv::hal::systick;

    // 1. System clock: HSE=8 MHz, M=8, N=360, P=2 → SYSCLK=180 MHz
    RCC_Controller<SIMULATION> rcc;
    rcc.configure_hse_pll_sysclk<8, 8, 360, 2>();
    rcc.enable_ahb1_clock<AHB1_Peripheral::GPIOA>();
    rcc.print_emulation_state();

    // 2. SysTick: 1 ms ticks driven by HCLK=180 MHz
    //    Must come after the system clock is stable so the reload value is correct.
    SysTick_Controller<SIMULATION> systick;
    systick.start<180'000'000>();

    // 3. PA5 as push-pull output (LD2 on NUCLEO-F429ZI)
    GPIO_Bank<GPIO_BANK::BANK_A, SIMULATION> gpio;
    gpio.agg_configure_pin_as_GP_output({GPIO_PIN::PIN_5});
    gpio.agg_commit_cached_config_to_hw();

    // 4. Blink at 1 Hz (500 ms on, 500 ms off)
    while (true) {
        gpio.set_pin<GPIO_PIN::PIN_5>();
        systick.delay_ms(500);
        gpio.reset_pin<GPIO_PIN::PIN_5>();
        systick.delay_ms(500);
    }
}
