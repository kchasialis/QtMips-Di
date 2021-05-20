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

    // returns then new pc.
    virtual std::uint32_t predict(const machine::Instruction &instr, std::uint32_t current_pc) = 0;
    virtual void update_bht(bool branch_taken) = 0;
    virtual void set_bht_entry(std::size_t bht_index, QString val) = 0;

    // returns an index in the branch history table based on the instruction.
    size_t bht_idx(const Instruction &branch_instr, bool ro = false);
    uint8_t get_bht_entry(std::size_t bht_index) const;
    double get_precision() const;
    bool last_prediction() const;
    void correct_predictions_inc();

protected:
    std::shared_ptr<BranchTargetBuffer> *btb_impl;
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

    std::uint32_t predict(const machine::Instruction &instr, std::uint32_t current_pc);
    void update_bht(bool branch_taken);
    void set_bht_entry(std::size_t bht_index, QString val);
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

    std::uint32_t predict(const machine::Instruction &instr, std::uint32_t current_pc);
    void update_bht(bool branch_taken);
    void set_bht_entry(std::size_t bht_index, QString val);
};

}

#endif // BRANCHPREDICTOR_H
