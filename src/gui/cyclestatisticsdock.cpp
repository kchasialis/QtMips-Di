#include <QVBoxLayout>
#include <QFormLayout>
#include "cyclestatisticsdock.h"
#include <vector>

CycleStatisticsDock::CycleStatisticsDock(QWidget *parent) : QDockWidget(parent) {
    setObjectName("Cycle Statistics");
    setWindowTitle("Cycle Statistics");

    std::vector<const char *> labels = {
        "Total Cycles:",
        "CPU Cycles:",
        "Core Stalls:",
        "L1 Data Stalls:",
        "L1 Program Stalls:",
        "L2 Unified Stalls:"
    };

    QWidget *content = new QWidget();

    auto *dock_layout = new QFormLayout(content);

    for (size_t i = 0 ; i < labels.size() ; i++) {
        cycle_stats_labels[i] = new QLabel("2147483647", this);
        cycle_stats_labels[i]->setFixedSize(cycle_stats_labels[i]->sizeHint());
        cycle_stats_labels[i]->setText("0");
        dock_layout->addRow(labels[i], cycle_stats_labels[i]);
    }
    content->setLayout(dock_layout);
    setWidget(content);
}

void CycleStatisticsDock::setup(machine::QtMipsMachine *machine) const {
    if (machine == nullptr) {
        // Reset data
        return;
    }

    connect(machine, &machine::QtMipsMachine::cycle_stats_update, this, &CycleStatisticsDock::cycle_stats_update);
}

void CycleStatisticsDock::cycle_stats_update(const machine::CycleStatistics &cycle_stats) {
    cycle_stats_labels[TOTAL_CYCLES]->setText(QString::number(cycle_stats.total_cycles));
    cycle_stats_labels[CPU_CYCLES]->setText(QString::number(cycle_stats.cpu_cycles));
    cycle_stats_labels[CORE_STALLS]->setText(QString::number(cycle_stats.core_stalls));
    cycle_stats_labels[L1_DATA_STALLS]->setText(QString::number(cycle_stats.l1_data_stalls));
    cycle_stats_labels[L1_PROGRAM_STALLS]->setText(QString::number(cycle_stats.l1_program_stalls));
    cycle_stats_labels[L2_UNIFIED_STALLS]->setText(QString::number(cycle_stats.l2_unified_stalls));
}
