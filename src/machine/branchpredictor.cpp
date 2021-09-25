#include "branchpredictor.h"
#include "branchtargetbuffer.h"

using namespace machine;

BranchPredictor::BranchPredictor(std::uint8_t bht_bits) {
    this->btb_impl = std::make_shared<BranchTargetBuffer>(bht_bits);
    this->bht_bits = bht_bits;
    this->bht_size = power_of_2(this->bht_bits);
    this->bht = new std::uint8_t[this->bht_size]();
    this->predictions = 0;
    this->correct_predictions = 0;
}

BranchPredictor::~BranchPredictor() {
    delete[] this->bht;
}

std::uint32_t BranchPredictor::bht_idx(std::uint32_t pc, bool ro) {
    // MIPS instructions always contain 2 bits for byte offset.
    std::uint32_t pos;

    pc = pc >> 2;
    pos = mask_bits(pc, 0, bht_bits - 1);

    // ro means we only want to get the bht index and not to update anything on the object.
    if (!ro) {
        if (!l_jmp) {
            emit pred_accessed_bht(pos);
        }
        predictions++;
    }

    return pos;
}

std::uint32_t BranchPredictor::predict(const machine::Instruction &bj_instr, std::uint32_t pc) {
    std::uint32_t address, idx;

    emit pred_inst_addr_value(pc);
    emit pred_instr_value(bj_instr);

    l_jmp = bj_instr.flags() & IMF_JUMP;
    idx = bht_idx(pc);

    if (l_jmp) {
        j_info.btb_miss = !btb_impl->pc_address(idx, pc, &address);
        j_info.pos_jmp = idx;

        return j_info.btb_miss ? (pc + 4) : address;
    } else if (bj_instr.flags() & IMF_BRANCH) {
        BranchInfo b_info;

        b_info.inst_addr = pc;
        b_info.branch = get_prediction(idx);
        b_info.btb_miss = false;
        if (b_info.branch) {
            b_info.btb_miss = !btb_impl->pc_address(idx, pc, &address);
            b_info.branch = !b_info.btb_miss;
        }
        b_info.pos_branch = idx;

        enqueue(b_info);

        return b_info.branch ? address : (pc + 4);
    } else {
        SANITY_ASSERT(0, "Debug me :)");
        return -1;
    }
}

uint8_t BranchPredictor::bht_entry(std::uint32_t bht_idx) const {
    Q_ASSERT(bht_idx < bht_size);

    return bht[bht_idx];
}

bool BranchPredictor::btb_entry_valid(std::uint32_t btb_idx) const {
    return btb_impl->btb_entry_valid(btb_idx);
}

std::uint32_t BranchPredictor::btb_entry_address(std::uint32_t btb_idx) const {
    return btb_impl->btb_entry_address(btb_idx);
}

std::uint32_t BranchPredictor::btb_entry_tag(std::uint32_t btb_idx) const {
    return btb_impl->btb_entry_tag(btb_idx);
}

double BranchPredictor::accuracy() const {
    return predictions > 0 ? ((double) correct_predictions / (double) predictions * 100.0) : 100.0;
}

bool BranchPredictor::prediction() const {
    return l_jmp ? !j_info.btb_miss : b_infos[0].branch;
}

std::uint32_t BranchPredictor::pos_predicted() const {
    return l_jmp ? !j_info.pos_jmp : b_infos[0].pos_branch;
}

const BranchTargetBuffer *BranchPredictor::btb() const {
    return btb_impl.get();
}

void BranchPredictor::handle_update_jump(std::uint32_t correct_address) {
    if (j_info.btb_miss) {
        // Last instruction was a jump and we had a btb miss, update the btb.
        btb_impl->update(j_info.pos_jmp, correct_address);
    } else {
        correct_predictions++;
    }
}

void BranchPredictor::enqueue(const BranchInfo &b_info) {
    b_infos.append(b_info);
}

BranchPredictor::BranchInfo BranchPredictor::dequeue() {
    BranchInfo ret = b_infos[0];

    remove(0);

    return ret;
}

void BranchPredictor::remove(std::uint32_t idx) {
    b_infos.remove(idx);
}

void BranchPredictor::remove(const InstAddr &inst_addr) {
    int idx = 0;

    for (int i = 0 ; i < b_infos.size() ; i++) {
        if (b_infos[i].inst_addr.val == inst_addr.val) {
            idx = i;
        }
    }

    remove(idx);
}

OneBitBranchPredictor::OneBitBranchPredictor(uint8_t bht_bits) : BranchPredictor(bht_bits) {}

bool OneBitBranchPredictor::get_prediction(std::uint32_t bht_idx) {
    return bht[bht_idx] == FSMStates::TAKEN;
}

void OneBitBranchPredictor::update_bht(bool branch, std::uint32_t correct_address) {
    if (!l_jmp) {
        const BranchInfo &b_info = dequeue();
        bool updated_btb = false;

        if (branch == b_info.branch) {
            correct_predictions++;
        } else {
            // Last instruction was a branch and we predicted the wrong result or we had a btb miss.
            switch (bht[b_info.pos_branch]) {
            case FSMStates::NOT_TAKEN:
                // It was taken, also update BTB.
                btb_impl->update(b_info.pos_branch, correct_address);
                updated_btb = true;
                bht[b_info.pos_branch] = FSMStates::TAKEN;
                break;
            case FSMStates::TAKEN:
                bht[b_info.pos_branch] = FSMStates::NOT_TAKEN;
                break;
            default:
                SANITY_ASSERT(0, "Debug me :)");
            }
            emit pred_updated_bht(b_info.pos_branch);
        }

        if (!updated_btb && b_info.btb_miss) {
            btb_impl->update(b_info.pos_branch, correct_address);
        }
    } else {
        BranchPredictor::handle_update_jump(correct_address);
    }
}

void OneBitBranchPredictor::set_bht_entry(std::uint32_t bht_idx, QString val) {
    Q_ASSERT(bht_idx < bht_size);
    if (val == "NT") {
        bht[bht_idx] = FSMStates::NOT_TAKEN;
    }
    else if (val == "T") {
        bht[bht_idx] = FSMStates::TAKEN;
    }
    else {
        SANITY_ASSERT(0, "Debug me :)");
    }
}

TwoBitBranchPredictor::TwoBitBranchPredictor(uint8_t bht_bits) : BranchPredictor(bht_bits) {}

bool TwoBitBranchPredictor::get_prediction(std::uint32_t bht_idx) {
    return bht[bht_idx] == FSMStates::WEAKLY_T || bht[bht_idx] == FSMStates::STRONGLY_T;
}

void TwoBitBranchPredictor::update_bht(bool branch, std::uint32_t correct_address) {

    if (!l_jmp) {
        const BranchInfo &b_info = dequeue();
        bool updated_btb = false;

        if (branch == b_info.branch) {
            correct_predictions++;
        } else {
            // Last instruction was a branch and we predicted the wrong result or we had a btb miss.
            switch (bht[b_info.pos_branch]) {
            case FSMStates::STRONGLY_NT:
                // On taken branches we also update BTB.
                btb_impl->update(b_info.pos_branch, correct_address);
                updated_btb = true;
                bht[b_info.pos_branch] = !branch ? FSMStates::STRONGLY_NT : FSMStates::WEAKLY_NT;
                break;
            case FSMStates::WEAKLY_NT:
                btb_impl->update(b_info.pos_branch, correct_address);
                updated_btb = true;
                bht[b_info.pos_branch] = !branch ? FSMStates::STRONGLY_NT : FSMStates::WEAKLY_T;
                break;
            case FSMStates::WEAKLY_T:
                bht[b_info.pos_branch] = branch ? FSMStates::STRONGLY_T : FSMStates::WEAKLY_NT;
                break;
            case FSMStates::STRONGLY_T:
                bht[b_info.pos_branch] = branch ? FSMStates::STRONGLY_T : FSMStates::WEAKLY_T;
                break;
            default:
                SANITY_ASSERT(0, "Debug me :)");
            }
            emit pred_updated_bht(b_info.pos_branch);
        }
        
        if (!updated_btb && b_info.btb_miss) {
            btb_impl->update(b_info.pos_branch, correct_address);
        }        
    } else {
        BranchPredictor::handle_update_jump(correct_address);
    }
}

void TwoBitBranchPredictor::set_bht_entry(std::uint32_t bht_idx, QString val) {
    Q_ASSERT(bht_idx < bht_size);
    if (val == "STRONGLY_NT") {
        bht[bht_idx] = FSMStates::STRONGLY_NT;
    }
    else if (val == "WEAKLY_NT") {
        bht[bht_idx] = FSMStates::WEAKLY_NT;
    }
    else if (val == "WEAKLY_T") {
        bht[bht_idx] = FSMStates::WEAKLY_T;
    }
    else if (val == "STRONGLY_T") {
        bht[bht_idx] = FSMStates::STRONGLY_T;
    }
    else {
        SANITY_ASSERT(0, "Debug me :)");
    }
}
