#ifndef QTMIPS_CYCLESTATISTICSDOCK_H
#define QTMIPS_CYCLESTATISTICSDOCK_H

#include <QLabel>
#include <QTextItem>
#include <QTextEdit>
#include <QDockWidget>
#include "qtmipsmachine.h"

class CycleStatisticsDock : public QDockWidget {
    Q_OBJECT
public:
    explicit CycleStatisticsDock(QWidget *parent);

    void setup(machine::QtMipsMachine *machine) const;

public slots:
    void cycle_stats_update(const machine::CycleStatistics &stats);

private:
    enum CycleStatIndex {
        TOTAL_CYCLES,
        INSTRUCTIONS,
        CPI,
        DATA_HAZARD_STALLS,
        CONTROL_HAZARD_STALLS,
        DRAM_STALLS,
        L1_DATA_STALLS,
        L1_PROGRAM_STALLS,
        L2_UNIFIED_STALLS
    };

   QLabel *cycle_stats_labels[9]{};
};

#endif
