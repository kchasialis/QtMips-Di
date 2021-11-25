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

uint32_t BranchPredictor::bht_idx(std::uint32_t pc, bool ro) {
    // MIPS instructions always contain 2 bits for byte offset.
    uint32_t pos;

    pc = pc >> 2;
    pos = pc % bht_size;

    // ro means we only want to get the bht index and not to update anything on the object.
    if (!ro)
        predictions++;

    return pos;
}

#include <QDebug>

std::uint32_t BranchPredictor::predict(const machine::Instruction &bj_instr, std::uint32_t pc, bool &accessed_btb) {
    std::uint32_t address, idx;

    emit pred_inst_addr_value(pc);
    emit pred_instr_value(bj_instr);

    bool jmp = bj_instr.flags() & IMF_JUMP;
    idx = bht_idx(pc);
    emit pred_accessed_bht(idx);

    if (jmp) {
        j_info.btb_miss = !btb_impl->pc_address(pc, &address);
        j_info.addr = pc;
        j_info.pos_jmp = idx;
        j_info.pred_addr = j_info.btb_miss ? (pc + 4) : address;

        accessed_btb = true;
        qDebug() << "accessing btb...";

        return j_info.pred_addr;
    } else if (bj_instr.flags() & IMF_BRANCH) {
        BranchInfo b_info;

        b_info.inst_addr = pc;
        b_info.branch = get_prediction(idx);
        b_info.btb_miss = false;
        accessed_btb = false;
        if (b_info.branch) {
            b_info.btb_miss = !btb_impl->pc_address(pc, &address);
            accessed_btb = true;
            b_info.branch = !b_info.btb_miss;
        }
        b_info.pos_branch = idx;
        b_info.pred_addr = b_info.branch ? address : (pc + 4);

        enqueue(b_info);

        return b_info.pred_addr;
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
    return predictions > 0 ? ((double) correct_predictions / (double) predictions) * 100.0 : 100.0;
}

uint32_t BranchPredictor::prediction(bool is_branch) const {
    return is_branch ? b_infos[0].pred_addr : j_info.pred_addr;
}

const BranchTargetBuffer *BranchPredictor::btb() const {
    return btb_impl.get();
}

void BranchPredictor::handle_update_jump(uint32_t correct_address) {
    if (j_info.btb_miss || j_info.pred_addr != correct_address) {
        // Last instruction was a jump and we had a btb miss, update the btb.
        btb_impl->update(j_info.addr, correct_address);
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

void BranchPredictor::reset() {
    for (size_t i = 0 ; i < this->bht_size ; i++) {
        this->bht[i] = 0;
    }
    this->predictions = 0;
    this->correct_predictions = 0;

    emit pred_updated_accuracy(accuracy());
}

OneBitBranchPredictor::OneBitBranchPredictor(uint8_t bht_bits) : BranchPredictor(bht_bits) {}

bool OneBitBranchPredictor::get_prediction(std::uint32_t bht_idx) {
    return bht[bht_idx] == FSMStates::TAKEN;
}

void OneBitBranchPredictor::update_bht(bool branch, bool is_branch, uint32_t correct_address) {
    if (is_branch) {
        const BranchInfo &b_info = dequeue();
        bool updated_btb = false;

        if (branch == b_info.branch) {
            correct_predictions++;
        } else {
            // Last instruction was a branch and we predicted the wrong result or we had a btb miss.
            switch (bht[b_info.pos_branch]) {
                case FSMStates::NOT_TAKEN:
                    // It was taken, also update BTB.
                    btb_impl->update(b_info.inst_addr.val, correct_address);
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
            btb_impl->update(b_info.inst_addr.val, correct_address);
        }
    } else {
        if (bht[j_info.pos_jmp] != FSMStates::TAKEN) {
            bht[j_info.pos_jmp] = FSMStates::TAKEN;
            emit pred_updated_bht(j_info.pos_jmp);
        }
        BranchPredictor::handle_update_jump(correct_address);
    }
    emit pred_updated_accuracy(accuracy());
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

void TwoBitBranchPredictor::update_bht(bool branch, bool is_branch, uint32_t correct_address) {
    if (is_branch) {
        const BranchInfo &b_info = dequeue();
        bool updated_btb = false;

        if (branch == b_info.branch) {
            correct_predictions++;
        }
        switch (bht[b_info.pos_branch]) {
            case FSMStates::STRONGLY_NT:
                if (branch) {
                    // Branch was taken but our prediction was STRONGLY_NT, we need to update btb
                    btb_impl->update(b_info.inst_addr.val, correct_address);
                    updated_btb = true;
                    bht[b_info.pos_branch] = FSMStates::WEAKLY_NT;
                    emit pred_updated_bht(b_info.pos_branch);
                }
                break;
            case FSMStates::WEAKLY_NT:
                if (branch) {
                    btb_impl->update(b_info.inst_addr.val, correct_address);
                    updated_btb = true;
                    bht[b_info.pos_branch] = FSMStates::WEAKLY_T;
                    emit pred_updated_bht(b_info.pos_branch);
                } else {
                    bht[b_info.pos_branch] = FSMStates::STRONGLY_NT;
                    emit pred_updated_bht(b_info.pos_branch);
                }
                break;
            case FSMStates::WEAKLY_T:
                bht[b_info.pos_branch] = branch ? FSMStates::STRONGLY_T : FSMStates::WEAKLY_NT;
                emit pred_updated_bht(b_info.pos_branch);
                break;
            case FSMStates::STRONGLY_T:
                if (!branch) {
                    bht[b_info.pos_branch] = FSMStates::WEAKLY_T;
                    emit pred_updated_bht(b_info.pos_branch);
                }
                break;
            default:
                SANITY_ASSERT(0, "Debug me :)");
        }

        if (!updated_btb && b_info.btb_miss) {
            btb_impl->update(b_info.inst_addr.val, correct_address);
        }        
    } else {
        if (bht[j_info.pos_jmp] != FSMStates::STRONGLY_T) {
            bht[j_info.pos_jmp] = FSMStates::STRONGLY_T;
            emit pred_updated_bht(j_info.pos_jmp);
        }
        BranchPredictor::handle_update_jump(correct_address);
    }
    emit pred_updated_accuracy(accuracy());
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
