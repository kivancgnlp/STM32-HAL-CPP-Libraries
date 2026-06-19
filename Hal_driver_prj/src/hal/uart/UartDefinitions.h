//
// Created by Kivanc Gunalp on 19.06.2026.
//

#ifndef HAL_DRIVER_PRJ_UARTDEFINITIONS_H
#define HAL_DRIVER_PRJ_UARTDEFINITIONS_H

#include <cassert>
#include <cstdint>
#include <string_view>

namespace kiv::hal::uart {

enum class USART_ID : uint8_t {
    USART_1 = 0,
    USART_2,
    USART_3,
    UART_4,
    UART_5,
    USART_6,
};

// USART registers in address order; ordinal * 4 == byte offset from peripheral base.
enum class USART_Register : uint8_t {
    SR   = 0,  // Status register      — offset 0x00
    DR   = 1,  // Data register        — offset 0x04
    BRR  = 2,  // Baud rate register   — offset 0x08
    CR1  = 3,  // Control register 1  — offset 0x0C
    CR2  = 4,  // Control register 2  — offset 0x10
    CR3  = 5,  // Control register 3  — offset 0x14
    NUM_REGISTERS
};

static constexpr uint8_t NUM_USART_REGISTERS =
    static_cast<uint8_t>(USART_Register::NUM_REGISTERS);

consteval uint32_t get_usart_byte_offset(USART_Register reg) {
    return static_cast<uint32_t>(reg) * 4;
}

// Peripheral base addresses (RM0090 §2.3 memory map)
// USART1, USART6 are on APB2; USART2–UART5 are on APB1.
consteval uint32_t get_usart_base_addr(USART_ID id) {
    switch (id) {
        case USART_ID::USART_1: return 0x40011000UL;
        case USART_ID::USART_2: return 0x40004400UL;
        case USART_ID::USART_3: return 0x40004800UL;
        case USART_ID::UART_4:  return 0x40004C00UL;
        case USART_ID::UART_5:  return 0x40005000UL;
        case USART_ID::USART_6: return 0x40011400UL;
    }
    assert(false);
    return 0;
}

// Returns true for USART_1 and USART_6 which are clocked from APB2.
// All others are clocked from APB1.
// Use this to pass the correct PCLK_HZ to configure<>().
consteval bool is_on_apb2(USART_ID id) {
    return id == USART_ID::USART_1 || id == USART_ID::USART_6;
}

// ---- SR register bit positions (RM0090 §30.6.1) ----
static constexpr uint8_t SR_PE_Pos   = 0;   // Parity error
static constexpr uint8_t SR_RXNE_Pos = 5;   // Read data register not empty
static constexpr uint8_t SR_TC_Pos   = 6;   // Transmission complete
static constexpr uint8_t SR_TXE_Pos  = 7;   // Transmit data register empty

// ---- CR1 register bit positions (RM0090 §30.6.4) ----
static constexpr uint8_t CR1_RE_Pos  = 2;   // Receiver enable
static constexpr uint8_t CR1_TE_Pos  = 3;   // Transmitter enable
static constexpr uint8_t CR1_M_Pos   = 12;  // Word length (0=8 bits, 1=9 bits)
static constexpr uint8_t CR1_UE_Pos  = 13;  // USART enable

// ---- CR2 register bit positions (RM0090 §30.6.5) ----
static constexpr uint8_t CR2_STOP_Pos = 12; // Stop bits [13:12] (0b00 = 1 stop bit)

constexpr std::string_view get_usart_id_txt(USART_ID id) {
    switch (id) {
        case USART_ID::USART_1: return "USART1";
        case USART_ID::USART_2: return "USART2";
        case USART_ID::USART_3: return "USART3";
        case USART_ID::UART_4:  return "UART4";
        case USART_ID::UART_5:  return "UART5";
        case USART_ID::USART_6: return "USART6";
    }
    assert(false);
    return "";
}

} // namespace kiv::hal::uart

#endif // HAL_DRIVER_PRJ_UARTDEFINITIONS_H
