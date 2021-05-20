#include "branchtargetbuffer.h"
#include "instruction.h"

BranchTargetBuffer::BranchTargetBuffer(std::uint8_t btb_bits) {
    this->btb_bits = btb_bits;
    this->btb_size = power_of_2(this->btb_bits);
    this->btb = new BTBEntry[btb_size];
}

BranchTargetBuffer::~BranchTargetBuffer() {
    delete[] this->btb;
}

bool BranchTargetBuffer::get_pc_address(std::uint32_t btb_idx, std::uint32_t current_pc, std::uint32_t *address) const {
    std::uint32_t tag = current_pc & mask_bits(16);

    *address = btb[btb_idx].address;
    need_update = !(btb[bht_idx].valid && btb[bht_idx].tag == tag);

    return !need_update;
}

void BranchTargetBuffer::update(std::uint32_t btb_idx, std::uint32_t new_pc, std::uint32_t bj_address) {
    if (need_update) {
        btb[btb_idx].tag = new_pc & mask_bits(16);
        btb[btb_idx].valid = true;
        btb[btb_idx].address = bj_address;
    }
}
