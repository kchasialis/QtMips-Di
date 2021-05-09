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

uint8_t BranchPredictor::get_bht_entry(std::size_t bht_index) const {
    Q_ASSERT(bht_index < bht_size);

    return bht[bht_index];
}

double BranchPredictor::get_precision() const {
    return predictions > 0.0 ? (double) correct_predictions / (double) predictions * 100.0 : 0.0;
}

bool BranchPredictor::current_prediction() const {
    return current_p;
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

//void OneBitBranchPredictor::print_current_state() {
//    QMap<std::uint8_t, QString> print_map;

//    print_map.insert(FSMStates::NOT_TAKEN, "Not Taken");
//    print_map.insert(FSMStates::TAKEN, "Taken");
//}

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
        SANITY_ASSERT(0, "Debug me :)");
    }
}

//void TwoBitBranchPredictor::print_current_state() {
//    QMap<std::uint8_t, QString> print_map;

//    print_map.insert(FSMStates::STRONGLY_NT, "Strongly Not Taken");
//    print_map.insert(FSMStates::WEAKLY_NT, "Weakly Not Taken");
//    print_map.insert(FSMStates::WEAKLY_T, "Weakly Taken");
//    print_map.insert(FSMStates::STRONGLY_T, "Strongly Taken");
//}

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
