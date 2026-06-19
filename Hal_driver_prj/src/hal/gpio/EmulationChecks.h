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

    std::uint32_t pin_states{};

    std::array<GPIO_MODE,              NUMBER_OF_PINS_IN_THE_BANK> resolved_mode_configs{};
    std::array<GPIO_OUTPUT_TYPE,       NUMBER_OF_PINS_IN_THE_BANK> resolved_output_type_configs{};
    std::array<GPIO_OUTPUT_SPEED,      NUMBER_OF_PINS_IN_THE_BANK> resolved_output_speed_configs{};
    std::array<GPIO_PULL_UP_DOWN_CONFIG, NUMBER_OF_PINS_IN_THE_BANK> resolved_pullup_pulldown_configs{};
    std::array<AlternateFunction,      NUMBER_OF_PINS_IN_THE_BANK> resolved_af_configs{};

    // Returns the array index for a register (same as enum ordinal, not the byte offset).
    static constexpr uint8_t reg_idx(GPIO_HW_Registers r) { return static_cast<uint8_t>(r); }

    // ---- 2-bit-per-pin field resolvers (MODER, OSPEEDR, PUPDR) ----

    template <unsigned... N>
    void resolve_mode_configs(uint32_t reg, std::integer_sequence<unsigned, N...>) {
        using kiv::utils::get_bit_field;
        resolved_mode_configs = {static_cast<GPIO_MODE>(get_bit_field<N*2, 2>(reg))...};
    }

    template <unsigned... N>
    void resolve_output_type_configs(uint32_t reg, std::integer_sequence<unsigned, N...>) {
        using kiv::utils::get_bit_field;
        resolved_output_type_configs = {static_cast<GPIO_OUTPUT_TYPE>(get_bit_field<N, 1>(reg))...};
    }

    template <unsigned... N>
    void resolve_output_speed_configs(uint32_t reg, std::integer_sequence<unsigned, N...>) {
        using kiv::utils::get_bit_field;
        resolved_output_speed_configs = {static_cast<GPIO_OUTPUT_SPEED>(get_bit_field<N*2, 2>(reg))...};
    }

    template <unsigned... N>
    void resolve_pullup_pulldown_configs(uint32_t reg, std::integer_sequence<unsigned, N...>) {
        using kiv::utils::get_bit_field;
        resolved_pullup_pulldown_configs = {static_cast<GPIO_PULL_UP_DOWN_CONFIG>(get_bit_field<N*2, 2>(reg))...};
    }

    // ---- 4-bit-per-pin AF resolvers (AFRL = pins 0-7, AFRH = pins 8-15) ----

    template <unsigned... N>
    void resolve_af_low(uint32_t afrl, std::integer_sequence<unsigned, N...>) {
        using kiv::utils::get_bit_field;
        ((resolved_af_configs[N] = static_cast<AlternateFunction>(get_bit_field<N*4, 4>(afrl))), ...);
    }

    template <unsigned... N>
    void resolve_af_high(uint32_t afrh, std::integer_sequence<unsigned, N...>) {
        using kiv::utils::get_bit_field;
        ((resolved_af_configs[N + 8] = static_cast<AlternateFunction>(get_bit_field<N*4, 4>(afrh))), ...);
    }

public:
    std::array<uint32_t, 10> bank_register_data{};

    // Parse all GPIO registers into resolved_* arrays, then print a summary table.
    // Called automatically by GPIO_Bank::agg_commit_cached_config_to_hw() in emulation mode.
    void resolve_register_data() {
        constexpr auto seq16 = std::make_integer_sequence<unsigned, NUMBER_OF_PINS_IN_THE_BANK>();
        constexpr auto seq8  = std::make_integer_sequence<unsigned, 8>();

        resolve_mode_configs(
            bank_register_data.at(reg_idx(GPIO_HW_Registers::MODE_CONFIG)), seq16);
        resolve_output_type_configs(
            bank_register_data.at(reg_idx(GPIO_HW_Registers::OUTPUT_TYPE_CONFIG)), seq16);
        resolve_output_speed_configs(
            bank_register_data.at(reg_idx(GPIO_HW_Registers::OUTPUT_SPEED_CONFIG)), seq16);
        resolve_pullup_pulldown_configs(
            bank_register_data.at(reg_idx(GPIO_HW_Registers::PULLUP_PULLDOWN_CONFIG)), seq16);

        resolve_af_low(
            bank_register_data.at(reg_idx(GPIO_HW_Registers::ALTERNATE_FUNCTION_CONFIG_L)), seq8);
        resolve_af_high(
            bank_register_data.at(reg_idx(GPIO_HW_Registers::ALTERNATE_FUNCTION_CONFIG_H)), seq8);

        print_resolved_config();
    }

    // Print a table of all 16 pins with their decoded register fields.
    // OType/Speed are shown as "-" for INPUT and ANALOG pins (fields don't apply).
    // AF is shown as "-" unless the pin is in ALTERNATE_FUNCTION mode.
    void print_resolved_config() const {
        std::printf("---- GPIO Bank Emulation State ----\n");
        std::printf(" Pin  %-20s%-12s%-11s%-11s%s\n",
                    "Mode", "OType", "Speed", "Pull", "AF");
        std::printf(" ---  %-20s%-12s%-11s%-11s%s\n",
                    "----", "-----", "-----", "----", "--");

        for (unsigned i = 0; i < NUMBER_OF_PINS_IN_THE_BANK; i++) {
            const GPIO_MODE              mode  = resolved_mode_configs.at(i);
            const GPIO_OUTPUT_TYPE       otype = resolved_output_type_configs.at(i);
            const GPIO_OUTPUT_SPEED      speed = resolved_output_speed_configs.at(i);
            const GPIO_PULL_UP_DOWN_CONFIG pull = resolved_pullup_pulldown_configs.at(i);
            const AlternateFunction      af    = resolved_af_configs.at(i);

            const bool has_output = (mode == GPIO_MODE::GP_OUTPUT ||
                                     mode == GPIO_MODE::ALTERNATE_FUNCTION);
            const bool has_af     = (mode == GPIO_MODE::ALTERNATE_FUNCTION);

            std::printf(" %3u  %-20s%-12s%-11s%-11s%s\n",
                i,
                get_mode_config_txt(mode).data(),
                has_output ? get_output_type_txt(otype).data() : "-",
                has_output ? get_output_speed_txt(speed).data() : "-",
                get_pullup_pulldown_txt(pull).data(),
                has_af     ? get_af_txt(af).data()              : "-");
        }
        std::printf("------------------------------------\n");
    }
};

} // namespace kiv::hal::gpio

#endif //HAL_DRIVER_PRJ_EMULATIONCHECKS_H
