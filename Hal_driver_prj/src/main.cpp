#include <iostream>

#include "hal/gpio/Gpio.h"

#include "hal/rcc/ClockConfiguration.h"


static constexpr bool SIMULATION = true;

void configure_gpio() {
    using namespace  kiv::hal::gpio;

    GPIO_Bank<GPIO_BANK::BANK_A, SIMULATION> gpio;

    gpio.agg_configure_pin_as_GP_output({GPIO_PIN::PIN_1,GPIO_PIN::PIN_5});
    gpio.agg_configure_pin_as_GP_input({GPIO_PIN::PIN_3,GPIO_PIN::PIN_4});
    gpio.agg_configure_pin_as_AF_Output<GPIO_PIN::PIN_0,AlternateFunction::AF_1>();

    gpio.agg_commit_cached_config_to_hw();

    gpio.set_pin<GPIO_PIN::PIN_1>();

    bool val = gpio.read_pin<GPIO_PIN::PIN_1>();

    gpio.reset_pin<GPIO_PIN::PIN_1>();

}

int main() {

    using namespace  kiv::hal::rcc;

    configure_external_system_clock_source_with_pll<8,8,360,2>();





    return 0;
}