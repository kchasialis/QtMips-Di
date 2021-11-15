#ifndef BRANCHTARGETBUFFER_H
#define BRANCHTARGETBUFFER_H

#include <QObject>
#include <cstddef>
#include <cstdint>
#include <climits>

class Instruction;

// returns the bits from [start, end] of number num, ranging from 0 - 31
constexpr std::uint32_t mask_bits(uint32_t num, uint32_t start, uint32_t end) {
    return num & ((~0U >> (31 - end)) & (~0U << start));
}

// returns 2^exponent
constexpr size_t power_of_2(uint8_t exponent) {
    return exponent == 0 ? 1 : 2 * power_of_2(exponent - 1);
}

namespace machine {

class BranchTargetBuffer : public QObject {
    Q_OBJECT
private:
    struct BTBEntry {
        bool valid;
        uint32_t tag;
        uint32_t address;

        BTBEntry() : valid(false), tag(0), address(0) {}
    };

    uint8_t btb_bits;
    size_t btb_size;
    BTBEntry *btb;

signals:
    void pred_updated_btb(std::int32_t);
    void pred_accessed_btb(std::int32_t);

public:
    explicit BranchTargetBuffer(std::uint8_t btb_bits);
    ~BranchTargetBuffer();

    bool pc_address(uint32_t current_pc, uint32_t *address);
    bool btb_entry_valid(std::uint32_t btb_idx) const;
    std::uint32_t btb_entry_address(std::uint32_t btb_idx) const;
    std::uint32_t btb_entry_tag(std::uint32_t btb_idx) const;
    void update(uint32_t pc, uint32_t inst_addr);
    void reset();
};

}
#endif // BRANCHTARGETBUFFER_H
