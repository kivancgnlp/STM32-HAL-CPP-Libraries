#include <iostream>

#include "hal/gpio/Gpio.h"

int main() {

    using namespace  kiv::hal::gpio;

    GPIO_Bank<GPIO_BANK::BANK_A,true> gpio;

    gpio.set_pin<GPIO_PIN::PIN_1>();
    gpio.reset_pin<GPIO_PIN::PIN_1>();


    return 0;
}