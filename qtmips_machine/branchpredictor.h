#ifndef BRANCHPREDICTOR_H
#define BRANCHPREDICTOR_H

#include <cstdint>
#include "qtmipsmachine.h"

namespace machine {

class Instruction;

class BranchPredictor {
public:
    explicit BranchPredictor(std::uint8_t bht_bits);
    virtual ~BranchPredictor();

    virtual bool predict(const machine::Instruction &branch_inst) = 0;
    virtual void update_bht(bool branch_taken) = 0;
    // Debug purposes, should be removed.
//    virtual void print_current_state() = 0;
    virtual void set_bht_entry(std::size_t bht_index, QString val) = 0;

    // returns an index in the branch history table based on the instruction.
    size_t bht_idx(const Instruction &branch_instr, bool ro = false);
    uint8_t get_bht_entry(std::size_t bht_index) const;
    double get_precision() const;
    bool current_prediction() const;

protected:
    std::uint8_t *bht; // The branch history table.
    std::uint8_t bht_bits; // The # of bits used to index the history table.
    std::size_t bht_size; // The size of the table.
    std::uint32_t last_pos_predicted; // Position in the table that was accessed.
    std::uint32_t correct_predictions; // # of correct predictions.
    std::uint32_t predictions; // # of all predictions.
    bool current_p; // Last prediction that was made.

    // returns a mask containing n ones (1)
    static constexpr std::uint32_t mask_bits(std::uint32_t n) {
        return static_cast<std::uint32_t>(-(n != 0)) &
                (static_cast<std::uint32_t>(-1) >>
                 ((sizeof(std::uint32_t) * CHAR_BIT) - n));
    }

    // returns 2^exponent
    constexpr size_t power_of_2(std::uint32_t exponent) const {
        return exponent == 0 ? 1 : 2 * power_of_2(exponent - 1);
    }
};

class OneBitBranchPredictor : public BranchPredictor {
private:
    enum FSMStates : std::uint8_t {
        NOT_TAKEN = 0x0,
        TAKEN = 0x1
    };

public:
    explicit OneBitBranchPredictor(std::uint8_t bht_bits);

    bool predict(const machine::Instruction &branch_instr);
    void update_bht(bool branch_taken);
//    void print_current_state();
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

    bool predict(const machine::Instruction &branch_instr);
    void update_bht(bool branch_taken);
//    void print_current_state();
    void set_bht_entry(std::size_t bht_index, QString val);
};

}

#endif // BRANCHPREDICTOR_H
