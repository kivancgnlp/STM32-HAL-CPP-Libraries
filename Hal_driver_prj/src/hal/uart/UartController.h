//
// Created by Kivanc Gunalp on 19.06.2026.
//

#ifndef HAL_DRIVER_PRJ_UARTCONTROLLER_H
#define HAL_DRIVER_PRJ_UARTCONTROLLER_H

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "UartDefinitions.h"
#include "../../utils/BitUtils.h"

namespace kiv::hal::uart {

using kiv::utils::get_bit_field;
using kiv::utils::update_bit_field;

// UART_Controller<ID, EMULATION>
//   Polling-only (no interrupts). Supports 8N1 configuration.
//
//   Before calling configure():
//     1. Enable the peripheral clock via RCC (APB1 or APB2 depending on USART_ID).
//     2. Configure the TX/RX GPIO pins as Alternate Function via GPIO_Bank.
//
//   PCLK_HZ for configure<BAUD, PCLK_HZ>():
//     - USART1, USART6 → APB2 clock
//     - USART2–UART5   → APB1 clock
//   Use is_on_apb2(ID) from UartDefinitions.h to check at compile time.
template<USART_ID ID, bool EMULATION>
class UART_Controller {

    std::array<uint32_t, NUM_USART_REGISTERS> emulation_reg_data{};

    // ---- Register access with logging (used for configuration) ----

    template<USART_Register REG>
    void write_hw_register(uint32_t value) {
        const uint32_t hw_addr = get_usart_base_addr(ID) + get_usart_byte_offset(REG);
        volatile uint32_t* ptr = reinterpret_cast<volatile uint32_t*>(hw_addr);

        std::printf("%s write_%s_register 0x%08x : 0x%08x\n",
                    EMULATION ? "Emulation" : "Real",
                    get_usart_id_txt(ID).data(), hw_addr, value);

        if constexpr (!EMULATION) {
            *ptr = value;
        } else {
            emulation_reg_data.at(static_cast<uint8_t>(REG)) = value;
        }
    }

    template<USART_Register REG>
    [[nodiscard]] uint32_t read_hw_register() {
        const uint32_t hw_addr = get_usart_base_addr(ID) + get_usart_byte_offset(REG);
        volatile uint32_t* ptr = reinterpret_cast<volatile uint32_t*>(hw_addr);

        uint32_t value{};
        if constexpr (!EMULATION) {
            value = *ptr;
        } else {
            value = emulation_reg_data.at(static_cast<uint8_t>(REG));
        }

        std::printf("%s read_%s_register  0x%08x : 0x%08x\n",
                    EMULATION ? "Emulation" : "Real",
                    get_usart_id_txt(ID).data(), hw_addr, value);
        return value;
    }

    // ---- Direct register access for runtime data path (no logging) ----
    // Logging every SR poll or DR write in the transmit/receive hot path would
    // flood stdout and make emulation unusable.

    [[nodiscard]] uint32_t read_sr_direct() const {
        const uint32_t hw_addr = get_usart_base_addr(ID) + get_usart_byte_offset(USART_Register::SR);
        return *reinterpret_cast<volatile const uint32_t*>(hw_addr);
    }

    void write_dr_direct(uint8_t byte) const {
        const uint32_t hw_addr = get_usart_base_addr(ID) + get_usart_byte_offset(USART_Register::DR);
        *reinterpret_cast<volatile uint32_t*>(hw_addr) = static_cast<uint32_t>(byte);
    }

    [[nodiscard]] uint8_t read_dr_direct() const {
        const uint32_t hw_addr = get_usart_base_addr(ID) + get_usart_byte_offset(USART_Register::DR);
        return static_cast<uint8_t>(*reinterpret_cast<volatile const uint32_t*>(hw_addr));
    }

public:

    // ================================================================
    // Configuration
    // 8N1 format (8 data bits, no parity, 1 stop bit).
    //
    // BRR formula (OVER8=0, oversampling by 16):
    //   BRR = PCLK_HZ / BAUD_RATE
    // This maps directly to the 12.4 fixed-point USARTDIV register field.
    // static_assert enforces valid range at compile time.
    // ================================================================

    template<uint32_t BAUD_RATE, uint32_t PCLK_HZ>
    void configure() {
        constexpr uint32_t brr_value = PCLK_HZ / BAUD_RATE;
        static_assert(brr_value >= 16,     "BRR too low — PCLK_HZ or BAUD_RATE is wrong");
        static_assert(brr_value <= 0xFFFF, "BRR overflow — PCLK_HZ or BAUD_RATE is wrong");

        if constexpr (EMULATION) {
            std::printf("Emulation %s configure: PCLK=%u Hz  BAUD=%u  BRR=0x%04x\n",
                        get_usart_id_txt(ID).data(), PCLK_HZ, BAUD_RATE, brr_value);
        }

        // BRR must be written before UE=1 (RM0090 §30.3.4)
        write_hw_register<USART_Register::BRR>(brr_value);
        write_hw_register<USART_Register::CR2>(0u);  // 1 stop bit (STOP[13:12]=0b00)
        write_hw_register<USART_Register::CR3>(0u);  // no flow control, no DMA

        uint32_t cr1 = 0u;
        update_bit_field<CR1_RE_Pos, 1>(cr1, 1u);  // receiver on
        update_bit_field<CR1_TE_Pos, 1>(cr1, 1u);  // transmitter on
        update_bit_field<CR1_UE_Pos, 1>(cr1, 1u);  // USART on
        // M=0 (default): 8 data bits
        write_hw_register<USART_Register::CR1>(cr1);
    }

    // ================================================================
    // Transmit
    // ================================================================

    void transmit_byte(uint8_t byte) {
        if constexpr (EMULATION) {
            std::putchar(static_cast<int>(byte));
            std::fflush(stdout);
        } else {
            // Wait until the transmit data register is empty (TXE=1)
            while (get_bit_field<SR_TXE_Pos, 1>(read_sr_direct()) == 0u) {}
            write_dr_direct(byte);
        }
    }

    void transmit(std::string_view str) {
        for (char c : str) {
            transmit_byte(static_cast<uint8_t>(c));
        }
    }

    // Blocks until the shift register has finished sending the last byte.
    // Call before disabling the USART or entering low-power mode.
    void wait_for_tx_complete() {
        if constexpr (!EMULATION) {
            while (get_bit_field<SR_TC_Pos, 1>(read_sr_direct()) == 0u) {}
        }
    }

    // ================================================================
    // Receive
    // ================================================================

    // Non-blocking: returns true if a byte is waiting in DR.
    [[nodiscard]] bool data_available() const {
        if constexpr (EMULATION) {
            return false;  // stdin polling is platform-specific; not implemented
        } else {
            return get_bit_field<SR_RXNE_Pos, 1>(read_sr_direct()) != 0u;
        }
    }

    // Blocking: waits until a byte arrives and returns it.
    [[nodiscard]] uint8_t receive_byte_blocking() {
        if constexpr (EMULATION) {
            return static_cast<uint8_t>(std::getchar());
        } else {
            while (get_bit_field<SR_RXNE_Pos, 1>(read_sr_direct()) == 0u) {}
            return read_dr_direct();
        }
    }

    // ================================================================
    // Diagnostics
    // ================================================================

    void print_emulation_state() const {
        if constexpr (!EMULATION) { return; }

        const uint32_t brr = emulation_reg_data.at(static_cast<uint8_t>(USART_Register::BRR));
        const uint32_t cr1 = emulation_reg_data.at(static_cast<uint8_t>(USART_Register::CR1));
        const uint32_t cr2 = emulation_reg_data.at(static_cast<uint8_t>(USART_Register::CR2));

        const bool ue = get_bit_field<CR1_UE_Pos, 1>(cr1) != 0u;
        const bool te = get_bit_field<CR1_TE_Pos, 1>(cr1) != 0u;
        const bool re = get_bit_field<CR1_RE_Pos, 1>(cr1) != 0u;
        const bool m  = get_bit_field<CR1_M_Pos,  1>(cr1) != 0u;
        const uint32_t stop = get_bit_field<CR2_STOP_Pos, 2>(cr2);

        std::printf("---- %s Emulation State ----\n", get_usart_id_txt(ID).data());
        std::printf("  BRR : 0x%04lx\n",  static_cast<unsigned long>(brr));
        std::printf("  CR1 : UE=%s  TE=%s  RE=%s  WordLen=%s\n",
                    ue ? "ON" : "off", te ? "ON" : "off", re ? "ON" : "off",
                    m ? "9-bit" : "8-bit");
        std::printf("  CR2 : STOP=%lu bit%s\n",
                    stop == 0 ? 1UL : static_cast<unsigned long>(stop),
                    stop == 0 ? "" : "s");
        std::printf("-----------------------------\n");
    }
};

} // namespace kiv::hal::uart

#endif // HAL_DRIVER_PRJ_UARTCONTROLLER_H
