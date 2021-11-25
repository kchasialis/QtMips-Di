#ifndef CYCLESTATISTICS_H
#define CYCLESTATISTICS_H

#include <cstdint>

namespace machine {
    struct CycleStatistics {
        uint64_t total_cycles;
        uint64_t instructions;
        uint64_t memory_cycles;
        uint64_t data_hazard_stalls;
        uint64_t control_hazard_stalls;
        uint64_t l1_data_stall_cycles;
        uint64_t l1_data_stall_cycles_total;
        uint64_t l1_program_stall_cycles;
        uint64_t l1_program_stall_cycles_total;
        uint64_t l2_unified_stall_cycles;
        uint64_t l2_unified_stall_cycles_total;

        CycleStatistics() : total_cycles(0), instructions(0), memory_cycles(0), data_hazard_stalls(0),
                            control_hazard_stalls(0), l1_data_stall_cycles(0), l1_data_stall_cycles_total(0),
                            l1_program_stall_cycles(0), l1_program_stall_cycles_total(0),
                            l2_unified_stall_cycles(0), l2_unified_stall_cycles_total(0) {}
    };
}

#endif
