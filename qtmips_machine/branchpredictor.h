#ifndef BRANCHPREDICTOR_H
#define BRANCHPREDICTOR_H

#include <cstdint>
#include <memory>
#include "qtmipsmachine.h"

namespace machine {

class Instruction;
class BranchTargetBuffer;

class BranchPredictor {
public:
    explicit BranchPredictor(std::uint8_t bht_bits);
    virtual ~BranchPredictor();

    virtual bool get_prediction(std::uint32_t bht_idx) = 0;
    virtual void update_bht(bool branch_taken, std::uint32_t correct_address) = 0;
    virtual void set_bht_entry(std::uint32_t bht_idx, QString val) = 0;

    // returns an index in the branch history table based on the instruction.
    std::uint32_t bht_idx(std::uint32_t pc, bool ro = false);
    // returns then new pc.
    std::uint32_t predict(const machine::Instruction &bj_instr, std::uint32_t pc);
    uint8_t get_bht_entry(std::uint32_t bht_idx) const;
    bool get_btb_entry_valid(std::uint32_t btb_idx) const;
    std::uint32_t get_btb_entry_address(std::uint32_t btb_idx) const;
    std::uint32_t get_btb_entry_tag(std::uint32_t btb_idx) const;
    double get_precision() const;
    bool last_prediction() const;

protected:
    std::shared_ptr<BranchTargetBuffer> btb_impl;
    std::uint8_t bht_bits; // The # of bits used to index the history table.
    size_t bht_size; // The size of the table.
    std::uint8_t *bht; // The branch history table.
    std::uint32_t last_pos_predicted; // Position in the table that was accessed.
    std::uint32_t correct_predictions; // # of correct predictions.
    std::uint32_t predictions; // # of all predictions.
    bool last_p; // Last prediction that was made.
    bool last_jump; // If our last instruction was a jump we do not update the bht.
};

class OneBitBranchPredictor : public BranchPredictor {
private:
    enum FSMStates : std::uint8_t {
        NOT_TAKEN = 0x0,
        TAKEN = 0x1
    };

public:
    explicit OneBitBranchPredictor(std::uint8_t bht_bits);

    bool get_prediction(std::uint32_t bht_idx);
    void update_bht(bool branch_taken, std::uint32_t correct_address);
    void set_bht_entry(std::uint32_t bht_idx, QString val);
};


class TwoBitBranchPredictor : public BranchPredictor {
private:
    enum FSMStates : std::uint8_t {
        STRONGLY_NT = 0x00, // Strongly not taken
        WEAKLY_NT = 0x01, // Weakly not taken
        WEAKLY_T = 0x10, // Weakly taken
        STRONGLY_T = 0x11 // Strongly taken
    };

public:
    explicit TwoBitBranchPredictor(std::uint8_t bht_bits);

    bool get_prediction(std::uint32_t bht_idx);
    void update_bht(bool branch_taken, std::uint32_t correct_address);
    void set_bht_entry(std::uint32_t bht_idx, QString val);
};

}

#endif // BRANCHPREDICTOR_H
