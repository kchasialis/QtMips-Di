#ifndef BRANCHPREDICTOR_H
#define BRANCHPREDICTOR_H

#include <cstdint>
#include <memory>
#include <QVector>
#include "qtmipsmachine.h"

namespace machine {

class Instruction;
class BranchTargetBuffer;

class BranchPredictor : public QObject {
    Q_OBJECT

public:
    struct InstAddr {
        std::uint32_t val;

        explicit InstAddr(std::uint32_t v) : val(v) {}
        InstAddr(const InstAddr &other) : val(other.val) {}
        InstAddr &operator=(std::uint32_t val) {
            this->val = val;
            return *this;
        }
        InstAddr &operator=(const InstAddr &inst_addr) {
            this->val = inst_addr.val;
            return *this;
        }
        bool operator!=(const InstAddr &inst_addr) {
            return !(*this == inst_addr);
        }
        bool operator==(const InstAddr &inst_addr) {
            return val == inst_addr.val;
        }
    };

    struct BranchInfo {
        // Valuable information about a prediction that was made.
        InstAddr inst_addr;
        uint32_t pred_addr;
        uint32_t pos_branch; // Last position of the table that was predicted for a branch instruction.
        bool btb_miss; // Wether or not we had a btb miss.
        bool branch; // Last prediction that was made (taken or not taken).

        BranchInfo() : inst_addr(0), pred_addr(0), pos_branch(0), btb_miss(false), branch(false) {}
    };

    struct JumpInfo {
        uint32_t pc; // PC of jump instruction.
        uint32_t pred_addr; // The predicted address.
        uint32_t pos_jmp; // Last position of the table that was predicted for a jump instruction.
        bool btb_miss; // Whether or not jump was taken (the only condition is if we had a btb miss).
    };

    explicit BranchPredictor(std::uint8_t bht_bits);
    ~BranchPredictor() override;

    virtual bool get_prediction(std::uint32_t bht_idx) = 0;
    virtual void update_bht(bool branch_taken, bool is_branch, uint32_t correct_address) = 0;
    virtual void set_bht_entry(std::uint32_t bht_idx, QString val) = 0;

    // returns an index in the branch history table based on the instruction.
    std::uint32_t bht_idx(std::uint32_t pc, bool ro = false);
    // returns then new pc.
    std::uint32_t predict(const machine::Instruction &bj_instr, std::uint32_t pc);
    uint8_t bht_entry(std::uint32_t bht_idx) const;
    bool btb_entry_valid(std::uint32_t btb_idx) const;
    std::uint32_t btb_entry_address(std::uint32_t btb_idx) const;
    std::uint32_t btb_entry_tag(std::uint32_t btb_idx) const;
    double accuracy() const;
    uint32_t prediction(bool is_branch) const;
//    std::uint32_t pos_predicted() const;
    const BranchTargetBuffer *btb() const;
    void handle_update_jump(std::uint32_t correct_address);
    void enqueue(const BranchInfo &b_info);
    BranchPredictor::BranchInfo dequeue();
    void remove(std::uint32_t idx);
    void remove(const InstAddr &bj_instr);
    void reset();

signals:
    void pred_accessed_bht(std::int32_t);
    void pred_updated_bht(std::int32_t);
    void pred_inst_addr_value(std::uint32_t);
    void pred_instr_value(const machine::Instruction &bj_instr);
    void pred_reset();

protected:
    std::shared_ptr<BranchTargetBuffer> btb_impl;
    std::uint8_t bht_bits; // The # of bits used to index the history table.
    size_t bht_size; // The size of the table.
    std::uint8_t *bht; // The branch history table.
    std::uint32_t correct_predictions; // # of correct predictions.
    std::uint32_t predictions; // # of all predictions.
    JumpInfo j_info;
    QVector<BranchInfo> b_infos;
};

class OneBitBranchPredictor : public BranchPredictor {
private:
    enum FSMStates : std::uint8_t {
        NOT_TAKEN = 0x0,
        TAKEN = 0x1
    };

public:
    explicit OneBitBranchPredictor(std::uint8_t bht_bits);

    bool get_prediction(std::uint32_t bht_idx) override;
    void update_bht(bool branch, bool is_branch, uint32_t correct_address) override;
    void set_bht_entry(std::uint32_t bht_idx, QString val) override;
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

    bool get_prediction(std::uint32_t bht_idx) override;
    void update_bht(bool branch, bool is_branch, uint32_t correct_address) override;
    void set_bht_entry(std::uint32_t bht_idx, QString val) override;
};

}

#endif // BRANCHPREDICTOR_H
