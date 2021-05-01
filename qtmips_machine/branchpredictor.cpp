#include "branchpredictor.h"

using namespace machine;

BranchPredictor::BranchPredictor(std::uint8_t bht_bits) {
    this->bht_bits = bht_bits;
    this->bht_size = power_of_2(this->bht_bits);
    this->bht = new std::uint8_t[this->bht_size]();
    this->predictions = 0;
    this->correct_predictions = 0;
}

BranchPredictor::~BranchPredictor() {
    delete[] this->bht;
}

bool BranchPredictor::current_prediction() const {
    return current_p;
}

size_t BranchPredictor::bht_idx(const Instruction &branch_instr) {
    std::uint32_t instr_bits = branch_instr.data();
    // MIPS instructions always contain 2 bits for byte offset.
    std::uint32_t pos = (instr_bits >> 2) & mask_bits(bht_bits);

    last_pos_predicted = pos;
    predictions++;

    return pos;
}

OneBitBranchPredictor::OneBitBranchPredictor(uint8_t bht_bits) : BranchPredictor(bht_bits) {}

bool OneBitBranchPredictor::predict(const Instruction &branch_instr) {
    size_t idx = bht_idx(branch_instr);
    current_p = (bht[idx] == FSMStates::TAKEN) ? true : false;

    return current_p;
}

void OneBitBranchPredictor::update_bht(bool branch_taken) {
    // If we predicted the wrong result, update the table.
    // Else, keep it as is.
    switch (bht[last_pos_predicted]) {
    case FSMStates::NOT_TAKEN:
        if (!branch_taken) {
           correct_predictions++;
        } else {
            bht[last_pos_predicted] = FSMStates::TAKEN;
        }
        break;
    case FSMStates::TAKEN:
        if (branch_taken) {
           correct_predictions++;
        } else {
            bht[last_pos_predicted] = FSMStates::NOT_TAKEN;
        }
        break;
    default:
        // This should never happen, it means we have a bug.
        assert(0);
    }
}

void OneBitBranchPredictor::print_current_state() {
    QMap<std::uint8_t, QString> print_map;

    print_map.insert(FSMStates::NOT_TAKEN, "Not Taken");
    print_map.insert(FSMStates::TAKEN, "Taken");
}

TwoBitBranchPredictor::TwoBitBranchPredictor(uint8_t bht_bits) : BranchPredictor(bht_bits) {}

bool TwoBitBranchPredictor::predict(const Instruction &branch_instr) {
    size_t idx = bht_idx(branch_instr);
    current_p = (bht[idx] == FSMStates::WEAKLY_T || bht[idx] == FSMStates::STRONGLY_T) ?
              true : false;

    return current_p;
}

void TwoBitBranchPredictor::update_bht(bool branch_taken) {
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
        bht[last_pos_predicted] = branch_taken ?
                                  FSMStates::STRONGLY_T : FSMStates::WEAKLY_NT;
        if (branch_taken) correct_predictions++;
        break;
    case FSMStates::STRONGLY_T:
        bht[last_pos_predicted] = branch_taken ?
                                  FSMStates::STRONGLY_T : FSMStates::WEAKLY_T;

        if (branch_taken) correct_predictions++;
        break;
    default:
        // This should never happen, it means we have a bug.
        assert(0);
    }
}

void TwoBitBranchPredictor::print_current_state() {
    QMap<std::uint8_t, QString> print_map;

    print_map.insert(FSMStates::STRONGLY_NT, "Strongly Not Taken");
    print_map.insert(FSMStates::WEAKLY_NT, "Weakly Not Taken");
    print_map.insert(FSMStates::WEAKLY_T, "Weakly Taken");
    print_map.insert(FSMStates::STRONGLY_T, "Strongly Taken");
}
