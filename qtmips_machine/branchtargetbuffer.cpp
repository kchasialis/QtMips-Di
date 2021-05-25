#include "branchtargetbuffer.h"
#include "instruction.h"

using namespace machine;

BranchTargetBuffer::BranchTargetBuffer(std::uint8_t btb_bits) {
    this->btb_bits = btb_bits;
    this->btb_size = power_of_2(this->btb_bits);
    this->btb = new BTBEntry[btb_size];
}

BranchTargetBuffer::~BranchTargetBuffer() {
    delete[] this->btb;
}

bool BranchTargetBuffer::get_pc_address(std::uint32_t btb_idx, std::uint32_t pc, std::uint32_t *address) {
    std::uint32_t tag = mask_bits(pc, btb_bits, 31);

    *address = btb[btb_idx].address;
    need_update = !(btb[btb_idx].valid && btb[btb_idx].tag == tag);

    return !need_update;
}

bool BranchTargetBuffer::get_btb_entry_valid(std::uint32_t btb_idx) const {
    return btb[btb_idx].valid;
}

std::uint32_t BranchTargetBuffer::get_btb_entry_address(std::uint32_t btb_idx) const {
    return btb[btb_idx].address;
}

std::uint32_t BranchTargetBuffer::get_btb_entry_tag(std::uint32_t btb_idx) const {
    return btb[btb_idx].tag;
}

void BranchTargetBuffer::update(std::uint32_t btb_idx, std::uint32_t inst_addr) {
    if (need_update) {
        btb[btb_idx].tag = mask_bits(inst_addr, btb_bits, 31);
        btb[btb_idx].valid = true;
        btb[btb_idx].address = inst_addr;
    }
}
