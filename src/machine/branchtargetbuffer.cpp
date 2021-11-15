#include "branchtargetbuffer.h"

using namespace machine;

BranchTargetBuffer::BranchTargetBuffer(std::uint8_t btb_bits) {
    this->btb_bits = btb_bits;
    this->btb_size = power_of_2(this->btb_bits);
    this->btb = new BTBEntry[btb_size];
}

BranchTargetBuffer::~BranchTargetBuffer() {
    delete[] this->btb;
}

bool BranchTargetBuffer::pc_address(uint32_t pc, uint32_t *address) {
    uint32_t tag, btb_idx;
    pc = pc >> 2;
    btb_idx = pc % btb_size;
    tag = pc / btb_size;

    *address = btb[btb_idx].address;

    emit pred_accessed_btb(btb_idx);

    return btb[btb_idx].valid && btb[btb_idx].tag == tag;
}

bool BranchTargetBuffer::btb_entry_valid(uint32_t btb_idx) const {
    return btb[btb_idx].valid;
}

std::uint32_t BranchTargetBuffer::btb_entry_address(uint32_t btb_idx) const {
    return btb[btb_idx].address;
}

std::uint32_t BranchTargetBuffer::btb_entry_tag(uint32_t btb_idx) const {
    return btb[btb_idx].tag;
}

void BranchTargetBuffer::update(uint32_t pc, uint32_t inst_addr) {
    uint32_t btb_idx;

    pc = pc >> 2;
    btb_idx = pc % btb_size;

    btb[btb_idx].tag = pc / btb_size;
    btb[btb_idx].valid = true;
    btb[btb_idx].address = inst_addr;

    emit pred_updated_btb(btb_idx);
}

void BranchTargetBuffer::reset() {
    for (size_t i = 0 ; i < btb_size ; i++) {
        this->btb[i].address = 0;
        this->btb[i].tag = 0;
        this->btb[i].valid = false;
    }
}
