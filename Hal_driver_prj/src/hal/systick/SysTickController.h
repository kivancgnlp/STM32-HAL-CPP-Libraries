//
// Created by Kivanc Gunalp on 19.06.2026.
//

#ifndef HAL_DRIVER_PRJ_SYSTICKCONTROLLER_H
#define HAL_DRIVER_PRJ_SYSTICKCONTROLLER_H

#include <array>
#include <cstdint>
#include <cstdio>

#include "SysTickDefinitions.h"
#include "SysTickEmulationHelper.h"
#include "../../utils/BitUtils.h"

namespace kiv::hal::systick {

// SysTick_Controller<EMULATION>
//   EMULATION=false : programs STK_LOAD/VAL/CTRL registers directly; SysTick_Handler ISR
//                     (defined in SysTickController.cpp) drives g_systick_ms_count.
//   EMULATION=true  : skips register writes; SysTick_EmulationHelper<true> spawns a
//                     std::thread that increments g_systick_ms_count on the host.
template<bool EMULATION>
class SysTick_Controller {

    std::array<uint32_t, NUM_SYSTICK_REGISTERS> emulation_reg_data{};
    SysTick_EmulationHelper<EMULATION> emulation_helper;

    template<SysTick_Register REG>
    void write_hw_register(uint32_t value) {
        const uint32_t hw_addr = STK_BASE_ADDR + get_systick_byte_offset(REG);
        volatile uint32_t* ptr = reinterpret_cast<volatile uint32_t*>(hw_addr);

        std::printf("%s write_stk_register 0x%08x : 0x%08x\n",
                    EMULATION ? "Emulation" : "Real", hw_addr, value);

        if constexpr (!EMULATION) {
            *ptr = value;
        } else {
            emulation_reg_data.at(static_cast<uint8_t>(REG)) = value;
        }
    }

    template<SysTick_Register REG>
    uint32_t read_hw_register() {
        const uint32_t hw_addr = STK_BASE_ADDR + get_systick_byte_offset(REG);
        volatile uint32_t* ptr = reinterpret_cast<volatile uint32_t*>(hw_addr);

        uint32_t value{};
        if constexpr (!EMULATION) {
            value = *ptr;
        } else {
            value = emulation_reg_data.at(static_cast<uint8_t>(REG));
        }

        std::printf("%s read_stk_register  0x%08x : 0x%08x\n",
                    EMULATION ? "Emulation" : "Real", hw_addr, value);
        return value;
    }

public:
    // Start SysTick.
    //   HCLK_HZ : processor clock in Hz — must match the clock configured by RCC_Controller.
    //   TICK_HZ : desired interrupt/tick frequency (default 1000 → 1 ms per tick).
    template<uint32_t HCLK_HZ, uint32_t TICK_HZ = 1000>
    void start() {
        constexpr uint32_t reload = HCLK_HZ / TICK_HZ - 1u;
        static_assert(reload <= 0x00FF'FFFFu, "Reload value exceeds the 24-bit SysTick counter");

        if constexpr (EMULATION) {
            std::printf("Emulation SysTick start: HCLK=%u Hz  TICK_HZ=%u  reload=%u\n",
                        HCLK_HZ, TICK_HZ, reload);
            constexpr uint32_t tick_period_us = 1'000'000u / TICK_HZ;
            emulation_helper.start(tick_period_us);
        } else {
            write_hw_register<SysTick_Register::LOAD>(reload);
            write_hw_register<SysTick_Register::VAL>(0u);  // clear current value before enabling

            using kiv::utils::update_bit_field;
            uint32_t ctrl = 0u;
            update_bit_field<STK_CTRL_ENABLE_Pos,    1>(ctrl, 1u); // counter on
            update_bit_field<STK_CTRL_TICKINT_Pos,   1>(ctrl, 1u); // fire IRQ on wrap
            update_bit_field<STK_CTRL_CLKSOURCE_Pos, 1>(ctrl, 1u); // use HCLK
            write_hw_register<SysTick_Register::CTRL>(ctrl);
        }
    }

    void stop() {
        if constexpr (!EMULATION) {
            write_hw_register<SysTick_Register::CTRL>(0u);
        } else {
            emulation_helper.stop();
        }
    }

    [[nodiscard]] uint32_t get_tick_ms() const {
        return g_systick_ms_count.load(std::memory_order_relaxed);
    }

    // Busy-wait delay. Handles 32-bit counter wrap correctly via unsigned subtraction.
    void delay_ms(uint32_t ms) const {
        const uint32_t start = get_tick_ms();
        while ((get_tick_ms() - start) < ms) {}
    }
};

} // namespace kiv::hal::systick

#endif // HAL_DRIVER_PRJ_SYSTICKCONTROLLER_H
