//
// Created by Kivanc Gunalp on 19.06.2026.
//

#ifndef HAL_DRIVER_PRJ_SYSTICKDEFINITIONS_H
#define HAL_DRIVER_PRJ_SYSTICKDEFINITIONS_H

#include <atomic>
#include <cstdint>

namespace kiv::hal::systick {

// SysTick lives in the ARM Cortex-M System Control Space — not on any APB/AHB bus.
// No RCC clock enable is required to use it.
static constexpr uint32_t STK_BASE_ADDR = 0xE000E010UL;

// Registers in address order; ordinal * 4 == byte offset from STK_BASE_ADDR.
enum class SysTick_Register : uint8_t {
    CTRL  = 0,  // Control and Status — offset 0x00
    LOAD  = 1,  // Reload Value       — offset 0x04
    VAL   = 2,  // Current Value      — offset 0x08
    CALIB = 3,  // Calibration        — offset 0x0C (read-only)
    NUM_REGISTERS
};

static constexpr uint8_t NUM_SYSTICK_REGISTERS =
    static_cast<uint8_t>(SysTick_Register::NUM_REGISTERS);

consteval uint32_t get_systick_byte_offset(SysTick_Register reg) {
    return static_cast<uint32_t>(reg) * 4;
}

// STK_CTRL bit positions (ARMv7-M Architecture Reference Manual §B3.3.4)
static constexpr uint8_t STK_CTRL_ENABLE_Pos    = 0;
static constexpr uint8_t STK_CTRL_TICKINT_Pos   = 1;
static constexpr uint8_t STK_CTRL_CLKSOURCE_Pos = 2;   // 1=HCLK, 0=HCLK/8
static constexpr uint8_t STK_CTRL_COUNTFLAG_Pos = 16;  // read-only, set on wrap

// Shared tick counter.
// - Real HW  : incremented by SysTick_Handler (ISR, defined in SysTickController.cpp).
// - Emulation: incremented by the simulation thread (SysTick_EmulationHelper<true>).
// std::atomic satisfies both: ISR-safe on Cortex-M (LDREX/STREX) and thread-safe on host.
extern std::atomic<uint32_t> g_systick_ms_count;

} // namespace kiv::hal::systick

#endif // HAL_DRIVER_PRJ_SYSTICKDEFINITIONS_H
