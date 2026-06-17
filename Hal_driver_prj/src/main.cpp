#include <iostream>

#include "hal/gpio/Gpio.h"
#include "hal/gpio/AlternateFunctions_Info.h"

static constexpr bool SIMULATION = true;

int main() {


    constexpr auto ne = bank_a_pin_alternate_functions.at(0).at(1);
    std::cout << ne.second << std::endl;

    using namespace  kiv::hal::gpio;

    GPIO_Bank<GPIO_BANK::BANK_A, SIMULATION> gpio;

    gpio.agg_configure_pin_as_GP_output({GPIO_PIN::PIN_1,GPIO_PIN::PIN_5});
    gpio.agg_configure_pin_as_GP_input({GPIO_PIN::PIN_3,GPIO_PIN::PIN_4});

    gpio.agg_commit_cached_config_to_hw();

    gpio.set_pin<GPIO_PIN::PIN_1>();

    bool val = gpio.read_pin<GPIO_PIN::PIN_1>();

    gpio.reset_pin<GPIO_PIN::PIN_1>();




    return 0;
}