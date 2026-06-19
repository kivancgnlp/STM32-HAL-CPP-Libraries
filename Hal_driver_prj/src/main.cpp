#include <iostream>

#include "hal/gpio/Gpio.h"
#include "hal/rcc/ClockConfiguration.h"


static constexpr bool SIMULATION = true;

void configure_gpio() {
    using namespace kiv::hal::gpio;

    GPIO_Bank<GPIO_BANK::BANK_A, SIMULATION> gpio;

    gpio.agg_configure_pin_as_GP_output({GPIO_PIN::PIN_1, GPIO_PIN::PIN_5});
    gpio.agg_configure_pin_as_GP_input({GPIO_PIN::PIN_3, GPIO_PIN::PIN_4});
    gpio.agg_configure_pin_as_AF_Output<GPIO_PIN::PIN_0, AlternateFunction::AF_1>();

    gpio.agg_commit_cached_config_to_hw();

    gpio.set_pin<GPIO_PIN::PIN_1>();

    bool val = gpio.read_pin<GPIO_PIN::PIN_1>();

    gpio.reset_pin<GPIO_PIN::PIN_1>();
}

int main() {
    using namespace kiv::hal::rcc;

    RCC_Controller<SIMULATION> rcc;

    // HSE=8 MHz, M=8, N=360, P=2 -> SYSCLK = (8/8)*360/2 = 180 MHz
    rcc.configure_hse_pll_sysclk<8, 8, 360, 2>();

    // Enable peripheral clocks after the system clock is configured
    rcc.enable_ahb1_clock<AHB1_Peripheral::GPIOA>();
    rcc.enable_apb1_clock<APB1_Peripheral::USART2>();

    rcc.print_emulation_state();

    configure_gpio();

    return 0;
}
