//
// Created by Kivanc Gunalp on 14.06.2026.
//

#ifndef STM_GPIO_CPP_DEN_HALGPIO_H
#define STM_GPIO_CPP_DEN_HALGPIO_H
#include <array>
#include <cstdint>
#include <cstdio>
#include <optional>

#include "EmulationChecks.h"
#include "GpioDefinitions.h"

namespace kiv::hal::gpio {


    template<GPIO_BANK BANK, bool EMULATION>
    class GPIO_Bank {

        std::array<GPIO_MODE, NUMBER_OF_PINS_IN_THE_BANK> cached_mode_configs{};
        std::array<GPIO_OUTPUT_TYPE, NUMBER_OF_PINS_IN_THE_BANK> cached_output_type_configs{};
        std::array<GPIO_OUTPUT_SPEED, NUMBER_OF_PINS_IN_THE_BANK> cached_output_speed_configs{};
        std::array<GPIO_PULL_UP_DOWN_CONFIG, NUMBER_OF_PINS_IN_THE_BANK> cached_pullup_pulldown_configs{};


        std::optional<uint32_t> input_data_register_cache;

        EmulationChecks<EMULATION> emulation_checks{};


        template<GPIO_HW_Registers REG>
        void write_hw_register(uint32_t value) {
            volatile uint32_t *ptr = (uint32_t *) get_bank_adr(BANK) + get_register_offset(REG);

            std::printf("%s write_hw_register %p : 0x%08x\n",EMULATION ? "Emulation":"Real", ptr, value);

            if constexpr (EMULATION == false) {
                *ptr = value;
            }else {
                emulation_checks.bank_register_data.at(static_cast<uint8_t>(REG)) = value;
            }
        }

        template<GPIO_HW_Registers REG>
        uint32_t read_hw_register() {
            volatile uint32_t *ptr = (uint32_t *) get_bank_adr(BANK) + get_register_offset(REG);

            uint32_t value{};
            if constexpr (EMULATION == false) {
                value = *ptr;
            }else {
                value = emulation_checks.bank_register_data.at(static_cast<uint8_t>(REG));
            }

            std::printf("%s read_hw_register %p : 0x%08x\n",EMULATION ? "Emulation":"Real", ptr, value);


            return value;
        }

    public:

        template<GPIO_OUTPUT_TYPE OUTPUT_TYPE = GPIO_OUTPUT_TYPE::PUSH_PULL, GPIO_OUTPUT_SPEED OUTPUT_SPEED = GPIO_OUTPUT_SPEED::LOW, GPIO_PULL_UP_DOWN_CONFIG PULL_UP_DOWN_CONFIG = GPIO_PULL_UP_DOWN_CONFIG::NO_PULLUP_OR_PULLDOWN>
        void agg_configure_pin_as_GP_output(std::initializer_list<GPIO_PIN> pin_list) {

            for (const GPIO_PIN & pin: pin_list) {
                cached_mode_configs.at(static_cast<unsigned>(pin)) = GPIO_MODE::GP_OUTPUT;
                cached_output_type_configs.at(static_cast<unsigned>(pin)) = OUTPUT_TYPE;
                cached_output_speed_configs.at(static_cast<unsigned>(pin)) = OUTPUT_SPEED;
                cached_pullup_pulldown_configs.at(static_cast<unsigned>(pin)) = PULL_UP_DOWN_CONFIG;
            }

        }


        template<GPIO_PULL_UP_DOWN_CONFIG PULL_UP_DOWN_CONFIG = GPIO_PULL_UP_DOWN_CONFIG::NO_PULLUP_OR_PULLDOWN>
        void agg_configure_pin_as_GP_input(std::initializer_list<GPIO_PIN> pin_list) {

            for (const GPIO_PIN & pin: pin_list) {
                cached_mode_configs.at(static_cast<unsigned>(pin)) = GPIO_MODE::INPUT;
                cached_output_type_configs.at(static_cast<unsigned>(pin)) = GPIO_OUTPUT_TYPE::PUSH_PULL; // Not used but better to set instead of unknown
                cached_output_speed_configs.at(static_cast<unsigned>(pin)) = GPIO_OUTPUT_SPEED::LOW;  // Not used but better to set instead of unknown
                cached_pullup_pulldown_configs.at(static_cast<unsigned>(pin)) = PULL_UP_DOWN_CONFIG;
            }


        }

        void agg_commit_cached_config_to_hw() {

            {
                uint32_t mode_configs{};

                for (unsigned int i = 0; i < NUMBER_OF_PINS_IN_THE_BANK; i++) {
                    mode_configs |= static_cast<unsigned>(cached_mode_configs.at(i)) << 2*i;
                }

                write_hw_register<GPIO_HW_Registers::MODE_CONFIG>(mode_configs);
            }

            {
                uint32_t output_type_configs{};

                for (unsigned int i = 0; i < NUMBER_OF_PINS_IN_THE_BANK; i++) {
                    output_type_configs |= static_cast<unsigned>(cached_output_type_configs.at(i)) << i;
                }

                write_hw_register<GPIO_HW_Registers::OUTPUT_TYPE_CONFIG>(output_type_configs);
            }

            {
                uint32_t output_speed_configs{};

                for (unsigned int i = 0; i < NUMBER_OF_PINS_IN_THE_BANK; i++) {
                    output_speed_configs |= static_cast<unsigned>(cached_output_speed_configs.at(i)) << 2*i;
                }

                write_hw_register<GPIO_HW_Registers::OUTPUT_SPEED_CONFIG>(output_speed_configs);
            }

            {
                uint32_t pullup_pulldown_configs{};

                for (unsigned int i = 0; i < NUMBER_OF_PINS_IN_THE_BANK; i++) {
                    pullup_pulldown_configs |= static_cast<unsigned>(cached_pullup_pulldown_configs.at(i)) << 2*i;
                }

                write_hw_register<GPIO_HW_Registers::PULLUP_PULLDOWN_CONFIG>(pullup_pulldown_configs);
            }

            if constexpr (EMULATION) {
                emulation_checks.resolve_register_data();
            }


        }

        template<GPIO_PIN GPIO_PIN, bool cached = false>
        bool read_pin() {

            if constexpr (cached) {
                if (input_data_register_cache) {
                    return input_data_register_cache.value() & get_pin_mask(GPIO_PIN);
                }
            }

            input_data_register_cache = read_hw_register<GPIO_HW_Registers::INPUT_DATA>();
            return input_data_register_cache.value() & get_pin_mask(GPIO_PIN);


        }

        template<GPIO_PIN PIN>
        void set_pin_value(bool value) {

            if (value) {
                set_pin<PIN>();
            }else {
                reset_pin<PIN>();
            }
        }

        template<GPIO_PIN GPIO_PIN>
        void set_pin() {
            write_hw_register<GPIO_HW_Registers::BITSET_RESET>(get_pin_mask(GPIO_PIN));
        }

        template<GPIO_PIN GPIO_PIN>
        void reset_pin() {
            write_hw_register<GPIO_HW_Registers::BITSET_RESET>(get_pin_mask(GPIO_PIN) << 16u);
        }
    };
}


#endif //STM_GPIO_CPP_DEN_HALGPIO_H
