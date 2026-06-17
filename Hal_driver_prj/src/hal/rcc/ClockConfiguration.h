//
// Created by Kiv on 6/17/2026.
//

#ifndef HAL_DRIVER_PRJ_CLOCKCONFIGURATION_H
#define HAL_DRIVER_PRJ_CLOCKCONFIGURATION_H

namespace kiv::hal::rcc{

    enum class SystemClockSource {
        HSI_INTERNAL,
        HSE_EXTERNAL,
    };


    template <SystemClockSource SystemClockSource>
    void configure_system_clock_source() {

    }

    template <unsigned INPUT_FRQ_IN_MHZ, unsigned M, unsigned N, unsigned P>
    void configure_external_system_clock_source_with_pll() {
        constexpr unsigned frq = (INPUT_FRQ_IN_MHZ / M) * N / P;
        static_assert(frq <= 180,"System clock can not exceed 180 MHz");

    }


    template <unsigned M, unsigned N, unsigned P>
    void configure_internal_system_clock_source_with_pll() {
        constexpr unsigned frq = (16 / M) * N / P;
        static_assert(frq <= 180,"System clock can not exceed 180 MHz");

    }

}


#endif //HAL_DRIVER_PRJ_CLOCKCONFIGURATION_H