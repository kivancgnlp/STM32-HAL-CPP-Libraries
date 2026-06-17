//
// Created by Kivanc Gunalp on 17.06.2026.
//

#ifndef HAL_DRIVER_PRJ_ALTERNATEFUNCTIONS_INFO_H
#define HAL_DRIVER_PRJ_ALTERNATEFUNCTIONS_INFO_H

#include <array>
#include <string_view>
#include "GpioDefinitions.h"

namespace kiv::hal::gpio {

    using my_pair = std::pair<int, std::string_view>;
    using my_ust_pair = std::array<my_pair, 16>;
    using top_arr = std::array<my_ust_pair, 2>;

    static constexpr top_arr bank_a_pin_alternate_functions = {

        my_ust_pair{
            std::pair{1, "TIM2_CH1"},
            std::pair{2, "TIM5_CH1"},
            std::pair{3, "TIM8_ETR"},
            std::pair{7, "USART2_CTS"},
            std::pair{8, "UART4_TX"},
            std::pair{11, "ETH_MII_CRS"},
            std::pair{15, "EVENT_TOUT"}
        },

        {
            std::pair{1, "TIM2_CH2"},
            std::pair{2, "TIM5_CH2"},
            std::pair{7, "USART2_RTS"},
            std::pair{8, "UART4_RX"},
            std::pair{11, "ETH_MII_RX_CLK"},
            std::pair{15, "EVENT_TOUT"}
        },

    };

    static constexpr std::string_view check_AF_availability(GPIO_PIN gpio_pin, AlternateFunction af) {

        auto ne = bank_a_pin_alternate_functions.at(static_cast<unsigned>(gpio_pin));

        unsigned pin_index = static_cast<unsigned>(af);
        for ( const auto &i : ne) {
            if (i.first == pin_index ) {
                return i.second;
            }
        }

        assert(false);
        return std::string_view{};
    }

};//namespace kiv::hal::gpio



#endif //HAL_DRIVER_PRJ_ALTERNATEFUNCTIONS_INFO_H
