//
// Created by Kivanc Gunalp on 13.06.2026.
//

#ifndef BNO055_REFACTOR_PROJECT_BITUTILS_H
#define BNO055_REFACTOR_PROJECT_BITUTILS_H

#include <cstdint>
#include <cassert>
#include <type_traits>

namespace kiv::utils {

    template<uint8_t start_bit, uint8_t bit_count, typename DataType = uint32_t>
    [[nodiscard]] constexpr DataType get_bit_mask() {
        constexpr uint8_t total_bits = sizeof(DataType) * 8;

        static_assert(std::is_unsigned_v<DataType>, "DataType must be an unsigned integer");
        static_assert(bit_count >= 1, "bit_count must be at least 1");
        static_assert(start_bit < total_bits, "start_bit is out of bounds");
        static_assert(start_bit + bit_count <= total_bits, "mask exceeds data type size");

        // Prevent UB: shifting a type by its full width is undefined behavior
        if constexpr (bit_count == total_bits) {
            return static_cast<DataType>(~static_cast<DataType>(0));
        } else {
            return static_cast<DataType>(((static_cast<DataType>(1) << bit_count) - 1u) << start_bit);
        }
    }

    template<uint8_t start_bit, uint8_t bit_count, typename DataType = uint32_t>
    constexpr void update_bit_field(DataType &original_value, DataType bit_field_to_embed) {
        constexpr DataType mask = get_bit_mask<start_bit, bit_count, DataType>();
        constexpr uint8_t total_bits = sizeof(DataType) * 8;

        // Prevent UB: check asset safely only if the shift amount is less than total_bits
        if constexpr (bit_count < total_bits) {
            assert((bit_field_to_embed >> bit_count) == 0 && "field to embed exceeds its allocated bit_count");
        }

        const DataType shifted = (bit_field_to_embed << start_bit) & mask;
        original_value = (original_value & ~mask) | shifted;
    }

    template<uint8_t start_bit, uint8_t bit_count, typename DataType = uint32_t>
    [[nodiscard]] constexpr DataType get_bit_field(DataType value) {
        constexpr DataType mask = get_bit_mask<start_bit, bit_count, DataType>();
        return (value & mask) >> start_bit;
    }
}; // namespace kiv::utils


#endif //BNO055_REFACTOR_PROJECT_BITUTILS_H
