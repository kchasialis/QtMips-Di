#ifndef BRANCHTARGETBUFFER_H
#define BRANCHTARGETBUFFER_H

#include <cstddef>
#include <cstdint>

class Instruction;

// returns a mask containing n ones (1) on the lower bits.
static constexpr std::uint32_t mask_bits(std::uint32_t n) {
    return static_cast<std::uint32_t>(-(n != 0)) &
            (static_cast<std::uint32_t>(-1) >>
             ((sizeof(std::uint32_t) * CHAR_BIT) - n));
}

// returns 2^exponent
constexpr size_t power_of_2(std::uint8_t exponent) const {
    return exponent == 0 ? 1 : 2 * power_of_2(exponent - 1);
}

class BranchTargetBuffer {
private:
    struct BTBEntry {
        bool valid;
        std::uint16_t tag;
        std::uint32_t address;

        BTBEntry() : valid(0), tag(0), address(0) {}
    };

    std::uint8_t btb_bits;
    size_t btb_size;
    BTBEntry *btb;
    bool need_update;

public:
    explicit BranchTargetBuffer(std::uint8_t btb_bits);
    ~BranchTargetBuffer();

    bool get_pc_address(std::uint32_t btb_idx, std::uint32_t current_pc, std::uint32_t *address) const;
    void update(std::uint32_t btb_idx, std::uint32_t new_pc, std::uint32_t bj_address);
};

#endif // BRANCHTARGETBUFFER_H
