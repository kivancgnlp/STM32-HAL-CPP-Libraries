//
// Created by Kivanc Gunalp on 14.06.2026.
//

#ifndef STM_GPIO_CPP_DEN_HALGPIO_H
#define STM_GPIO_CPP_DEN_HALGPIO_H
#include <array>
#include <cstdint>
#include <cstdio>
#include <optional>

namespace kiv::hal::gpio {

    static constexpr uint32_t PERIPH_BASE{0x40000000UL};
    static constexpr uint32_t APB1PERIPH_BASE = PERIPH_BASE;

    enum class GPIO_BANK : uint8_t {
        BANK_A = 0,
        BANK_B,
        BANK_C,
        BANK_D,
        BANK_E,
        BANK_F,
        BANK_G,
        BANK_H,
        BANK_I,
        BANK_J,
        BANK_K,
    };

    enum class GPIO_PIN : uint8_t {
        PIN_0 = 0,
        PIN_1,
        PIN_2,
        PIN_3,
        PIN_4,
        PIN_5,
        PIN_6,
        PIN_7,
        PIN_8,
        PIN_9,
        PIN_10,
        PIN_11,
        PIN_12,
        PIN_13,
        PIN_14,
        PIN_15,
    };

    enum class GPIO_HW_Registers : uint8_t {
        MODE_CONFIG = 0, /*!< GPIO port mode register,               Address offset: 0x00      */
        OUTPUT_TYPE_CONFIG, /*!< GPIO port output type register,        Address offset: 0x04      */
        OUTPUT_SPEED_CONFIG, /*!< GPIO port output speed register,       Address offset: 0x08      */
        PULLUP_PULLDOWN_CONFIG, /*!< GPIO port pull-up/pull-down register,  Address offset: 0x0C */
        INPUT_DATA, /*!< GPIO port input data register,         Address offset: 0x10      */
        OUTPUT_DATA, /*!< GPIO port output data register,        Address offset: 0x14      */
        BITSET_RESET, /*!< GPIO port bit set/reset register,      Address offset: 0x18      */
        LOCK, /*!< GPIO port configuration lock register, Address offset: 0x1C      */
        ALTERNATE_FUNCTION_CONFIG_L, /*!< GPIO alternate function registers,     Address offset: 0x20-0x24 */
        ALTERNATE_FUNCTION_CONFIG_H,
    };

    enum class GPIO_MODE : uint8_t {
        INPUT = 0,
        GP_OUTPUT,
        ALTERNATE_FUNCTION,
        ANALOG,
    };

    enum class GPIO_OUTPUT_TYPE : uint8_t {
        PUSH_PULL = 0,
        OPEN_DRAIN,
    };

    enum class GPIO_OUTPUT_SPEED : uint8_t {
        LOW = 0,
        MEDIUM,
        HIGH,
        VERY_HIGH,
    };

    enum class GPIO_PULL_UP_DOWN_CONFIG : uint8_t {
        NO_PULLUP_OR_PULLDOWN = 0,
        PULL_UP,
        PULL_DOWN,
        RESERVED,
    };

    consteval uint32_t get_register_offset(GPIO_HW_Registers reg) {
        return static_cast<uint32_t>(reg) * 4;
    }

    consteval uint32_t get_bank_adr(GPIO_BANK bank) {
        return APB1PERIPH_BASE + static_cast<uint32_t>(bank) * 0x400UL;
    }

    consteval uint32_t get_pin_mask(GPIO_PIN pin) {
        return 1 << static_cast<uint8_t>(pin);
    }

    template <bool EMULATION>
    struct EmulationData {

    };

    template <>
    struct EmulationData<true> {
        std::array<uint32_t, 10> bank_register_data;
        uint32_t pin_states;

    };


    template<GPIO_BANK BANK, bool EMULATION>
    class GPIO_Bank {
        static constexpr uint8_t NUMBER_OF_PINS_IN_THE_BANK{16};

        std::array<GPIO_MODE, NUMBER_OF_PINS_IN_THE_BANK> cached_mode_configs{};
        std::array<GPIO_OUTPUT_TYPE, NUMBER_OF_PINS_IN_THE_BANK> cached_output_type_configs{};
        std::array<GPIO_OUTPUT_SPEED, NUMBER_OF_PINS_IN_THE_BANK> cached_output_speed_configs{};
        std::array<GPIO_PULL_UP_DOWN_CONFIG, NUMBER_OF_PINS_IN_THE_BANK> cached_pullup_pulldown_configs{};


        std::optional<uint32_t> input_data_register_cache;

        EmulationData<EMULATION> emulation_data{};


        template<GPIO_HW_Registers REG>
        void write_hw_register(uint32_t value) {
            volatile uint32_t *ptr = (uint32_t *) get_bank_adr(BANK) + get_register_offset(REG);

            std::printf("%s write_hw_register %p : 0x%08x\n",EMULATION ? "Emulation":"Real", ptr, value);

            if constexpr (EMULATION == false) {
                *ptr = value;
            }else {
                emulation_data.bank_register_data.at(static_cast<uint8_t>(REG)) = value;
            }
        }

        template<GPIO_HW_Registers REG>
        uint32_t read_hw_register() {
            volatile uint32_t *ptr = (uint32_t *) get_bank_adr(BANK) + get_register_offset(REG);

            uint32_t value{};
            if constexpr (EMULATION == false) {
                value = *ptr;
            }else {
                value = emulation_data.bank_register_data.at(static_cast<uint8_t>(REG));
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
