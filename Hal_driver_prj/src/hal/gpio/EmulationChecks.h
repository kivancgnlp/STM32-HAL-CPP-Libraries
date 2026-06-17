//
// Created by Kiv on 6/16/2026.
//

#ifndef HAL_DRIVER_PRJ_EMULATIONCHECKS_H
#define HAL_DRIVER_PRJ_EMULATIONCHECKS_H
#include <array>
#include <cstdint>
#include <cstdio>

#include "BitUtils.h"
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

    void resolve_register_data() {

        {
            const uint32_t mode_config_reg_val = bank_register_data.at(get_register_offset(GPIO_HW_Registers::MODE_CONFIG));

            unsigned i = 0;
            resolved_mode_configs.at(i++) = static_cast<GPIO_MODE>(get_bit_field<uint32_t,0,2>(mode_config_reg_val));
            resolved_mode_configs.at(i++) = static_cast<GPIO_MODE>(get_bit_field<uint32_t,2,2>(mode_config_reg_val));
            resolved_mode_configs.at(i++) = static_cast<GPIO_MODE>(get_bit_field<uint32_t,4,2>(mode_config_reg_val));
            resolved_mode_configs.at(i++) = static_cast<GPIO_MODE>(get_bit_field<uint32_t,6,2>(mode_config_reg_val));

            resolved_mode_configs.at(i++) = static_cast<GPIO_MODE>(get_bit_field<uint32_t,8,2>(mode_config_reg_val));
            resolved_mode_configs.at(i++) = static_cast<GPIO_MODE>(get_bit_field<uint32_t,10,2>(mode_config_reg_val));
            resolved_mode_configs.at(i++) = static_cast<GPIO_MODE>(get_bit_field<uint32_t,12,2>(mode_config_reg_val));
            resolved_mode_configs.at(i++) = static_cast<GPIO_MODE>(get_bit_field<uint32_t,14,2>(mode_config_reg_val));

            resolved_mode_configs.at(i++) = static_cast<GPIO_MODE>(get_bit_field<uint32_t,16,2>(mode_config_reg_val));
            resolved_mode_configs.at(i++) = static_cast<GPIO_MODE>(get_bit_field<uint32_t,18,2>(mode_config_reg_val));
            resolved_mode_configs.at(i++) = static_cast<GPIO_MODE>(get_bit_field<uint32_t,20,2>(mode_config_reg_val));
            resolved_mode_configs.at(i++) = static_cast<GPIO_MODE>(get_bit_field<uint32_t,22,2>(mode_config_reg_val));

            resolved_mode_configs.at(i++) = static_cast<GPIO_MODE>(get_bit_field<uint32_t,24,2>(mode_config_reg_val));
            resolved_mode_configs.at(i++) = static_cast<GPIO_MODE>(get_bit_field<uint32_t,26,2>(mode_config_reg_val));
            resolved_mode_configs.at(i++) = static_cast<GPIO_MODE>(get_bit_field<uint32_t,28,2>(mode_config_reg_val));
            resolved_mode_configs.at(i++) = static_cast<GPIO_MODE>(get_bit_field<uint32_t,30,2>(mode_config_reg_val));

        }

        {
            const uint32_t output_type_config_reg_val = bank_register_data.at(get_register_offset(GPIO_HW_Registers::OUTPUT_TYPE_CONFIG));

            unsigned i = 0;
            resolved_output_type_configs.at(i++) = static_cast<GPIO_OUTPUT_TYPE>(get_bit_field<uint32_t,0,1>(output_type_config_reg_val));
            resolved_output_type_configs.at(i++) = static_cast<GPIO_OUTPUT_TYPE>(get_bit_field<uint32_t,1,1>(output_type_config_reg_val));
            resolved_output_type_configs.at(i++) = static_cast<GPIO_OUTPUT_TYPE>(get_bit_field<uint32_t,2,1>(output_type_config_reg_val));
            resolved_output_type_configs.at(i++) = static_cast<GPIO_OUTPUT_TYPE>(get_bit_field<uint32_t,3,1>(output_type_config_reg_val));

            resolved_output_type_configs.at(i++) = static_cast<GPIO_OUTPUT_TYPE>(get_bit_field<uint32_t,4,1>(output_type_config_reg_val));
            resolved_output_type_configs.at(i++) = static_cast<GPIO_OUTPUT_TYPE>(get_bit_field<uint32_t,5,1>(output_type_config_reg_val));
            resolved_output_type_configs.at(i++) = static_cast<GPIO_OUTPUT_TYPE>(get_bit_field<uint32_t,6,1>(output_type_config_reg_val));
            resolved_output_type_configs.at(i++) = static_cast<GPIO_OUTPUT_TYPE>(get_bit_field<uint32_t,7,1>(output_type_config_reg_val));

            resolved_output_type_configs.at(i++) = static_cast<GPIO_OUTPUT_TYPE>(get_bit_field<uint32_t,8,1>(output_type_config_reg_val));
            resolved_output_type_configs.at(i++) = static_cast<GPIO_OUTPUT_TYPE>(get_bit_field<uint32_t,9,1>(output_type_config_reg_val));
            resolved_output_type_configs.at(i++) = static_cast<GPIO_OUTPUT_TYPE>(get_bit_field<uint32_t,10,1>(output_type_config_reg_val));
            resolved_output_type_configs.at(i++) = static_cast<GPIO_OUTPUT_TYPE>(get_bit_field<uint32_t,11,1>(output_type_config_reg_val));

            resolved_output_type_configs.at(i++) = static_cast<GPIO_OUTPUT_TYPE>(get_bit_field<uint32_t,12,1>(output_type_config_reg_val));
            resolved_output_type_configs.at(i++) = static_cast<GPIO_OUTPUT_TYPE>(get_bit_field<uint32_t,13,1>(output_type_config_reg_val));
            resolved_output_type_configs.at(i++) = static_cast<GPIO_OUTPUT_TYPE>(get_bit_field<uint32_t,14,1>(output_type_config_reg_val));
            resolved_output_type_configs.at(i++) = static_cast<GPIO_OUTPUT_TYPE>(get_bit_field<uint32_t,15,1>(output_type_config_reg_val));
        }


        for (unsigned i = 0; i < NUMBER_OF_PINS_IN_THE_BANK; i++) {
            std::printf("Pin %d : mode : %s\n", i,get_mode_config_txt(resolved_mode_configs.at(i)).data());
        }




    }
};


}// namespace kiv::hal::gpio

#endif //HAL_DRIVER_PRJ_EMULATIONCHECKS_H