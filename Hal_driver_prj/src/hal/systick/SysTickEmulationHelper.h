//
// Created by Kivanc Gunalp on 19.06.2026.
//

#ifndef HAL_DRIVER_PRJ_SYSTICKEMULATIONHELPER_H
#define HAL_DRIVER_PRJ_SYSTICKEMULATIONHELPER_H

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <thread>

#include "SysTickDefinitions.h"

namespace kiv::hal::systick {

// Hardware build: empty shell — zero cost, no thread headers pulled into the TU.
template<bool EMULATION>
class SysTick_EmulationHelper {};

// Simulation build: drives g_systick_ms_count from a dedicated std::thread
// instead of the real SysTick IRQ, so the host can run the blink loop normally.
template<>
class SysTick_EmulationHelper<true> {
    std::thread    tick_thread;
    std::atomic<bool> running{false};

public:
    void start(uint32_t tick_period_us) {
        g_systick_ms_count.store(0, std::memory_order_relaxed);
        running.store(true, std::memory_order_relaxed);

        tick_thread = std::thread([this, tick_period_us]() {
            while (running.load(std::memory_order_relaxed)) {
                std::this_thread::sleep_for(std::chrono::microseconds(tick_period_us));
                g_systick_ms_count.fetch_add(1, std::memory_order_relaxed);
            }
        });

        std::printf("Emulation SysTick thread started (period: %u us)\n", tick_period_us);
    }

    void stop() {
        running.store(false, std::memory_order_relaxed);
        if (tick_thread.joinable()) {
            tick_thread.join();
        }
        std::printf("Emulation SysTick thread stopped\n");
    }

    ~SysTick_EmulationHelper() { stop(); }
};

} // namespace kiv::hal::systick

#endif // HAL_DRIVER_PRJ_SYSTICKEMULATIONHELPER_H
