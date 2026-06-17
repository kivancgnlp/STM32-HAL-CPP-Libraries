//
// Created by Kiv on 6/16/2026.
//

#ifndef HAL_DRIVER_PRJ_EMULATIONCHECKS_H
#define HAL_DRIVER_PRJ_EMULATIONCHECKS_H
#include <array>
#include <cstdint>
#include <cstdio>

#include "../../utils/BitUtils.h"
#include "GpioDefinitions.h"

namespace kiv::hal::gpio {


template <bool EMULATION>
class EmulationChecks {

};

template <>
class EmulationChecks<true> {


    std::uint32_t pin_states;

    std::array<GPIO_MODE, NUMBER_OF_PINS_IN_THE_BANK> resolved_mode_configs{};
    std::array<GPIO_OUTPUT_TYPE, NUMBER_OF_PINS_IN_THE_BANK> resolved_output_type_configs{};
    std::array<GPIO_OUTPUT_SPEED, NUMBER_OF_PINS_IN_THE_BANK> resolved_output_speed_configs{};
    std::array<GPIO_PULL_UP_DOWN_CONFIG, NUMBER_OF_PINS_IN_THE_BANK> resolved_pullup_pulldown_configs{};

public:
    std::array<uint32_t, 10> bank_register_data;



    template <unsigned... N>
    void resolve_mode_configs(const uint32_t mode_config_reg_value, std::integer_sequence<unsigned,N...>) {

        using kiv::utils::get_bit_field;
        resolved_mode_configs = {static_cast<GPIO_MODE>(get_bit_field<uint32_t,N*2,2>(mode_config_reg_value))...};

    }

    template <unsigned... N>
    void resolve_output_type_configs(const uint32_t output_type_config_reg_val, std::integer_sequence<unsigned,N...>) {

        using kiv::utils::get_bit_field;
        resolved_output_type_configs = {static_cast<GPIO_OUTPUT_TYPE>(get_bit_field<uint32_t,N,1>(output_type_config_reg_val))...};

    }

    void resolve_register_data() {



        {
            const uint32_t mode_config_reg_val = bank_register_data.at(get_register_offset(GPIO_HW_Registers::MODE_CONFIG));
            resolve_mode_configs(mode_config_reg_val, std::make_integer_sequence<unsigned,NUMBER_OF_PINS_IN_THE_BANK>());


        }

        {
            const uint32_t output_type_config_reg_val = bank_register_data.at(get_register_offset(GPIO_HW_Registers::OUTPUT_TYPE_CONFIG));
            resolve_output_type_configs(output_type_config_reg_val, std::make_integer_sequence<unsigned,NUMBER_OF_PINS_IN_THE_BANK>());

        }


        for (unsigned i = 0; i < NUMBER_OF_PINS_IN_THE_BANK; i++) {
            std::printf("Pin %d : mode : %s\n", i,get_mode_config_txt(resolved_mode_configs.at(i)).data());
        }




    }
};


}// namespace kiv::hal::gpio

#endif //HAL_DRIVER_PRJ_EMULATIONCHECKS_H