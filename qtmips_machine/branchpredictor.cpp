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

uint8_t BranchPredictor::get_bht_entry(std::uint32_t bht_idx) const {
    Q_ASSERT(bht_idx < bht_size);

    return bht[bht_idx];
}

bool BranchPredictor::get_btb_entry_valid(std::uint32_t btb_idx) const {
    return btb_impl->get_btb_entry_valid(btb_idx);
}

std::uint32_t BranchPredictor::get_btb_entry_address(std::uint32_t btb_idx) const {
    return btb_impl->get_btb_entry_address(btb_idx);
}

std::uint32_t BranchPredictor::get_btb_entry_tag(std::uint32_t btb_idx) const {
    return btb_impl->get_btb_entry_tag(btb_idx);
}

double BranchPredictor::get_precision() const {
    return predictions > 0.0 ? (double) correct_predictions / (double) predictions * 100.0 : 0.0;
}

bool BranchPredictor::last_prediction() const {
    return last_p;
}

std::uint32_t BranchPredictor::bht_idx(std::uint32_t pc, bool ro) {
    // MIPS instructions always contain 2 bits for byte offset.
    std::uint32_t pos;

    pc = pc >> 2;
    pos = mask_bits(pc, 0, bht_bits - 1);

    // ro means we only want to get the bht index and not to update anything on the object.
    if (!ro) {
        last_pos_predicted = pos;
        predictions++;
    }

    return pos;
}

std::uint32_t BranchPredictor::predict(const machine::Instruction &bj_instr, std::uint32_t pc) {
    std::uint32_t address, idx;

    last_jump = bj_instr.flags() & IMF_JUMP;
    if (last_jump) {
        idx = bht_idx(pc, true);
        return btb_impl->get_pc_address(idx, pc, &address) ? address : (pc + 4);
    } else if (bj_instr.flags() & IMF_BRANCH) {
        idx = bht_idx(pc);
        last_p = get_prediction(idx) && btb_impl->get_pc_address(idx, pc, &address);
        return last_p ? address : (pc + 4);
    } else {
        SANITY_ASSERT(0, "Debug me :)");
        return -1;
    }
}

OneBitBranchPredictor::OneBitBranchPredictor(uint8_t bht_bits) : BranchPredictor(bht_bits) {}

bool OneBitBranchPredictor::get_prediction(std::uint32_t bht_idx) {
    return bht[bht_idx] == FSMStates::TAKEN;
}

void OneBitBranchPredictor::update_bht(bool branch_taken, std::uint32_t correct_address) {
    if (branch_taken == last_p) {
        correct_predictions++;
    } else if (!last_jump) {
        // If we predicted the wrong result (this is checked before calling)
        // and the last instruction was not a jump, update the table (and possibly BTB).
        switch (bht[last_pos_predicted]) {
        case FSMStates::NOT_TAKEN:
            bht[last_pos_predicted] = FSMStates::TAKEN;
            break;
        case FSMStates::TAKEN:
            // It was taken, also update the BTB.
            btb_impl->update(last_pos_predicted, correct_address);
            bht[last_pos_predicted] = FSMStates::NOT_TAKEN;
            break;
        default:
            SANITY_ASSERT(0, "Debug me :)");
        }
    } else {
        // Last instruction was a jump, we do not update the table, just the BTB.
        btb_impl->update(last_pos_predicted, correct_address);
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

void TwoBitBranchPredictor::update_bht(bool branch_taken, std::uint32_t correct_address) {
    if (branch_taken == last_p) {
        correct_predictions++;
    }

    if (!last_jump) {
        switch (bht[last_pos_predicted]) {
        case FSMStates::STRONGLY_NT:
            bht[last_pos_predicted] = !branch_taken ?
                                      FSMStates::STRONGLY_NT : FSMStates::WEAKLY_NT;
            break;
        case FSMStates::WEAKLY_NT:
            bht[last_pos_predicted] = !branch_taken ?
                                      FSMStates::STRONGLY_NT : FSMStates::WEAKLY_T;
            break;
        case FSMStates::WEAKLY_T:
            // On taken branches we also update BTB.
            btb_impl->update(last_pos_predicted, correct_address);
            bht[last_pos_predicted] = branch_taken ?
                                      FSMStates::STRONGLY_T : FSMStates::WEAKLY_NT;
            break;
        case FSMStates::STRONGLY_T:
            btb_impl->update(last_pos_predicted, correct_address);
            bht[last_pos_predicted] = branch_taken ?
                                      FSMStates::STRONGLY_T : FSMStates::WEAKLY_T;
            break;
        default:
            SANITY_ASSERT(0, "Debug me :)");
        }
    } else {
        // Last instruction was a jump, we do not update the table, just the BTB.
        btb_impl->update(last_pos_predicted, correct_address);
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
