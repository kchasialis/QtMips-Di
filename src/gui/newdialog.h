// SPDX-License-Identifier: GPL-2.0+
/*******************************************************************************
 * QtMips - MIPS 32-bit Architecture Subset Simulator
 *
 * Implemented to support following courses:
 *
 *   B35APO - Computer Architectures
 *   https://cw.fel.cvut.cz/wiki/courses/b35apo
 *
 *   B4M35PAP - Advanced Computer Architectures
 *   https://cw.fel.cvut.cz/wiki/courses/b4m35pap/start
 *
 * Copyright (c) 2017-2019 Karel Koci<cynerd@email.cz>
 * Copyright (c) 2019      Pavel Pisa <pisa@cmp.felk.cvut.cz>
 *
 * Faculty of Electrical Engineering (http://www.fel.cvut.cz)
 * Czech Technical University        (http://www.cvut.cz/)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 ******************************************************************************/

#ifndef NEWDIALOG_H
#define NEWDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include "ui_NewDialog.h"
#include "ui_NewDialogCache.h"
#include "machineconfig.h"
#include "memory.h"

class NewDialogCacheHandler;

class NewDialog : public QDialog {
    Q_OBJECT
public:
    NewDialog(QWidget *parent, QSettings *settings);
    ~NewDialog();

    void switch2custom();
    void set_default_settings(bool);
    NewDialogCacheHandler *l1_data_cache_handler();
    NewDialogCacheHandler *l1_program_cache_handler();
    Ui::NewDialogCache *l2_cache_dialog();

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void cancel();
    void create();
    void create_empty();
    void browse_elf();
    void elf_change(QString val);
    void set_preset();
    void pipelined_change(bool);
    void hazard_unit_change();
    void branch_unit_change();
    void mem_protec_exec_change(bool);
    void mem_protec_write_change(bool);
    void mem_time_read_change(int);
    void mem_time_write_change(int);
    void mem_time_burst_change(int);
    void osemu_enable_change(bool);
    void osemu_known_syscall_stop_change(bool);
    void osemu_unknown_syscall_stop_change(bool);
    void osemu_interrupt_stop_change(bool);
    void osemu_exception_stop_change(bool);
    void browse_osemu_fs_root();
    void osemu_fs_root_change(QString val);
    void reset_at_compile_change(bool);

private:
    Ui::NewDialog *ui;
    Ui::NewDialogCache *ui_l1_p_cache, *ui_l1_d_cache;
    Ui::NewDialogCache *ui_l2_cache;
    QSettings *settings;
    bool default_settings; // Whether or not to use previous settings or default.

    machine::MachineConfig *config;
    void config_gui(); // Apply configuration to gui

    unsigned preset_number();
    void load_settings();
    void store_settings();
    NewDialogCacheHandler *l1_p_cache_handler, *l1_d_cache_handler;
    NewDialogCacheHandler *l2_u_cache_handler;
};

class NewDialogCacheHandler : QObject {
	Q_OBJECT
public:
	NewDialogCacheHandler(NewDialog *nd, Ui::NewDialogCache *ui);

    void set_config(machine::MachineConfigCache *config);

    void config_gui(int time_read = 1, int time_write = 1, int time_burst = 0);

private slots:
	void enabled(bool);
	void numsets();
	void blocksize();
	void degreeassociativity();
	void replacement(int);
	void writeback(int);
    void access_read(int);
    void access_write(int);
    void access_burst(int);

private:
	NewDialog *nd;
	Ui::NewDialogCache *cache_ui;
	machine::MachineConfigCache *config;
};

#endif // NEWDIALOG_H
