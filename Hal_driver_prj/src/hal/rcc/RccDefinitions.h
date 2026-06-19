//
// Created by Kivanc Gunalp on 19.06.2026.
//

#ifndef HAL_DRIVER_PRJ_RCCDEFINITIONS_H
#define HAL_DRIVER_PRJ_RCCDEFINITIONS_H

#include <cstdint>
#include <cassert>
#include <string_view>

namespace kiv::hal::rcc {

// RCC peripheral base address (AHB1 bus, RM0090 §6.3.26)
static constexpr uint32_t RCC_BASE_ADDR{0x40023800UL};

// RCC registers used by this module, ordinal-indexed for the emulation array.
// Actual hardware byte offsets are resolved by get_rcc_byte_offset().
enum class RCC_Register : uint8_t {
    CR = 0,      // Control register
    PLLCFGR,     // PLL configuration register
    CFGR,        // Clock configuration register
    AHB1ENR,     // AHB1 peripheral clock enable register
    APB1ENR,     // APB1 peripheral clock enable register
    APB2ENR,     // APB2 peripheral clock enable register
    BDCR,        // Backup domain control register
    CSR,         // Control/status register
    NUM_REGISTERS
};

static constexpr uint8_t NUM_RCC_REGISTERS = static_cast<uint8_t>(RCC_Register::NUM_REGISTERS);

// Maps enum ordinals to their actual byte offsets within the RCC peripheral (RM0090 §6.3).
consteval uint32_t get_rcc_byte_offset(RCC_Register reg) {
    switch (reg) {
        case RCC_Register::CR:      return 0x00U;
        case RCC_Register::PLLCFGR: return 0x04U;
        case RCC_Register::CFGR:    return 0x08U;
        case RCC_Register::AHB1ENR: return 0x30U;
        case RCC_Register::APB1ENR: return 0x40U;
        case RCC_Register::APB2ENR: return 0x44U;
        case RCC_Register::BDCR:    return 0x70U;
        case RCC_Register::CSR:     return 0x74U;
        default:                    return 0xFFU;
    }
}

// ---- CR register bit positions (RM0090 §6.3.1) ----
static constexpr uint8_t CR_HSION_Pos  = 0;
static constexpr uint8_t CR_HSIRDY_Pos = 1;
static constexpr uint8_t CR_HSEON_Pos  = 16;
static constexpr uint8_t CR_HSERDY_Pos = 17;
static constexpr uint8_t CR_HSEBYP_Pos = 18;
static constexpr uint8_t CR_PLLON_Pos  = 24;
static constexpr uint8_t CR_PLLRDY_Pos = 25;

// ---- PLLCFGR register bit positions (RM0090 §6.3.2) ----
static constexpr uint8_t PLLCFGR_PLLM_Pos   = 0;
static constexpr uint8_t PLLCFGR_PLLN_Pos   = 6;
static constexpr uint8_t PLLCFGR_PLLP_Pos   = 16;
static constexpr uint8_t PLLCFGR_PLLSRC_Pos = 22;
static constexpr uint8_t PLLCFGR_PLLQ_Pos   = 24;

// ---- CFGR register bit positions (RM0090 §6.3.3) ----
static constexpr uint8_t CFGR_SW_Pos    = 0;  // [1:0] system clock switch
static constexpr uint8_t CFGR_SWS_Pos   = 2;  // [3:2] sysclk source status (read-only)
static constexpr uint8_t CFGR_HPRE_Pos  = 4;  // [7:4] AHB prescaler
static constexpr uint8_t CFGR_PPRE1_Pos = 10; // [12:10] APB1 prescaler
static constexpr uint8_t CFGR_PPRE2_Pos = 13; // [15:13] APB2 prescaler

// ---- Clock source selections ----
enum class SystemClockSource : uint8_t {
    HSI = 0b00,
    HSE = 0b01,
    PLL = 0b10,
};

enum class PLL_Source : uint8_t {
    HSI = 0,
    HSE = 1,
};

// ---- AHB prescaler (HPRE field in CFGR) ----
enum class AHB_Prescaler : uint8_t {
    DIV1   = 0b0000,
    DIV2   = 0b1000,
    DIV4   = 0b1001,
    DIV8   = 0b1010,
    DIV16  = 0b1011,
    DIV64  = 0b1100,
    DIV128 = 0b1101,
    DIV256 = 0b1110,
    DIV512 = 0b1111,
};

// ---- APB1/APB2 prescaler (PPRE1/PPRE2 fields in CFGR) ----
enum class APB_Prescaler : uint8_t {
    DIV1  = 0b000,
    DIV2  = 0b100,
    DIV4  = 0b101,
    DIV8  = 0b110,
    DIV16 = 0b111,
};

// ---- AHB1 peripheral clock enable bits in AHB1ENR (RM0090 §6.3.10) ----
enum class AHB1_Peripheral : uint8_t {
    GPIOA = 0,
    GPIOB = 1,
    GPIOC = 2,
    GPIOD = 3,
    GPIOE = 4,
    GPIOF = 5,
    GPIOG = 6,
    GPIOH = 7,
    GPIOI = 8,
    DMA1  = 21,
    DMA2  = 22,
};

// ---- APB1 peripheral clock enable bits in APB1ENR (RM0090 §6.3.12) ----
enum class APB1_Peripheral : uint8_t {
    TIM2   = 0,
    TIM3   = 1,
    TIM4   = 2,
    TIM5   = 3,
    WWDG   = 11,
    SPI2   = 14,
    SPI3   = 15,
    USART2 = 17,
    USART3 = 18,
    I2C1   = 21,
    I2C2   = 22,
    I2C3   = 23,
    PWR    = 28,
};

// ---- APB2 peripheral clock enable bits in APB2ENR (RM0090 §6.3.14) ----
enum class APB2_Peripheral : uint8_t {
    TIM1   = 0,
    USART1 = 4,
    USART6 = 5,
    ADC1   = 8,
    SPI1   = 12,
    SYSCFG = 14,
    TIM9   = 16,
    TIM10  = 17,
    TIM11  = 18,
};

// ---- PLLP helpers ----

consteval bool is_valid_pllp(unsigned P) {
    return P == 2 || P == 4 || P == 6 || P == 8;
}

// Converts PLLP divider value (2/4/6/8) to the 2-bit register encoding.
// Hardware formula: output = VCO / ((reg_val + 1) * 2), so reg_val = P/2 - 1.
consteval uint32_t pllp_to_reg(unsigned P) {
    return (P / 2U) - 1U;
}

constexpr std::string_view get_sysclk_source_txt(SystemClockSource src) {
    switch (src) {
        case SystemClockSource::HSI: return "HSI (16 MHz internal)";
        case SystemClockSource::HSE: return "HSE (external crystal)";
        case SystemClockSource::PLL: return "PLL output";
    }
    assert(false);
    return "";
}

constexpr std::string_view get_pll_source_txt(PLL_Source src) {
    switch (src) {
        case PLL_Source::HSI: return "HSI";
        case PLL_Source::HSE: return "HSE";
    }
    assert(false);
    return "";
}

} // namespace kiv::hal::rcc

#endif // HAL_DRIVER_PRJ_RCCDEFINITIONS_H
