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

#include "newdialog.h"
#include "mainwindow.h"
#include "qtmipsexception.h"

#ifdef __EMSCRIPTEN__
#include <QFileInfo>
#include "qhtml5file.h"
#endif

#define MIN_BHT_BITS 5
#define MAX_BHT_BITS 14

NewDialog::NewDialog(QWidget *parent, QSettings *settings) : QDialog(parent) {
    QMessageBox m_box;
    QAbstractButton *prev;

    setWindowTitle("New machine");

    this->settings = settings;
    config = nullptr;

    m_box.setText("Load previous or default settings?");
    prev = m_box.addButton(tr("Previous"), QMessageBox::YesRole);
    m_box.addButton(tr("Default"), QMessageBox::NoRole);
    m_box.exec();
    set_default_settings(m_box.clickedButton() != prev);

    ui = new Ui::NewDialog();
    ui->setupUi(this);

    ui_l1_p_cache = new Ui::NewDialogCache();
    ui_l1_p_cache->setupUi(ui->tab_l1_program_cache);
    ui_l1_p_cache->write_policy->hide();
    // We assume L1 caches access time = CPU time and cannot be altered.
    ui_l1_p_cache->access_time->hide();

    ui_l1_d_cache = new Ui::NewDialogCache();
    ui_l1_d_cache->setupUi(ui->tab_l1_data_cache);
    // We assume L1 caches access time = CPU time and cannot be altered.
    ui_l1_d_cache->access_time->hide();

    ui_l2_cache = new Ui::NewDialogCache();
    ui_l2_cache->setupUi(ui->tab_l2_unified_cache);

    ui->predictor_bits->addItem("1");
    ui->predictor_bits->addItem("2");
    for (size_t i = MIN_BHT_BITS ; i <= MAX_BHT_BITS ; i++) {
        ui->bht_bits->addItem(QString::number(i));
    }
    ui->resolution->addItem("ID");
    ui->resolution->addItem("EX");

    connect(ui->pushButton_start_empty, SIGNAL(clicked(bool)), this, SLOT(create_empty()));
    connect(ui->pushButton_load, SIGNAL(clicked(bool)), this, SLOT(create()));
    connect(ui->pushButton_cancel, SIGNAL(clicked(bool)), this, SLOT(cancel()));
    connect(ui->pushButton_browse, SIGNAL(clicked(bool)), this, SLOT(browse_elf()));
    connect(ui->elf_file, SIGNAL(textChanged(QString)), this, SLOT(elf_change(QString)));
    connect(ui->preset_no_pipeline, SIGNAL(toggled(bool)), this, SLOT(set_preset()));
    connect(ui->preset_no_pipeline_cache, SIGNAL(toggled(bool)), this, SLOT(set_preset()));
    connect(ui->preset_pipelined_bare, SIGNAL(toggled(bool)), this, SLOT(set_preset()));
    connect(ui->preset_pipelined, SIGNAL(toggled(bool)), this, SLOT(set_preset()));
    connect(ui->reset_at_compile, SIGNAL(clicked(bool)), this, SLOT(reset_at_compile_change(bool)));

    connect(ui->pipelined, SIGNAL(clicked(bool)), this, SLOT(pipelined_change(bool)));
    connect(ui->data_hazard_unit, SIGNAL(clicked(bool)), this, SLOT(data_hazard_unit_change()));
    connect(ui->data_hazard_stall, SIGNAL(clicked(bool)), this, SLOT(data_hazard_unit_change()));
    connect(ui->data_hazard_stall_forward, SIGNAL(clicked(bool)), this, SLOT(data_hazard_unit_change()));
    // Connections for branch unit are being done on load_settings() function.

    // Connections need to be done here. A bug is caused otherwise.
    connect(ui->none, SIGNAL(clicked(bool)), this, SLOT(control_hazard_unit_change()));
    connect(ui->stall, SIGNAL(clicked(bool)), this, SLOT(control_hazard_unit_change()));
    connect(ui->delay_slot, SIGNAL(clicked(bool)), this, SLOT(control_hazard_unit_change()));
    connect(ui->branch_predictor, SIGNAL(clicked(bool)), this, SLOT(control_hazard_unit_change()));
    connect(ui->predictor_bits, SIGNAL(currentIndexChanged(QString)), this, SLOT(control_hazard_unit_change()));
    connect(ui->bht_bits, SIGNAL(currentIndexChanged(QString)), this, SLOT(control_hazard_unit_change()));
    connect(ui->resolution, SIGNAL(currentIndexChanged(QString)), this, SLOT(control_hazard_unit_change()));

    connect(ui->mem_protec_exec, SIGNAL(clicked(bool)), this, SLOT(mem_protec_exec_change(bool)));
    connect(ui->mem_protec_write, SIGNAL(clicked(bool)), this, SLOT(mem_protec_write_change(bool)));
    connect(ui->mem_access_read, SIGNAL(valueChanged(int)), this, SLOT(mem_time_read_change(int)));
    connect(ui->mem_access_write, SIGNAL(valueChanged(int)), this, SLOT(mem_time_write_change(int)));
    connect(ui->mem_access_burst, SIGNAL(valueChanged(int)), this, SLOT(mem_time_burst_change(int)));

    connect(ui->osemu_enable, SIGNAL(clicked(bool)), this, SLOT(osemu_enable_change(bool)));
    connect(ui->osemu_known_syscall_stop, SIGNAL(clicked(bool)), this, SLOT(osemu_known_syscall_stop_change(bool)));
    connect(ui->osemu_unknown_syscall_stop, SIGNAL(clicked(bool)), this, SLOT(osemu_unknown_syscall_stop_change(bool)));
    connect(ui->osemu_interrupt_stop, SIGNAL(clicked(bool)), this, SLOT(osemu_interrupt_stop_change(bool)));
    connect(ui->osemu_exception_stop, SIGNAL(clicked(bool)), this, SLOT(osemu_exception_stop_change(bool)));
    connect(ui->osemu_fs_root_browse, SIGNAL(clicked(bool)), this, SLOT(browse_osemu_fs_root()));
    connect(ui->osemu_fs_root, SIGNAL(textChanged(QString)), this, SLOT(osemu_fs_root_change(QString)));

    l1_p_cache_handler = new NewDialogCacheHandler(this, ui_l1_p_cache);
    l1_d_cache_handler = new NewDialogCacheHandler(this, ui_l1_d_cache);
    l2_u_cache_handler = new NewDialogCacheHandler(this, ui_l2_cache);

    // TODO remove this block when protections are implemented
    ui->mem_protec_exec->setVisible(false);
    ui->mem_protec_write->setVisible(false);

    load_settings(); // Also configures gui
}

NewDialog::~NewDialog() {
    delete l1_p_cache_handler;
    delete l1_d_cache_handler;
    delete l2_u_cache_handler;
    delete ui;
    // Settings is freed by parent
    delete config;
}

void NewDialog::switch2custom() {
	ui->preset_custom->setChecked(true);
	config_gui();
}

void NewDialog::set_default_settings(bool ds) {
    default_settings = ds;
}

void NewDialog::closeEvent(QCloseEvent *) {
    load_settings(); // Reset from settings
    // Close main window if not already configured
    MainWindow *prnt = (MainWindow*)parent();
    if (!prnt->configured())
        prnt->close();
}

NewDialogCacheHandler *NewDialog::l1_data_cache_handler() {
    return l1_d_cache_handler;
}

NewDialogCacheHandler *NewDialog::l1_program_cache_handler() {
    return l1_p_cache_handler;
}

Ui::NewDialogCache * NewDialog::l2_cache_dialog() {
    return ui_l2_cache;
}

void NewDialog::cancel() {
    this->close();
}

void NewDialog::create() {
    MainWindow *prnt = (MainWindow*)parent();

    try {
        prnt->create_core(*config, true, false);
    } catch (const machine::QtMipsExceptionInput &e) {
        QMessageBox msg(this);
        msg.setText(e.msg(false));
        msg.setIcon(QMessageBox::Critical);
        msg.setToolTip("Please check that ELF executable really exists and is in correct format.");
        msg.setDetailedText(e.msg(true));
        msg.setWindowTitle("Error while initializing new machine");
        msg.exec();
        return;
    }

    store_settings(); // Save to settings
    this->close();
}

void NewDialog::create_empty() {
    MainWindow *prnt = (MainWindow*)parent();
    prnt->create_core(*config, false, true);
    store_settings(); // Save to settings
    this->close();
}


void NewDialog::browse_elf() {
#ifndef __EMSCRIPTEN__
    QFileDialog elf_dialog(this);
    elf_dialog.setFileMode(QFileDialog::ExistingFile);
    if (elf_dialog.exec()) {
        QString path = elf_dialog.selectedFiles()[0];
        ui->elf_file->setText(path);
        config->set_elf(path);
    }
    // Elf shouldn't have any other effect so we skip config_gui here
#else
    QHtml5File::load("*", [&](const QByteArray &content, const QString &fileName) {
                QFileInfo fi(fileName);
                QString elf_name = fi.fileName();
                QFile file(elf_name);
                file.open(QIODevice::WriteOnly | QIODevice::Truncate);
                file.write(content);
                file.close();
                ui->elf_file->setText(elf_name);
                config->set_elf(elf_name);
            });
#endif
}

void NewDialog::elf_change(QString val) {
    config->set_elf(val);
}

void NewDialog::set_preset() {
    unsigned pres_n = preset_number();
    if (pres_n > 0) {
        config->preset((enum machine::ConfigPresets)(pres_n - 1));
        config_gui();
    }
}

void NewDialog::pipelined_change(bool val) {
    config->set_pipelined(val);

	switch2custom();
    // Pipeline changes also affects control hazard unit.
    control_hazard_unit_change();
}

void NewDialog::data_hazard_unit_change() {
    if (ui->data_hazard_unit->isChecked()) {
        config->set_data_hazard_unit(ui->data_hazard_stall->isChecked() ? machine::MachineConfig::DHU_STALL : machine::MachineConfig::DHU_STALL_FORWARD);
	} else {
        config->set_data_hazard_unit(machine::MachineConfig::DHU_NONE);
	}
    switch2custom();
}

void NewDialog::control_hazard_unit_change() {
    if (config->pipelined()) {
        uint8_t bht_bits_num = 0;
        if (ui->branch_predictor->isChecked()) {
            QString predictor_bits = ui->predictor_bits->currentText();
            QString bht_bits = ui->bht_bits->currentText();

            config->set_control_hazard_unit(predictor_bits.toShort() == 1 ?
                                            machine::MachineConfig::CHU_ONE_BIT_BP :
                                            machine::MachineConfig::CHU_TWO_BIT_BP);
            bht_bits_num = bht_bits.toShort();
        } else if (ui->delay_slot->isChecked() || ui->none->isChecked()) {
            config->set_control_hazard_unit(machine::MachineConfig::CHU_DELAY_SLOT);
        } else {
            config->set_control_hazard_unit(machine::MachineConfig::CHU_STALL);
        }
        config->set_bht_bits(bht_bits_num);
        config->set_branch_res_id(ui->resolution->currentText() == "ID");
    } else {
        if (ui->branch_predictor->isChecked() || ui->stall->isChecked() || ui->none->isChecked()) {
            // branch predictor / stall-on-branch do not have any value in a non-pipelined mode.
            config->set_control_hazard_unit(machine::MachineConfig::CHU_NONE);
            config->set_bht_bits(0);
        } else {
            // Delay slot is checked.
            config->set_control_hazard_unit(machine::MachineConfig::CHU_DELAY_SLOT);
            config->set_bht_bits(0);
        }
    }

    switch2custom();
}

void NewDialog::mem_protec_exec_change(bool v) {
    config->set_memory_execute_protection(v);
	switch2custom();
}

void NewDialog::mem_protec_write_change(bool v) {
    config->set_memory_write_protection(v);
	switch2custom();
}

void NewDialog::mem_time_read_change(int v) {
    if (config->l2_unified_cache().mem_access_read() != (unsigned)v) {
        config->set_ram_access_read(v);
        switch2custom();
    }
}

void NewDialog::mem_time_write_change(int v) {
    if (config->l2_unified_cache().mem_access_write() != (unsigned)v) {
        config->set_ram_access_write(v);
        switch2custom();
    }
}

void NewDialog::mem_time_burst_change(int v) {
    if (config->l2_unified_cache().mem_access_burst() != (unsigned)v) {
        config->set_ram_access_burst(v);
        switch2custom();
    }
}

void NewDialog::osemu_enable_change(bool v) {
    config->set_osemu_enable(v);
}

void NewDialog::osemu_known_syscall_stop_change(bool v) {
    config->set_osemu_known_syscall_stop(v);
}

void NewDialog::osemu_unknown_syscall_stop_change(bool v) {
    config->set_osemu_unknown_syscall_stop(v);
}

void NewDialog::osemu_interrupt_stop_change(bool v) {
    config->set_osemu_interrupt_stop(v);
}

void NewDialog::osemu_exception_stop_change(bool v) {
    config->set_osemu_exception_stop(v);
}

void NewDialog::browse_osemu_fs_root() {
    QFileDialog osemu_fs_root_dialog(this);
    osemu_fs_root_dialog.setFileMode(QFileDialog::DirectoryOnly);
    if (osemu_fs_root_dialog.exec()) {
        QString path = osemu_fs_root_dialog.selectedFiles().at(0);
        ui->osemu_fs_root->setText(path);
        config->set_osemu_fs_root(path);
    }
}

void NewDialog::osemu_fs_root_change(QString val) {
    config->set_osemu_fs_root(val);
}

void NewDialog::reset_at_compile_change(bool v) {
    config->set_reset_at_compile(v);
}

void NewDialog::config_gui() {
    // Basic
    ui->elf_file->setText(config->elf());
    ui->reset_at_compile->setChecked(config->reset_at_compile());
    // Core
    ui->pipelined->setChecked(config->pipelined());
    ui->data_hazard_unit->setChecked(config->data_hazard_unit() != machine::MachineConfig::DHU_NONE);
    ui->data_hazard_stall->setChecked(config->data_hazard_unit() == machine::MachineConfig::DHU_STALL);
    ui->data_hazard_stall_forward->setChecked(config->data_hazard_unit() == machine::MachineConfig::DHU_STALL_FORWARD);
    ui->none->setChecked(config->control_hazard_unit() == machine::MachineConfig::CHU_NONE);
    ui->stall->setChecked(config->control_hazard_unit() == machine::MachineConfig::CHU_STALL);
    ui->delay_slot->setChecked(config->control_hazard_unit() == machine::MachineConfig::CHU_DELAY_SLOT);
    ui->branch_predictor->setChecked(config->predictor());
    ui->resolution->setCurrentIndex(config->branch_res_id() ? 0 : 1);
    if (config->predictor()) {
        ui->predictor_bits->setCurrentIndex(
                config->control_hazard_unit() == machine::MachineConfig::CHU_ONE_BIT_BP ? 0 : 1);
        ui->bht_bits->setCurrentIndex(config->bht_bits() - MIN_BHT_BITS);
    } else {
        ui->predictor_bits->setCurrentIndex(0);
        ui->bht_bits->setCurrentIndex(0);
    }

    // Memory
    ui->mem_protec_exec->setChecked(config->memory_execute_protection());
    ui->mem_protec_write->setChecked(config->memory_write_protection());
    ui->mem_access_read->setValue(config->ram_access_read());
    ui->mem_access_write->setValue(config->ram_access_write());
    ui->mem_access_burst->setValue(config->ram_access_burst());
    // Cache
    l1_d_cache_handler->config_gui();
    l1_p_cache_handler->config_gui();
    l2_u_cache_handler->config_gui(config->l2_unified_cache().mem_access_read(),
                                   config->l2_unified_cache().mem_access_write(),
                                   config->l2_unified_cache().mem_access_burst());
    // Operating system and exceptions
    ui->osemu_enable->setChecked(config->osemu_enable());
    ui->osemu_known_syscall_stop->setChecked(config->osemu_known_syscall_stop());
    ui->osemu_unknown_syscall_stop->setChecked(config->osemu_unknown_syscall_stop());
    ui->osemu_interrupt_stop->setChecked(config->osemu_interrupt_stop());
    ui->osemu_exception_stop->setChecked(config->osemu_exception_stop());
    ui->osemu_fs_root->setText(config->osemu_fs_root());

    // Disable various sections according to configuration
    ui->data_hazard_unit->setEnabled(config->pipelined());
    ui->stall->setEnabled(config->pipelined());
    ui->none->setEnabled(!config->pipelined());
    ui->branch_predictor->setEnabled(config->pipelined());
    ui->bht_bits_label->setEnabled(config->predictor());
    ui->bht_bits->setEnabled(config->predictor());
    ui->predictor_bits_label->setEnabled(config->predictor());
    ui->predictor_bits->setEnabled(config->predictor());
    ui->resolution_label->setEnabled(config->pipelined());
    ui->resolution->setEnabled(config->pipelined());
}

unsigned NewDialog::preset_number() {
    enum machine::ConfigPresets preset;
    if (ui->preset_no_pipeline->isChecked())
        preset = machine::ConfigPresets::CP_SINGLE;
    else if (ui->preset_no_pipeline_cache->isChecked())
        preset = machine::ConfigPresets::CP_SINGLE_CACHE;
    else if (ui->preset_pipelined_bare->isChecked())
        preset = machine::ConfigPresets::CP_PIPE_NO_HAZARD;
    else if (ui->preset_pipelined->isChecked())
        preset = machine::ConfigPresets::CP_PIPE;
    else
        return 0;
    return (unsigned)preset + 1;
}

void NewDialog::load_settings() {
    if (config != nullptr)
        delete config;

    // Load config
    config = default_settings ? new machine::MachineConfig() : new machine::MachineConfig(settings);
    l1_d_cache_handler->set_config(config->access_l1_data_cache());
    l1_p_cache_handler->set_config(config->access_l1_program_cache());
    l2_u_cache_handler->set_config(config->access_l2_unified_cache());

    // Load preset
    unsigned preset = settings->value("Preset", 1).toUInt();
    if (preset != 0) {
        auto p = (enum machine::ConfigPresets)(preset - 1);
        config->preset(p);
        switch (p) {
        case machine::ConfigPresets::CP_SINGLE:
            ui->preset_no_pipeline->setChecked(true);
            break;
        case machine::ConfigPresets::CP_SINGLE_CACHE:
            ui->preset_no_pipeline_cache->setChecked(true);
            break;
        case machine::ConfigPresets::CP_PIPE_NO_HAZARD:
            ui->preset_pipelined_bare->setChecked(true);
            break;
        case machine::ConfigPresets::CP_PIPE:
            ui->preset_pipelined->setChecked(true);
            break;
        }
    } else {
        ui->preset_custom->setChecked(true);
    }

    config_gui();
}

void NewDialog::store_settings() {
    config->store(settings);

    // Presets are not stored in settings so we have to store them explicitly
    if (ui->preset_custom->isChecked()) {
        settings->setValue("Preset", 0);
	} else {
        settings->setValue("Preset", preset_number());
	}
}

NewDialogCacheHandler::NewDialogCacheHandler(NewDialog *nd, Ui::NewDialogCache *cui) {
	this->nd = nd;
	this->cache_ui = cui;
	this->config = nullptr;
	connect(cache_ui->enabled, SIGNAL(clicked(bool)), this, SLOT(enabled(bool)));
	connect(cache_ui->number_of_sets, SIGNAL(editingFinished()), this, SLOT(numsets()));
	connect(cache_ui->block_size, SIGNAL(editingFinished()), this, SLOT(blocksize()));
	connect(cache_ui->degree_of_associativity, SIGNAL(editingFinished()), this, SLOT(degreeassociativity()));
	connect(cache_ui->replacement_policy, SIGNAL(activated(int)), this, SLOT(replacement(int)));
    connect(cache_ui->write_pol, SIGNAL(activated(int)), this, SLOT(writepolicy(int)));
    connect(cache_ui->write_alloc, SIGNAL(stateChanged(int)), this, SLOT(writeallocate(int)));
    connect(cache_ui->access_read, SIGNAL(valueChanged(int)), this, SLOT(access_read(int)));
    connect(cache_ui->access_write, SIGNAL(valueChanged(int)), this, SLOT(access_write(int)));
    connect(cache_ui->access_burst, SIGNAL(valueChanged(int)), this, SLOT(access_burst(int)));
}

void NewDialogCacheHandler::set_config(machine::MachineConfigCache *config) {
	this->config = config;
}

void NewDialogCacheHandler::config_gui(int time_read, int time_write, int time_burst) {
    cache_ui->enabled->setChecked(config->enabled());
    cache_ui->number_of_sets->setValue(config->sets());
    cache_ui->block_size->setValue(config->blocks());
    cache_ui->degree_of_associativity->setValue(config->associativity());
    cache_ui->replacement_policy->setCurrentIndex((int)config->replacement_policy());
    cache_ui->write_pol->setCurrentIndex((int) config->write_policy());
    cache_ui->write_alloc->setChecked(config->write_alloc());
    cache_ui->access_read->setValue(time_read);
    cache_ui->access_write->setValue(time_write);
    cache_ui->access_burst->setValue(time_burst);
}

void NewDialogCacheHandler::enabled(bool val) {
	config->set_enabled(val);

    // We must prohibit the user to enable L2 cache without enabling L1.
    switch (config->type()) {
        case machine::MemoryAccess::MemoryType::L1_PROGRAM_CACHE:
        case machine::MemoryAccess::MemoryType::L1_DATA_CACHE:
            if (nd->l1_data_cache_handler()->config->enabled() || nd->l1_program_cache_handler()->config->enabled()) {
                // L1 program or data cache are enabled, allow user to also enable L2 Cache.
                nd->l2_cache_dialog()->enabled->setEnabled(true);
                nd->l2_cache_dialog()->enabled->setCheckable(true);
                nd->l2_cache_dialog()->access_time->setEnabled(true);
            } else {
                // L1 program and data cache are both disabled, also disable L2 cache.
                nd->l2_cache_dialog()->enabled->clicked(false);
                nd->l2_cache_dialog()->enabled->setEnabled(false);
                nd->l2_cache_dialog()->enabled->setCheckable(false);
                nd->l2_cache_dialog()->access_time->setEnabled(false);
            }
            break;
        case machine::MemoryAccess::MemoryType::L2_UNIFIED_CACHE:
            if (val) {
                // L2 cache is enabled, check if at least one of L1 program or data cache is also enabled.
                if (!nd->l1_data_cache_handler()->config->enabled() && !nd->l1_program_cache_handler()->config->enabled()) {
                    nd->l2_cache_dialog()->enabled->clicked(false);
                    nd->l2_cache_dialog()->enabled->setCheckable(false);
                    nd->l2_cache_dialog()->access_time->setEnabled(false);

                    QMessageBox messageBox;
                    messageBox.critical(nullptr, "Error", "L2 cache can't be enabled without enabling one of L1 caches");
                    messageBox.setFixedSize(500, 200);
                }
            }
            break;
        default:
            SANITY_ASSERT(0, "Debug me :)");
    }

	nd->switch2custom();
}

void NewDialogCacheHandler::numsets() {
	config->set_sets(cache_ui->number_of_sets->value());
	nd->switch2custom();
}

void NewDialogCacheHandler::blocksize() {
	config->set_blocks(cache_ui->block_size->value());
	nd->switch2custom();
}

void NewDialogCacheHandler::degreeassociativity() {
	config->set_associativity(cache_ui->degree_of_associativity->value());
	nd->switch2custom();
}

void NewDialogCacheHandler::replacement(int val) {
	config->set_replacement_policy((enum machine::MachineConfigCache::ReplacementPolicy)val);
	nd->switch2custom();
}

void NewDialogCacheHandler::writepolicy(int val) {
	config->set_write_policy((enum machine::MachineConfigCache::WritePolicy)val);
	nd->switch2custom();
}

void NewDialogCacheHandler::writeallocate(int /*val*/) {
    config->set_write_alloc(cache_ui->write_alloc->isChecked());
    nd->switch2custom();
}

void NewDialogCacheHandler::access_read(int val) {
    config->set_mem_access_read(val);
    nd->switch2custom();
}

void NewDialogCacheHandler::access_write(int val) {
    config->set_mem_access_write(val);
    nd->switch2custom();
}

void NewDialogCacheHandler::access_burst(int val) {
    config->set_mem_access_burst(val);
    nd->switch2custom();
}
