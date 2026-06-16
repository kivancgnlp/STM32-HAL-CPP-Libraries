//
// Created by Kivanc Gunalp on 13.06.2026.
//

#ifndef BNO055_REFACTOR_PROJECT_BITUTILS_H
#define BNO055_REFACTOR_PROJECT_BITUTILS_H
#include <cstdint>
#include <cassert>

template <uint8_t start_bit,uint8_t bit_count>
constexpr uint32_t get_bit_mask() {
    static_assert(bit_count >= 1, "bit_count must be at least 1");
    static_assert(bit_count < 32, "bit_count must be lower than 32");
    static_assert(start_bit < 32, "start_bit must be in [0, 31]");
    static_assert(start_bit + bit_count <= 32, "mask must fit within 32 bits (start_bit + bit_count <= 32)");

    return (((1u << bit_count) - 1u) << start_bit);

}

template <uint8_t start_bit, uint8_t bit_count>
constexpr void update_bit_field(uint32_t &original_value, uint32_t bit_field_to_embed) {
    constexpr uint32_t mask = get_bit_mask<start_bit, bit_count>();
    assert((bit_field_to_embed >> bit_count) == 0 && "field to embed exceeds its size, possible bug");
    const uint32_t shifted = (bit_field_to_embed << start_bit) & mask;
    original_value = (original_value & ~mask) | shifted;
}

template <uint8_t start_bit, uint8_t bit_count>
[[nodiscard]] constexpr uint32_t get_bit_field(uint32_t value) {
    constexpr uint32_t mask = get_bit_mask<start_bit, bit_count>();
    return (value & mask) >> start_bit;
}



#endif //BNO055_REFACTOR_PROJECT_BITUTILS_H
