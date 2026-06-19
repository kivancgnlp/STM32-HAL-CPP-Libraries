//
// Created by Kiv on 6/16/2026.
//

#ifndef HAL_DRIVER_PRJ_GPIODEFINITIONS_H
#define HAL_DRIVER_PRJ_GPIODEFINITIONS_H
#include <string_view>
#include <cassert>

namespace kiv::hal::gpio {
    static constexpr uint32_t PERIPH_BASE{0x40000000UL};
    static constexpr uint32_t APB1PERIPH_BASE = PERIPH_BASE;
    static constexpr uint8_t NUMBER_OF_PINS_IN_THE_BANK{16};

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

    enum class AlternateFunction : uint8_t {
        AF_0 = 0,
        AF_1,
        AF_2,
        AF_3,
        AF_4,
        AF_5,
        AF_6,
        AF_7,
        AF_8,
        AF_9,
        AF_10,
        AF_11,
        AF_12,
        AF_13,
        AF_14,
        AF_15,
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

    enum class GPIO_MODE_CONFIG_INDICATOR : uint8_t {// This data type is only for software checks
        UNCONFIGURATED = 0,
        INPUT,
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

    constexpr std::string_view get_mode_config_txt(GPIO_MODE gpio_mode) {
        switch (gpio_mode) {
            case GPIO_MODE::INPUT: return "INPUT";
            case GPIO_MODE::GP_OUTPUT: return "GP_OUTPUT";
            case GPIO_MODE::ALTERNATE_FUNCTION: return "ALTERNATE_FUNCTION";
            case GPIO_MODE::ANALOG: return "ANALOG";
        }

        assert(false);

    }

    constexpr std::string_view get_mode_config_configuration_txt(GPIO_MODE_CONFIG_INDICATOR gpio_mode) {
        switch (gpio_mode) {
            case GPIO_MODE_CONFIG_INDICATOR::UNCONFIGURATED: return "UNCONFIGURATED";
                case GPIO_MODE_CONFIG_INDICATOR::INPUT: return "INPUT";
                case GPIO_MODE_CONFIG_INDICATOR::GP_OUTPUT: return "GP_OUTPUT";
                case GPIO_MODE_CONFIG_INDICATOR::ALTERNATE_FUNCTION: return "ALTERNATE_FUNCTION";
                case GPIO_MODE_CONFIG_INDICATOR::ANALOG: return "ANALOG";
        }

        assert(false);

    }

    constexpr std::string_view get_output_type_txt(GPIO_OUTPUT_TYPE t) {
        switch (t) {
            case GPIO_OUTPUT_TYPE::PUSH_PULL:  return "PUSH_PULL";
            case GPIO_OUTPUT_TYPE::OPEN_DRAIN: return "OPEN_DRAIN";
        }
        assert(false);
    }

    constexpr std::string_view get_output_speed_txt(GPIO_OUTPUT_SPEED s) {
        switch (s) {
            case GPIO_OUTPUT_SPEED::LOW:       return "LOW";
            case GPIO_OUTPUT_SPEED::MEDIUM:    return "MEDIUM";
            case GPIO_OUTPUT_SPEED::HIGH:      return "HIGH";
            case GPIO_OUTPUT_SPEED::VERY_HIGH: return "VERY_HIGH";
        }
        assert(false);
    }

    constexpr std::string_view get_pullup_pulldown_txt(GPIO_PULL_UP_DOWN_CONFIG p) {
        switch (p) {
            case GPIO_PULL_UP_DOWN_CONFIG::NO_PULLUP_OR_PULLDOWN: return "NONE";
            case GPIO_PULL_UP_DOWN_CONFIG::PULL_UP:               return "PULL_UP";
            case GPIO_PULL_UP_DOWN_CONFIG::PULL_DOWN:             return "PULL_DOWN";
            case GPIO_PULL_UP_DOWN_CONFIG::RESERVED:              return "RESERVED";
        }
        assert(false);
    }

    constexpr std::string_view get_af_txt(AlternateFunction af) {
        switch (af) {
            case AlternateFunction::AF_0:  return "AF_0";
            case AlternateFunction::AF_1:  return "AF_1";
            case AlternateFunction::AF_2:  return "AF_2";
            case AlternateFunction::AF_3:  return "AF_3";
            case AlternateFunction::AF_4:  return "AF_4";
            case AlternateFunction::AF_5:  return "AF_5";
            case AlternateFunction::AF_6:  return "AF_6";
            case AlternateFunction::AF_7:  return "AF_7";
            case AlternateFunction::AF_8:  return "AF_8";
            case AlternateFunction::AF_9:  return "AF_9";
            case AlternateFunction::AF_10: return "AF_10";
            case AlternateFunction::AF_11: return "AF_11";
            case AlternateFunction::AF_12: return "AF_12";
            case AlternateFunction::AF_13: return "AF_13";
            case AlternateFunction::AF_14: return "AF_14";
            case AlternateFunction::AF_15: return "AF_15";
        }
        assert(false);
    }
}

#endif //HAL_DRIVER_PRJ_GPIODEFINITIONS_H