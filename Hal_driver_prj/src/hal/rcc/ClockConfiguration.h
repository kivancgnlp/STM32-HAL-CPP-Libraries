//
// Created by Kiv on 6/17/2026.
//

#ifndef HAL_DRIVER_PRJ_CLOCKCONFIGURATION_H
#define HAL_DRIVER_PRJ_CLOCKCONFIGURATION_H

#include <array>
#include <cstdint>
#include <cstdio>

#include "RccDefinitions.h"
#include "../../utils/BitUtils.h"

namespace kiv::hal::rcc {

// RCC_Controller<EMULATION> mirrors the GPIO_Bank<BANK, EMULATION> design:
//  - EMULATION=true : register writes/reads go to an internal array; hardware
//    ready-flag polling is skipped so the code runs on host.
//  - EMULATION=false: all accesses target the real RCC peripheral registers.
template <bool EMULATION>
class RCC_Controller {

    // Backing store used in emulation mode (indexed by RCC_Register ordinal).
    std::array<uint32_t, NUM_RCC_REGISTERS> emulation_reg_data{};

    // ---- Low-level register access ----

    template<RCC_Register REG>
    void write_hw_register(uint32_t value) {
        // Integer arithmetic keeps the byte offset exact before the cast;
        // pointer arithmetic on a uint32_t* would scale by 4 and give a wrong address.
        const uint32_t hw_addr = RCC_BASE_ADDR + get_rcc_byte_offset(REG);
        volatile uint32_t* ptr = reinterpret_cast<volatile uint32_t*>(hw_addr);

        std::printf("%s write_rcc_register 0x%08x : 0x%08x\n",
                    EMULATION ? "Emulation" : "Real", hw_addr, value);

        if constexpr (!EMULATION) {
            *ptr = value;
        } else {
            emulation_reg_data.at(static_cast<uint8_t>(REG)) = value;
        }
    }

    template<RCC_Register REG>
    uint32_t read_hw_register() {
        const uint32_t hw_addr = RCC_BASE_ADDR + get_rcc_byte_offset(REG);
        volatile uint32_t* ptr = reinterpret_cast<volatile uint32_t*>(hw_addr);

        uint32_t value{};
        if constexpr (!EMULATION) {
            value = *ptr;
        } else {
            value = emulation_reg_data.at(static_cast<uint8_t>(REG));
        }

        std::printf("%s read_rcc_register  0x%08x : 0x%08x\n",
                    EMULATION ? "Emulation" : "Real", hw_addr, value);
        return value;
    }

    // ---- Ready-flag polling helpers (templated for compile-time bit position) ----
    // In emulation mode returns the expected state immediately.

    template<uint8_t BIT_POS>
    bool is_cr_bit_set() {
        if constexpr (EMULATION) { return true; }
        return kiv::utils::get_bit_field<uint32_t, BIT_POS, 1>(
                   read_hw_register<RCC_Register::CR>()) != 0u;
    }

    template<uint8_t BIT_POS>
    bool is_cr_bit_clear() {
        if constexpr (EMULATION) { return true; }
        return kiv::utils::get_bit_field<uint32_t, BIT_POS, 1>(
                   read_hw_register<RCC_Register::CR>()) == 0u;
    }

    bool is_sysclk_switched_to(SystemClockSource src) {
        if constexpr (EMULATION) { return true; }
        return kiv::utils::get_bit_field<uint32_t, CFGR_SWS_Pos, 2>(
                   read_hw_register<RCC_Register::CFGR>())
               == static_cast<uint32_t>(src);
    }

public:

    // ================================================================
    // Oscillator control
    // Mirrors HAL_RCC_OscConfig() logic in stm32f4xx_hal_rcc.c §6.3.1–§6.3.4
    // ================================================================

    void enable_hsi() {
        uint32_t cr = read_hw_register<RCC_Register::CR>();
        kiv::utils::update_bit_field<uint32_t, CR_HSION_Pos, 1>(cr, 1u);
        write_hw_register<RCC_Register::CR>(cr);
        while (!is_cr_bit_set<CR_HSIRDY_Pos>()) {}
    }

    void disable_hsi() {
        uint32_t cr = read_hw_register<RCC_Register::CR>();
        kiv::utils::update_bit_field<uint32_t, CR_HSION_Pos, 1>(cr, 0u);
        write_hw_register<RCC_Register::CR>(cr);
        while (!is_cr_bit_clear<CR_HSIRDY_Pos>()) {}
    }

    // Enables HSE oscillator (external crystal).
    void enable_hse() {
        uint32_t cr = read_hw_register<RCC_Register::CR>();
        kiv::utils::update_bit_field<uint32_t, CR_HSEON_Pos, 1>(cr, 1u);
        write_hw_register<RCC_Register::CR>(cr);
        while (!is_cr_bit_set<CR_HSERDY_Pos>()) {}
    }

    // Enables HSE in bypass mode (external clock signal, no crystal).
    void enable_hse_bypass() {
        uint32_t cr = read_hw_register<RCC_Register::CR>();
        kiv::utils::update_bit_field<uint32_t, CR_HSEBYP_Pos, 1>(cr, 1u);
        kiv::utils::update_bit_field<uint32_t, CR_HSEON_Pos,  1>(cr, 1u);
        write_hw_register<RCC_Register::CR>(cr);
        while (!is_cr_bit_set<CR_HSERDY_Pos>()) {}
    }

    void disable_hse() {
        uint32_t cr = read_hw_register<RCC_Register::CR>();
        kiv::utils::update_bit_field<uint32_t, CR_HSEON_Pos,  1>(cr, 0u);
        kiv::utils::update_bit_field<uint32_t, CR_HSEBYP_Pos, 1>(cr, 0u);
        write_hw_register<RCC_Register::CR>(cr);
        while (!is_cr_bit_clear<CR_HSERDY_Pos>()) {}
    }

    // ================================================================
    // PLL configuration
    // Mirrors PLLCFGR programming sequence from stm32f4xx_hal_rcc.c §HAL_RCC_OscConfig
    //
    // Template parameters:
    //   SRC  - PLL input clock (PLL_Source::HSI or PLL_Source::HSE)
    //   M    - input divider  [2..63],   VCO_in = SRC_MHz / M (target 1–2 MHz)
    //   N    - multiplier     [50..432],  VCO    = VCO_in * N (target 100–432 MHz)
    //   P    - output divider {2,4,6,8},  SYSCLK = VCO / P
    //   Q    - USB divider    [2..15],    USB_CLK = VCO / Q (48 MHz for USB OTG)
    // ================================================================

    template<PLL_Source SRC, unsigned M, unsigned N, unsigned P, unsigned Q = 7>
    void configure_and_enable_pll() {
        static_assert(M >= 2 && M <= 63,   "PLLM must be in [2, 63]");
        static_assert(N >= 50 && N <= 432, "PLLN must be in [50, 432]");
        static_assert(is_valid_pllp(P),    "PLLP must be 2, 4, 6, or 8");
        static_assert(Q >= 2 && Q <= 15,   "PLLQ must be in [2, 15]");

        // Step 1: disable PLL before reconfiguring (RM0090 §6.3.2 note)
        {
            uint32_t cr = read_hw_register<RCC_Register::CR>();
            kiv::utils::update_bit_field<uint32_t, CR_PLLON_Pos, 1>(cr, 0u);
            write_hw_register<RCC_Register::CR>(cr);
        }
        while (!is_cr_bit_clear<CR_PLLRDY_Pos>()) {}

        // Step 2: write all PLL parameters in one shot
        {
            uint32_t pllcfgr = 0u;
            kiv::utils::update_bit_field<uint32_t, PLLCFGR_PLLM_Pos,   6>(pllcfgr, static_cast<uint32_t>(M));
            kiv::utils::update_bit_field<uint32_t, PLLCFGR_PLLN_Pos,   9>(pllcfgr, static_cast<uint32_t>(N));
            kiv::utils::update_bit_field<uint32_t, PLLCFGR_PLLP_Pos,   2>(pllcfgr, static_cast<uint32_t>(pllp_to_reg(P)));
            kiv::utils::update_bit_field<uint32_t, PLLCFGR_PLLSRC_Pos, 1>(pllcfgr, static_cast<uint32_t>(SRC));
            kiv::utils::update_bit_field<uint32_t, PLLCFGR_PLLQ_Pos,   4>(pllcfgr, static_cast<uint32_t>(Q));
            write_hw_register<RCC_Register::PLLCFGR>(pllcfgr);
        }

        // Step 3: re-enable PLL and wait for lock
        {
            uint32_t cr = read_hw_register<RCC_Register::CR>();
            kiv::utils::update_bit_field<uint32_t, CR_PLLON_Pos, 1>(cr, 1u);
            write_hw_register<RCC_Register::CR>(cr);
        }
        while (!is_cr_bit_set<CR_PLLRDY_Pos>()) {}
    }

    // ================================================================
    // Bus prescalers
    // Mirrors HAL_RCC_ClockConfig() in stm32f4xx_hal_rcc.c
    //
    // STM32F4 limits: HCLK ≤ 180 MHz, PCLK1 ≤ 45 MHz, PCLK2 ≤ 90 MHz.
    // Defaults for 180 MHz SYSCLK: AHB=DIV1, APB1=DIV4, APB2=DIV2.
    // ================================================================

    template<AHB_Prescaler AHB_PRE  = AHB_Prescaler::DIV1,
             APB_Prescaler APB1_PRE = APB_Prescaler::DIV4,
             APB_Prescaler APB2_PRE = APB_Prescaler::DIV2>
    void configure_bus_prescalers() {
        uint32_t cfgr = read_hw_register<RCC_Register::CFGR>();
        kiv::utils::update_bit_field<uint32_t, CFGR_HPRE_Pos,  4>(cfgr, static_cast<uint32_t>(AHB_PRE));
        kiv::utils::update_bit_field<uint32_t, CFGR_PPRE1_Pos, 3>(cfgr, static_cast<uint32_t>(APB1_PRE));
        kiv::utils::update_bit_field<uint32_t, CFGR_PPRE2_Pos, 3>(cfgr, static_cast<uint32_t>(APB2_PRE));
        write_hw_register<RCC_Register::CFGR>(cfgr);
    }

    // ================================================================
    // System clock source selection
    // Mirrors __HAL_RCC_SYSCLK_CONFIG() + SWS polling in stm32f4xx_hal_rcc.c
    // ================================================================

    template<SystemClockSource SRC>
    void select_sysclk_source() {
        uint32_t cfgr = read_hw_register<RCC_Register::CFGR>();
        kiv::utils::update_bit_field<uint32_t, CFGR_SW_Pos, 2>(cfgr, static_cast<uint32_t>(SRC));
        write_hw_register<RCC_Register::CFGR>(cfgr);
        // Wait until the hardware confirms the switch via the read-only SWS field
        while (!is_sysclk_switched_to(SRC)) {}
    }

    // ================================================================
    // Peripheral clock enables / disables
    // Mirrors __HAL_RCC_GPIOx_CLK_ENABLE() pattern in stm32f4xx_hal_rcc.h.
    // The dummy read after enabling provides the mandatory propagation delay
    // before the first peripheral register access (RM0090 §6.1 errata note).
    // ================================================================

    template<AHB1_Peripheral PERIPH>
    void enable_ahb1_clock() {
        uint32_t enr = read_hw_register<RCC_Register::AHB1ENR>();
        kiv::utils::update_bit_field<uint32_t, static_cast<uint8_t>(PERIPH), 1>(enr, 1u);
        write_hw_register<RCC_Register::AHB1ENR>(enr);
        (void)read_hw_register<RCC_Register::AHB1ENR>(); // propagation delay
    }

    template<AHB1_Peripheral PERIPH>
    void disable_ahb1_clock() {
        uint32_t enr = read_hw_register<RCC_Register::AHB1ENR>();
        kiv::utils::update_bit_field<uint32_t, static_cast<uint8_t>(PERIPH), 1>(enr, 0u);
        write_hw_register<RCC_Register::AHB1ENR>(enr);
    }

    template<APB1_Peripheral PERIPH>
    void enable_apb1_clock() {
        uint32_t enr = read_hw_register<RCC_Register::APB1ENR>();
        kiv::utils::update_bit_field<uint32_t, static_cast<uint8_t>(PERIPH), 1>(enr, 1u);
        write_hw_register<RCC_Register::APB1ENR>(enr);
        (void)read_hw_register<RCC_Register::APB1ENR>();
    }

    template<APB1_Peripheral PERIPH>
    void disable_apb1_clock() {
        uint32_t enr = read_hw_register<RCC_Register::APB1ENR>();
        kiv::utils::update_bit_field<uint32_t, static_cast<uint8_t>(PERIPH), 1>(enr, 0u);
        write_hw_register<RCC_Register::APB1ENR>(enr);
    }

    template<APB2_Peripheral PERIPH>
    void enable_apb2_clock() {
        uint32_t enr = read_hw_register<RCC_Register::APB2ENR>();
        kiv::utils::update_bit_field<uint32_t, static_cast<uint8_t>(PERIPH), 1>(enr, 1u);
        write_hw_register<RCC_Register::APB2ENR>(enr);
        (void)read_hw_register<RCC_Register::APB2ENR>();
    }

    template<APB2_Peripheral PERIPH>
    void disable_apb2_clock() {
        uint32_t enr = read_hw_register<RCC_Register::APB2ENR>();
        kiv::utils::update_bit_field<uint32_t, static_cast<uint8_t>(PERIPH), 1>(enr, 0u);
        write_hw_register<RCC_Register::APB2ENR>(enr);
    }

    // ================================================================
    // High-level clock configuration helpers
    // Compose the individual steps into the full sequence required to
    // switch the system clock, matching the typical CubeMX init flow.
    // ================================================================

    // Configure SYSCLK from HSE through PLL.
    // SYSCLK_MHz = (HSE_MHZ / M) * N / P
    template<unsigned HSE_MHZ, unsigned M, unsigned N, unsigned P,
             unsigned Q             = 7,
             AHB_Prescaler AHB_PRE  = AHB_Prescaler::DIV1,
             APB_Prescaler APB1_PRE = APB_Prescaler::DIV4,
             APB_Prescaler APB2_PRE = APB_Prescaler::DIV2>
    void configure_hse_pll_sysclk() {
        constexpr unsigned sysclk_mhz = (HSE_MHZ / M) * N / P;
        static_assert(sysclk_mhz <= 180, "SYSCLK cannot exceed 180 MHz");

        if constexpr (EMULATION) {
            std::printf("Emulation configure_hse_pll_sysclk: HSE=%u MHz  "
                        "M=%u N=%u P=%u -> SYSCLK=%u MHz\n",
                        HSE_MHZ, M, N, P, sysclk_mhz);
        }

        enable_hse();
        configure_and_enable_pll<PLL_Source::HSE, M, N, P, Q>();
        configure_bus_prescalers<AHB_PRE, APB1_PRE, APB2_PRE>();
        select_sysclk_source<SystemClockSource::PLL>();
    }

    // Configure SYSCLK from the internal HSI (fixed 16 MHz) through PLL.
    // SYSCLK_MHz = (16 / M) * N / P
    template<unsigned M, unsigned N, unsigned P,
             unsigned Q             = 7,
             AHB_Prescaler AHB_PRE  = AHB_Prescaler::DIV1,
             APB_Prescaler APB1_PRE = APB_Prescaler::DIV4,
             APB_Prescaler APB2_PRE = APB_Prescaler::DIV2>
    void configure_hsi_pll_sysclk() {
        constexpr unsigned sysclk_mhz = (16u / M) * N / P;
        static_assert(sysclk_mhz <= 180, "SYSCLK cannot exceed 180 MHz");

        if constexpr (EMULATION) {
            std::printf("Emulation configure_hsi_pll_sysclk: HSI=16 MHz  "
                        "M=%u N=%u P=%u -> SYSCLK=%u MHz\n",
                        M, N, P, sysclk_mhz);
        }

        enable_hsi();
        configure_and_enable_pll<PLL_Source::HSI, M, N, P, Q>();
        configure_bus_prescalers<AHB_PRE, APB1_PRE, APB2_PRE>();
        select_sysclk_source<SystemClockSource::PLL>();
    }

    // ================================================================
    // Emulation diagnostic — decodes and prints current register state.
    // No-op when EMULATION=false.
    // ================================================================

    void print_emulation_state() {
        if constexpr (!EMULATION) { return; }

        using kiv::utils::get_bit_field;

        const uint32_t cr      = emulation_reg_data.at(static_cast<uint8_t>(RCC_Register::CR));
        const uint32_t pllcfgr = emulation_reg_data.at(static_cast<uint8_t>(RCC_Register::PLLCFGR));
        const uint32_t cfgr    = emulation_reg_data.at(static_cast<uint8_t>(RCC_Register::CFGR));
        const uint32_t ahb1enr = emulation_reg_data.at(static_cast<uint8_t>(RCC_Register::AHB1ENR));
        const uint32_t apb1enr = emulation_reg_data.at(static_cast<uint8_t>(RCC_Register::APB1ENR));
        const uint32_t apb2enr = emulation_reg_data.at(static_cast<uint8_t>(RCC_Register::APB2ENR));

        const bool hsi_on = get_bit_field<uint32_t, CR_HSION_Pos,  1>(cr) != 0u;
        const bool hse_on = get_bit_field<uint32_t, CR_HSEON_Pos,  1>(cr) != 0u;
        const bool pll_on = get_bit_field<uint32_t, CR_PLLON_Pos,  1>(cr) != 0u;

        const uint32_t pllm   = get_bit_field<uint32_t, PLLCFGR_PLLM_Pos,   6>(pllcfgr);
        const uint32_t plln   = get_bit_field<uint32_t, PLLCFGR_PLLN_Pos,   9>(pllcfgr);
        const uint32_t pllp_r = get_bit_field<uint32_t, PLLCFGR_PLLP_Pos,   2>(pllcfgr);
        const uint32_t pllq   = get_bit_field<uint32_t, PLLCFGR_PLLQ_Pos,   4>(pllcfgr);
        const auto pllsrc     = static_cast<PLL_Source>(
                                    get_bit_field<uint32_t, PLLCFGR_PLLSRC_Pos, 1>(pllcfgr));
        const uint32_t pllp   = (pllp_r + 1u) * 2u; // convert reg encoding back to divider

        const auto sw = static_cast<SystemClockSource>(
                            get_bit_field<uint32_t, CFGR_SW_Pos, 2>(cfgr));

        std::printf("---- RCC Emulation State ----\n");
        std::printf("  CR      : HSI=%s  HSE=%s  PLL=%s\n",
                    hsi_on ? "ON" : "off", hse_on ? "ON" : "off", pll_on ? "ON" : "off");
        if (pll_on) {
            std::printf("  PLLCFGR : src=%s  M=%lu  N=%lu  P=%lu  Q=%lu\n",
                        get_pll_source_txt(pllsrc).data(),
                        static_cast<unsigned long>(pllm),
                        static_cast<unsigned long>(plln),
                        static_cast<unsigned long>(pllp),
                        static_cast<unsigned long>(pllq));
        }
        std::printf("  SYSCLK  : %s\n", get_sysclk_source_txt(sw).data());
        std::printf("  AHB1ENR : 0x%08lx\n", static_cast<unsigned long>(ahb1enr));
        std::printf("  APB1ENR : 0x%08lx\n", static_cast<unsigned long>(apb1enr));
        std::printf("  APB2ENR : 0x%08lx\n", static_cast<unsigned long>(apb2enr));
        std::printf("-----------------------------\n");
    }
};

} // namespace kiv::hal::rcc

#endif // HAL_DRIVER_PRJ_CLOCKCONFIGURATION_H
