#include <iostream>

#include "hal/gpio/Gpio.h"
#include "hal/rcc/ClockConfiguration.h"
#include "hal/systick/SysTickController.h"
#include "hal/uart/UartController.h"

static constexpr bool SIMULATION = true;

int main() {
    using namespace kiv::hal::rcc;
    using namespace kiv::hal::gpio;
    using namespace kiv::hal::systick;
    using namespace kiv::hal::uart;

    // 1. System clock: HSE=8 MHz, M=8, N=360, P=2 → SYSCLK=180 MHz
    //    AHB=180 MHz (DIV1), APB1=45 MHz (DIV4), APB2=90 MHz (DIV2)
    RCC_Controller<SIMULATION> rcc;
    rcc.configure_hse_pll_sysclk<8, 8, 360, 2>();
    rcc.enable_ahb1_clock<AHB1_Peripheral::GPIOA>();
    rcc.enable_apb1_clock<APB1_Peripheral::USART2>();
    rcc.print_emulation_state();

    // 2. SysTick: 1 ms ticks
    SysTick_Controller<SIMULATION> systick;
    systick.start<180'000'000>();

    // 3. Configure PA2 (TX) and PA3 (RX) as AF7 for USART2
    GPIO_Bank<GPIO_BANK::BANK_A, SIMULATION> gpio;
    gpio.agg_configure_pin_as_GP_output({GPIO_PIN::PIN_5}); // PA5 = LD2 LED
    gpio.agg_configure_pin_as_AF_Output<GPIO_PIN::PIN_2, PeripheralFunction::USART2_TX>();
    // PA3 (USART2_RX) would be configured as AF input — not yet supported by GPIO driver
    gpio.agg_commit_cached_config_to_hw();

    // 4. Configure USART2 at 115200 baud
    //    USART2 is on APB1 → PCLK = 45 MHz (SYSCLK/4)
    UART_Controller<USART_ID::USART_2, SIMULATION> uart;
    uart.configure<115200, 45'000'000>();
    uart.print_emulation_state();

    uart.transmit("Boot OK\r\n");

    // 5. Blink at 1 Hz, report over UART each cycle
    uint32_t blink_count = 0;
    while (true) {
        gpio.set_pin<GPIO_PIN::PIN_5>();
        systick.delay_ms(500);

        gpio.reset_pin<GPIO_PIN::PIN_5>();
        systick.delay_ms(500);

        ++blink_count;
        char buf[32];
        std::snprintf(buf, sizeof(buf), "Blink #%lu\r\n",
                      static_cast<unsigned long>(blink_count));
        uart.transmit(buf);
    }
}
