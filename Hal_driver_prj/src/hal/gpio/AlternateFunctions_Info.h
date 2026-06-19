//
// Created by Kivanc Gunalp on 17.06.2026.
//

#ifndef HAL_DRIVER_PRJ_ALTERNATEFUNCTIONS_INFO_H
#define HAL_DRIVER_PRJ_ALTERNATEFUNCTIONS_INFO_H

#include <array>
#include <string_view>
#include "GpioDefinitions.h"

// Source: STM32F429 datasheet Table 12 — Alternate function mapping
// Each cell is [pin_number][af_number] → peripheral name.
// Empty string_view means "this AF is not available on this pin".
// Add entries here as new peripherals are brought up.

namespace kiv::hal::gpio {

using AF_Row   = std::array<std::string_view, 16>;  // AF0–AF15
using AF_Table = std::array<AF_Row, 16>;             // PIN_0–PIN_15

// ---- BANK A ----
static constexpr AF_Table bank_a_af_table = {{
    // PIN_0
    { "MCO",         "TIM2_CH1",    "TIM5_CH1",    "TIM8_ETR",    "",            "",            "",            "USART2_CTS",  "UART4_TX",    "",            "",            "ETH_MII_CRS", "",            "",            "",            "EVENTOUT" },
    // PIN_1
    { "",            "TIM2_CH2",    "TIM5_CH2",    "",            "",            "",            "",            "USART2_RTS",  "UART4_RX",    "",            "",            "ETH_MII_RX_CLK","",          "",            "",            "EVENTOUT" },
    // PIN_2
    { "",            "TIM2_CH3",    "TIM5_CH3",    "TIM9_CH1",    "",            "",            "",            "USART2_TX",   "",            "",            "",            "ETH_MDIO",    "",            "",            "",            "EVENTOUT" },
    // PIN_3
    { "",            "TIM2_CH4",    "TIM5_CH4",    "TIM9_CH2",    "",            "",            "",            "USART2_RX",   "",            "",            "OTG_HS_ULPI_D0","ETH_MII_COL","",           "",            "",            "EVENTOUT" },
    // PIN_4
    { "",            "",            "",            "",            "",            "SPI1_NSS",    "SPI3_NSS",    "USART2_CK",   "",            "",            "",            "",            "OTG_HS_SOF",  "DCMI_HSYNC",  "",            "EVENTOUT" },
    // PIN_5
    { "",            "TIM2_CH1",    "",            "TIM8_CH1N",   "",            "SPI1_SCK",    "",            "",            "",            "",            "OTG_HS_ULPI_CK","",          "",            "",            "",            "EVENTOUT" },
    // PIN_6
    { "",            "TIM1_BKIN",   "TIM3_CH1",    "TIM8_BKIN",   "",            "SPI1_MISO",   "",            "",            "",            "TIM13_CH1",   "OTG_FS_SOF",  "",            "",            "DCMI_PIXCLK", "",            "EVENTOUT" },
    // PIN_7
    { "",            "TIM1_CH1N",   "TIM3_CH2",    "TIM8_CH1N",   "",            "SPI1_MOSI",   "",            "",            "",            "TIM14_CH1",   "",            "ETH_MII_RX_DV","",          "",            "",            "EVENTOUT" },
    // PIN_8
    { "MCO1",        "TIM1_CH1",    "",            "",            "I2C3_SCL",    "",            "",            "USART1_CK",   "",            "",            "OTG_FS_SOF",  "",            "",            "",            "",            "EVENTOUT" },
    // PIN_9
    { "",            "TIM1_CH2",    "",            "",            "I2C3_SMBA",   "",            "",            "USART1_TX",   "",            "",            "",            "",            "",            "DCMI_D0",     "",            "EVENTOUT" },
    // PIN_10
    { "",            "TIM1_CH3",    "",            "",            "",            "",            "",            "USART1_RX",   "",            "",            "OTG_FS_ID",   "",            "",            "DCMI_D1",     "",            "EVENTOUT" },
    // PIN_11
    { "",            "TIM1_CH4",    "",            "",            "",            "",            "",            "USART1_CTS",  "",            "CAN1_RX",     "OTG_FS_DM",   "",            "",            "",            "",            "EVENTOUT" },
    // PIN_12
    { "",            "TIM1_ETR",    "",            "",            "",            "",            "",            "USART1_RTS",  "",            "CAN1_TX",     "OTG_FS_DP",   "",            "",            "",            "",            "EVENTOUT" },
    // PIN_13
    { "JTMS_SWDIO",  "",            "",            "",            "",            "",            "",            "",            "",            "",            "",            "",            "",            "",            "",            "EVENTOUT" },
    // PIN_14
    { "JTCK_SWCLK",  "",            "",            "",            "",            "",            "",            "",            "",            "",            "",            "",            "",            "",            "",            "EVENTOUT" },
    // PIN_15
    { "JTDI",        "TIM2_CH1",    "",            "",            "",            "SPI1_NSS",    "SPI3_NSS",    "",            "",            "",            "",            "",            "",            "",            "",            "EVENTOUT" },
}};

// ---- Peripheral function names ----
// Every unique peripheral function that appears in any bank AF table.
// Adding a new bank's table: extend this enum and get_peripheral_str() together.

enum class PeripheralFunction : uint8_t {
    // Clock outputs
    MCO, MCO1,
    // TIM1
    TIM1_BKIN, TIM1_CH1, TIM1_CH1N, TIM1_CH2, TIM1_CH3, TIM1_CH4, TIM1_ETR,
    // TIM2
    TIM2_CH1, TIM2_CH2, TIM2_CH3, TIM2_CH4,
    // TIM3
    TIM3_CH1, TIM3_CH2,
    // TIM5
    TIM5_CH1, TIM5_CH2, TIM5_CH3, TIM5_CH4,
    // TIM8
    TIM8_BKIN, TIM8_CH1N, TIM8_ETR,
    // TIM9, TIM13, TIM14
    TIM9_CH1, TIM9_CH2, TIM13_CH1, TIM14_CH1,
    // SPI1
    SPI1_MISO, SPI1_MOSI, SPI1_NSS, SPI1_SCK,
    // SPI3
    SPI3_NSS,
    // I2C3
    I2C3_SCL, I2C3_SMBA,
    // USART1
    USART1_CK, USART1_CTS, USART1_RTS, USART1_RX, USART1_TX,
    // USART2
    USART2_CK, USART2_CTS, USART2_RTS, USART2_RX, USART2_TX,
    // UART4
    UART4_RX, UART4_TX,
    // CAN1
    CAN1_RX, CAN1_TX,
    // OTG FS
    OTG_FS_DM, OTG_FS_DP, OTG_FS_ID, OTG_FS_SOF,
    // OTG HS
    OTG_HS_SOF, OTG_HS_ULPI_CK, OTG_HS_ULPI_D0,
    // Ethernet
    ETH_MDIO, ETH_MII_COL, ETH_MII_CRS, ETH_MII_RX_CLK, ETH_MII_RX_DV,
    // DCMI
    DCMI_D0, DCMI_D1, DCMI_HSYNC, DCMI_PIXCLK,
    // Debug
    JTCK_SWCLK, JTDI, JTMS_SWDIO,
    // System
    EVENTOUT,
};

// Maps PeripheralFunction enum → the exact string stored in the AF tables.
// Must stay in sync with the table contents.
consteval std::string_view get_peripheral_str(PeripheralFunction pf) {
    switch (pf) {
        case PeripheralFunction::MCO:             return "MCO";
        case PeripheralFunction::MCO1:            return "MCO1";
        case PeripheralFunction::TIM1_BKIN:       return "TIM1_BKIN";
        case PeripheralFunction::TIM1_CH1:        return "TIM1_CH1";
        case PeripheralFunction::TIM1_CH1N:       return "TIM1_CH1N";
        case PeripheralFunction::TIM1_CH2:        return "TIM1_CH2";
        case PeripheralFunction::TIM1_CH3:        return "TIM1_CH3";
        case PeripheralFunction::TIM1_CH4:        return "TIM1_CH4";
        case PeripheralFunction::TIM1_ETR:        return "TIM1_ETR";
        case PeripheralFunction::TIM2_CH1:        return "TIM2_CH1";
        case PeripheralFunction::TIM2_CH2:        return "TIM2_CH2";
        case PeripheralFunction::TIM2_CH3:        return "TIM2_CH3";
        case PeripheralFunction::TIM2_CH4:        return "TIM2_CH4";
        case PeripheralFunction::TIM3_CH1:        return "TIM3_CH1";
        case PeripheralFunction::TIM3_CH2:        return "TIM3_CH2";
        case PeripheralFunction::TIM5_CH1:        return "TIM5_CH1";
        case PeripheralFunction::TIM5_CH2:        return "TIM5_CH2";
        case PeripheralFunction::TIM5_CH3:        return "TIM5_CH3";
        case PeripheralFunction::TIM5_CH4:        return "TIM5_CH4";
        case PeripheralFunction::TIM8_BKIN:       return "TIM8_BKIN";
        case PeripheralFunction::TIM8_CH1N:       return "TIM8_CH1N";
        case PeripheralFunction::TIM8_ETR:        return "TIM8_ETR";
        case PeripheralFunction::TIM9_CH1:        return "TIM9_CH1";
        case PeripheralFunction::TIM9_CH2:        return "TIM9_CH2";
        case PeripheralFunction::TIM13_CH1:       return "TIM13_CH1";
        case PeripheralFunction::TIM14_CH1:       return "TIM14_CH1";
        case PeripheralFunction::SPI1_MISO:       return "SPI1_MISO";
        case PeripheralFunction::SPI1_MOSI:       return "SPI1_MOSI";
        case PeripheralFunction::SPI1_NSS:        return "SPI1_NSS";
        case PeripheralFunction::SPI1_SCK:        return "SPI1_SCK";
        case PeripheralFunction::SPI3_NSS:        return "SPI3_NSS";
        case PeripheralFunction::I2C3_SCL:        return "I2C3_SCL";
        case PeripheralFunction::I2C3_SMBA:       return "I2C3_SMBA";
        case PeripheralFunction::USART1_CK:       return "USART1_CK";
        case PeripheralFunction::USART1_CTS:      return "USART1_CTS";
        case PeripheralFunction::USART1_RTS:      return "USART1_RTS";
        case PeripheralFunction::USART1_RX:       return "USART1_RX";
        case PeripheralFunction::USART1_TX:       return "USART1_TX";
        case PeripheralFunction::USART2_CK:       return "USART2_CK";
        case PeripheralFunction::USART2_CTS:      return "USART2_CTS";
        case PeripheralFunction::USART2_RTS:      return "USART2_RTS";
        case PeripheralFunction::USART2_RX:       return "USART2_RX";
        case PeripheralFunction::USART2_TX:       return "USART2_TX";
        case PeripheralFunction::UART4_RX:        return "UART4_RX";
        case PeripheralFunction::UART4_TX:        return "UART4_TX";
        case PeripheralFunction::CAN1_RX:         return "CAN1_RX";
        case PeripheralFunction::CAN1_TX:         return "CAN1_TX";
        case PeripheralFunction::OTG_FS_DM:       return "OTG_FS_DM";
        case PeripheralFunction::OTG_FS_DP:       return "OTG_FS_DP";
        case PeripheralFunction::OTG_FS_ID:       return "OTG_FS_ID";
        case PeripheralFunction::OTG_FS_SOF:      return "OTG_FS_SOF";
        case PeripheralFunction::OTG_HS_SOF:      return "OTG_HS_SOF";
        case PeripheralFunction::OTG_HS_ULPI_CK:  return "OTG_HS_ULPI_CK";
        case PeripheralFunction::OTG_HS_ULPI_D0:  return "OTG_HS_ULPI_D0";
        case PeripheralFunction::ETH_MDIO:        return "ETH_MDIO";
        case PeripheralFunction::ETH_MII_COL:     return "ETH_MII_COL";
        case PeripheralFunction::ETH_MII_CRS:     return "ETH_MII_CRS";
        case PeripheralFunction::ETH_MII_RX_CLK:  return "ETH_MII_RX_CLK";
        case PeripheralFunction::ETH_MII_RX_DV:   return "ETH_MII_RX_DV";
        case PeripheralFunction::DCMI_D0:         return "DCMI_D0";
        case PeripheralFunction::DCMI_D1:         return "DCMI_D1";
        case PeripheralFunction::DCMI_HSYNC:      return "DCMI_HSYNC";
        case PeripheralFunction::DCMI_PIXCLK:     return "DCMI_PIXCLK";
        case PeripheralFunction::JTCK_SWCLK:      return "JTCK_SWCLK";
        case PeripheralFunction::JTDI:            return "JTDI";
        case PeripheralFunction::JTMS_SWDIO:      return "JTMS_SWDIO";
        case PeripheralFunction::EVENTOUT:        return "EVENTOUT";
    }
    return "";
}

// ---- Per-peripheral GPIO output defaults ----

struct PeripheralFunctionConfig {
    GPIO_OUTPUT_TYPE  output_type;
    GPIO_OUTPUT_SPEED output_speed;
};

// Returns the recommended GPIO output type and speed for a peripheral function.
// These are the correct defaults from the STM32F429 reference manual:
//   - I2C      : open-drain (protocol requirement — pull-up resistors provide the high level)
//   - Ethernet : very-high speed (MII signals can reach 25/50 MHz)
//   - USB OTG  : high speed
//   - SPI      : high speed (clock can reach tens of MHz)
//   - CAN      : medium speed (≤ 1 Mbps nominal)
//   - UART     : low speed (sufficient up to ~10 Mbaud with typical MCU clocks)
//   - MCO      : very-high speed (clock output signal)
//   - Everything else defaults to push-pull, low speed.
consteval PeripheralFunctionConfig get_peripheral_defaults(PeripheralFunction pf) {
    switch (pf) {
        // I2C — open-drain is a hard protocol requirement
        case PeripheralFunction::I2C3_SCL:
        case PeripheralFunction::I2C3_SMBA:
            return { GPIO_OUTPUT_TYPE::OPEN_DRAIN, GPIO_OUTPUT_SPEED::LOW };

        // Ethernet — signals up to 50 MHz (RMII) or 25 MHz (MII)
        case PeripheralFunction::ETH_MDIO:
        case PeripheralFunction::ETH_MII_COL:
        case PeripheralFunction::ETH_MII_CRS:
        case PeripheralFunction::ETH_MII_RX_CLK:
        case PeripheralFunction::ETH_MII_RX_DV:
            return { GPIO_OUTPUT_TYPE::PUSH_PULL, GPIO_OUTPUT_SPEED::VERY_HIGH };

        // USB OTG
        case PeripheralFunction::OTG_FS_DM:
        case PeripheralFunction::OTG_FS_DP:
        case PeripheralFunction::OTG_FS_ID:
        case PeripheralFunction::OTG_FS_SOF:
        case PeripheralFunction::OTG_HS_SOF:
        case PeripheralFunction::OTG_HS_ULPI_CK:
        case PeripheralFunction::OTG_HS_ULPI_D0:
            return { GPIO_OUTPUT_TYPE::PUSH_PULL, GPIO_OUTPUT_SPEED::HIGH };

        // SPI — clock and data lines need to keep up with peripheral clock
        case PeripheralFunction::SPI1_SCK:
        case PeripheralFunction::SPI1_MOSI:
        case PeripheralFunction::SPI1_MISO:
        case PeripheralFunction::SPI1_NSS:
        case PeripheralFunction::SPI3_NSS:
            return { GPIO_OUTPUT_TYPE::PUSH_PULL, GPIO_OUTPUT_SPEED::HIGH };

        // CAN — 1 Mbps max, medium is sufficient
        case PeripheralFunction::CAN1_TX:
        case PeripheralFunction::CAN1_RX:
            return { GPIO_OUTPUT_TYPE::PUSH_PULL, GPIO_OUTPUT_SPEED::MEDIUM };

        // UART / USART — low speed sufficient at typical baud rates (≤ 10 Mbaud)
        case PeripheralFunction::USART1_TX: case PeripheralFunction::USART1_RX:
        case PeripheralFunction::USART1_CK: case PeripheralFunction::USART1_CTS:
        case PeripheralFunction::USART1_RTS:
        case PeripheralFunction::USART2_TX: case PeripheralFunction::USART2_RX:
        case PeripheralFunction::USART2_CK: case PeripheralFunction::USART2_CTS:
        case PeripheralFunction::USART2_RTS:
        case PeripheralFunction::UART4_TX:  case PeripheralFunction::UART4_RX:
            return { GPIO_OUTPUT_TYPE::PUSH_PULL, GPIO_OUTPUT_SPEED::LOW };

        // MCO — clock output, needs very-high speed
        case PeripheralFunction::MCO:
        case PeripheralFunction::MCO1:
            return { GPIO_OUTPUT_TYPE::PUSH_PULL, GPIO_OUTPUT_SPEED::VERY_HIGH };

        // DCMI — pixel clock can be high
        case PeripheralFunction::DCMI_D0:   case PeripheralFunction::DCMI_D1:
        case PeripheralFunction::DCMI_HSYNC: case PeripheralFunction::DCMI_PIXCLK:
            return { GPIO_OUTPUT_TYPE::PUSH_PULL, GPIO_OUTPUT_SPEED::HIGH };

        // Everything else: safe conservative default
        default:
            return { GPIO_OUTPUT_TYPE::PUSH_PULL, GPIO_OUTPUT_SPEED::LOW };
    }
}

// Returns true for peripheral functions whose protocol mandates open-drain output.
// Used to static_assert against accidental PUSH_PULL override on I2C lines.
consteval bool is_open_drain_required(PeripheralFunction pf) {
    switch (pf) {
        case PeripheralFunction::I2C3_SCL:
        case PeripheralFunction::I2C3_SMBA:
            return true;
        default:
            return false;
    }
}

// ---- Lookup and validation ----

// Returns the peripheral name for the given bank/pin/AF combination,
// or an empty string_view if the AF is not available on that pin.
consteval std::string_view get_af_name(GPIO_BANK bank, GPIO_PIN pin, AlternateFunction af) {
    switch (bank) {
        case GPIO_BANK::BANK_A:
            return bank_a_af_table
                       .at(static_cast<uint8_t>(pin))
                       .at(static_cast<uint8_t>(af));
        default:
            return "";  // other banks: add their tables here
    }
}

// Returns true if the AF number is mapped to a peripheral on the given bank/pin.
consteval bool is_af_available(GPIO_BANK bank, GPIO_PIN pin, AlternateFunction af) {
    return !get_af_name(bank, pin, af).empty();
}

// Returns true if the PeripheralFunction is available on the given bank/pin.
consteval bool is_peripheral_available(GPIO_BANK bank, GPIO_PIN pin, PeripheralFunction pf) {
    const std::string_view target = get_peripheral_str(pf);
    const uint8_t pin_idx = static_cast<uint8_t>(pin);
    switch (bank) {
        case GPIO_BANK::BANK_A:
            for (uint8_t i = 0; i < 16; ++i) {
                if (bank_a_af_table[pin_idx][i] == target) return true;
            }
            return false;
        default:
            return false;  // other banks: add cases here
    }
}

// Returns the AF number for a PeripheralFunction on the given bank/pin.
// Only call this after is_peripheral_available() returns true (enforced via static_assert).
consteval AlternateFunction find_af_for_peripheral(GPIO_BANK bank, GPIO_PIN pin, PeripheralFunction pf) {
    const std::string_view target = get_peripheral_str(pf);
    const uint8_t pin_idx = static_cast<uint8_t>(pin);
    switch (bank) {
        case GPIO_BANK::BANK_A:
            for (uint8_t i = 0; i < 16; ++i) {
                if (bank_a_af_table[pin_idx][i] == target)
                    return static_cast<AlternateFunction>(i);
            }
            break;
        default: break;
    }
    return AlternateFunction::AF_0;  // unreachable if guarded by is_peripheral_available()
}

} // namespace kiv::hal::gpio

#endif // HAL_DRIVER_PRJ_ALTERNATEFUNCTIONS_INFO_H
