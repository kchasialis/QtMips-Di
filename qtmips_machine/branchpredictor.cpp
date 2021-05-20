#include "branchpredictor.h"
#include "branchtargetbuffer.h"

using namespace machine;

BranchPredictor::BranchPredictor(std::uint8_t bht_bits) {
    this->btb_impl = new BranchTargetBuffer(bht_bits);
    this->bht_bits = bht_bits;
    this->bht_size = power_of_2(this->bht_bits);
    this->bht = new std::uint8_t[this->bht_size]();
    this->predictions = 0;
    this->correct_predictions = 0;
}

BranchPredictor::~BranchPredictor() {
    delete[] this->bht;
}

uint8_t BranchPredictor::get_bht_entry(std::size_t bht_index) const {
    Q_ASSERT(bht_index < bht_size);

    return bht[bht_index];
}

double BranchPredictor::get_precision() const {
    return predictions > 0.0 ? (double) correct_predictions / (double) predictions * 100.0 : 0.0;
}

bool BranchPredictor::last_prediction() const {
    return last_p;
}

void BranchPredictor::correct_predictions_inc() {
    correct_predictions++;
}

size_t BranchPredictor::bht_idx(const Instruction &branch_instr, bool ro) {
    std::uint32_t instr_bits = branch_instr.data();
    // MIPS instructions always contain 2 bits for byte offset.
    std::uint32_t pos = (instr_bits >> 2) & mask_bits(bht_bits);

    // ro means we only want to get the bht index and not to update anything on the object.
    if (!ro) {
        last_pos_predicted = pos;
        predictions++;
    }

    return pos;
}

OneBitBranchPredictor::OneBitBranchPredictor(uint8_t bht_bits) : BranchPredictor(bht_bits) {}

std::uint32_t OneBitBranchPredictor::predict(const Instruction &instr, std::uint32_t current_pc) {
    std::uint32_t address;
    size_t idx;

    last_jump = bj_instr.flags() & IMF_JUMP;
    if (last_jump) {
        return btb_impl->get_pc_address(bht_idx, current_pc, &address) ? address : (current_pc + 4);
    } else if (bj_instr.flags() & IMF_BRANCH) {
        idx = bht_idx(bj_instr);
        last_p = bht[idx] == FSMStates::TAKEN && btb_impl->get_pc_address(bht_idx, current_pc, &address);
        return last_p ? address : (current_pc + 4);
    } else {
        SANITY_ASSERT(0, "Debug me :)");
        return -1;
    }
}

void OneBitBranchPredictor::update_bht(bool branch_taken, std::uint32_t bj_address) {
    // If we predicted the wrong result (this is checked before calling) and the last instruction was not a jump, update the table.
    if (!last_jump) {
        switch (bht[last_pos_predicted]) {
        case FSMStates::NOT_TAKEN:
            bht[last_pos_predicted] = FSMStates::TAKEN;
            break;
        case FSMStates::TAKEN:
            // It was taken, also update the BTB.
            btb_impl->update(last_pos_predicted, bj_address);
            bht[last_pos_predicted] = FSMStates::NOT_TAKEN;
            break;
        default:
            SANITY_ASSERT(0, "Debug me :)");
        }
    } else {
        btb_impl->update(last_pos_predicted, bj_address);
    }
}

void OneBitBranchPredictor::set_bht_entry(std::size_t bht_index, QString val) {
    Q_ASSERT(bht_index < bht_size);
    if (val == "NT") {
        bht[bht_index] = FSMStates::NOT_TAKEN;
    }
    else if (val == "T") {
        bht[bht_index] = FSMStates::TAKEN;
    }
    else {
        SANITY_ASSERT(0, "Debug me :)");
    }
}

TwoBitBranchPredictor::TwoBitBranchPredictor(uint8_t bht_bits) : BranchPredictor(bht_bits) {}

std::uint32_t TwoBitBranchPredictor::predict(const Instruction &instr, std::uint32_t current_pc) {
    std::uint32_t address;
    size_t idx;

    idx = bht_idx(bj_instr);
    current_p = (bht[idx] == FSMStates::WEAKLY_T || bht[idx] == FSMStates::STRONGLY_T) \
            && btb_impl->get_bj_address(bj_instr, &address);

    return current_p ? address : (last_pc + 4);
}

void TwoBitBranchPredictor::update_bht(bool branch_taken, std::uint32_t bj_address) {
    switch (bht[last_pos_predicted]) {
    case FSMStates::STRONGLY_NT:
        bht[last_pos_predicted] = !branch_taken ?
                                  FSMStates::STRONGLY_NT : FSMStates::WEAKLY_NT;
        if (!branch_taken) correct_predictions++;
        break;
    case FSMStates::WEAKLY_NT:
        bht[last_pos_predicted] = !branch_taken ?
                                  FSMStates::STRONGLY_NT : FSMStates::WEAKLY_T;
        if (!branch_taken) correct_predictions++;
        break;
    case FSMStates::WEAKLY_T:
        btb_impl->update(last_pos_predicted, bj_address);
        bht[last_pos_predicted] = branch_taken ?
                                  FSMStates::STRONGLY_T : FSMStates::WEAKLY_NT;
        if (branch_taken) correct_predictions++;
        break;
    case FSMStates::STRONGLY_T:
        btb_impl->update(last_pos_predicted, bj_address);
        bht[last_pos_predicted] = branch_taken ?
                                  FSMStates::STRONGLY_T : FSMStates::WEAKLY_T;

        if (branch_taken) correct_predictions++;
        break;
    default:
        SANITY_ASSERT(0, "Debug me :)");
    }
}

void TwoBitBranchPredictor::set_bht_entry(std::size_t bht_index, QString val) {
    Q_ASSERT(bht_index < bht_size);
    if (val == "STRONGLY_NT") {
        bht[bht_index] = FSMStates::STRONGLY_NT;
    }
    else if (val == "WEAKLY_NT") {
        bht[bht_index] = FSMStates::WEAKLY_NT;
    }
    else if (val == "WEAKLY_T") {
        bht[bht_index] = FSMStates::WEAKLY_T;
    }
    else if (val == "STRONGLY_T") {
        bht[bht_index] = FSMStates::STRONGLY_T;
    }
    else {
        SANITY_ASSERT(0, "Debug me :)");
    }
}
