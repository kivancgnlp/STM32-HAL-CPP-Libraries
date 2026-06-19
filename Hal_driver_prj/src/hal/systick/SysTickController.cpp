//
// Created by Kivanc Gunalp on 19.06.2026.
//

#include "SysTickController.h"

namespace kiv::hal::systick {
    std::atomic<uint32_t> g_systick_ms_count{0};
}

// On real hardware this ISR fires every time the SysTick counter wraps to zero.
// In simulation (EMULATION=true) it is never called — the emulation thread drives
// g_systick_ms_count directly so this symbol is present but unreachable.
extern "C" void SysTick_Handler() {
    kiv::hal::systick::g_systick_ms_count.fetch_add(1, std::memory_order_relaxed);
}
