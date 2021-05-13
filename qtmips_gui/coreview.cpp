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

#include "coreview.h"
#include "fontsize.h"
#include <cmath>

#include <QDebug>

//////////////////////////////////////////////////////////////////////////////
/// Size of visible view area
#define SC_WIDTH 720
#define SC_HEIGHT 540
//////////////////////////////////////////////////////////////////////////////

#define NEW_B(OBJ, TYPE, VAR, ...) do { \
        VAR = new coreview::TYPE(__VA_ARGS__);\
        OBJ->addItem(VAR);\
    } while(false)
#define NEW(OBJ, TYPE, VAR, X, Y, ...) do { \
        NEW_B(OBJ, TYPE, VAR, __VA_ARGS__); \
        VAR->setPos(X, Y); \
    } while(false)
#define NEW_I(OBJ, VAR, X, Y, SIG, ...) do { \
        NEW(OBJ, InstructionView, VAR, X, Y, __VA_ARGS__); \
        OBJ->connect(machine->core(), &machine::Core::SIG, \
                VAR, &coreview::InstructionView::instruction_update); \
    } while(false)
#define NEW_V(OBJ, X, Y, SIG, ...) do { \
        NEW(OBJ, Value, val, X, Y, __VA_ARGS__); \
        OBJ->connect(machine->core(), SIGNAL(SIG(std::uint32_t)), val, SLOT(value_update(std::uint32_t))); \
    } while(false)
#define NEW_MULTI(OBJ, VAR, X, Y, SIG, ...) do { \
        NEW(OBJ, MultiText, VAR, X, Y, __VA_ARGS__); \
        OBJ->connect(machine->core(), &machine::Core::SIG, \
                VAR, &coreview::MultiText::multitext_update); \
    } while(false)
#define NEW_MUX(OBJ, VAR, X, Y, SIG, ...) do { \
        NEW(OBJ, Multiplexer, VAR, X, Y, __VA_ARGS__); \
        OBJ->connect(machine->core(), SIGNAL(SIG(std::uint32_t)), VAR, SLOT(set(std::uint32_t))); \
    } while(false)
#define NEW_MINIMUX(OBJ, VAR, X, Y, SIG, ...) do { \
        NEW(OBJ, MiniMux, VAR, X, Y, __VA_ARGS__); \
        OBJ->connect(machine->core(), SIGNAL(SIG(std::uint32_t)), VAR, SLOT(set(std::uint32_t))); \
    } while(false)

#define BPVIEW_OFF 20

CoreViewScene::CoreViewScene(machine::QtMipsMachine *machine) : QGraphicsScene() {
    coreview::Value *val;
    static QMap<std::uint32_t, QString> excause_map =
        {{machine::EXCAUSE_NONE,     "NONE"},
         {machine::EXCAUSE_INT,      "INT"},
         {machine::EXCAUSE_ADDRL,    "ADDRL"},
         {machine::EXCAUSE_ADDRS,    "ADDRS"},
         {machine::EXCAUSE_IBUS,     "IBUS"},
         {machine::EXCAUSE_DBUS,     "DBUS"},
         {machine::EXCAUSE_SYSCALL,  "SYSCALL"},
         {machine::EXCAUSE_BREAK,    "BREAK"},
         {machine::EXCAUSE_OVERFLOW, "OVERFLOW"},
         {machine::EXCAUSE_TRAP,     "TRAP"},
         {machine::EXCAUSE_HWBREAK,  "HWBREAK"}
        };

    setSceneRect(0, 0, SC_WIDTH, SC_HEIGHT);

    // Elements
    // Primary points
    NEW(this, ProgramMemory, mem_program, 90, 240, machine);
    NEW(this, DataMemory, mem_data, 580, 258, machine);
    NEW(this, Registers, regs, 230, 240);
    NEW(this, Alu, alu, 490, 233);
    NEW(this, LogicBlock, peripherals, 610, 350, "Peripherals");
    NEW(this, LogicBlock, terminal, 610, 400, "Terminal");

    // Execute stage
    NEW(this, Junction, ex.j_mux, 450, 303);
    NEW_MUX(this, ex.mux_imm, 470, 292, execute_alusrc_value, 2, true);
    NEW_MUX(this, ex.mux_regdest, 480, 370, execute_regdest_value, 2, true);

    // Memory stage
    NEW(this, Junction, mm.j_addr, 570, mem_data->connector_address()->y(), true, 8);
    NEW_MULTI(this, mm.multi_excause, 602, 447, memory_excause_value, excause_map, true);
    new_label("Exception", 595, 437);

    // WriteBack stage
    NEW_MUX(this, wb.mem_or_reg, 690, 252, writeback_memtoreg_value, 2, true);
    NEW(this, Junction, wb.j_reg_write_val, 411, 510);

    // Connections
    // Execute stage
    new_bus(ex.j_mux->new_connector(CON_AX_X), ex.mux_imm->connector_in(0));
    new_bus(ex.mux_imm->connector_out(), alu->connector_in_b());

    // Memory stage
    new_bus(mm.j_addr->new_connector(CON_AX_X), mem_data->connector_address());

    // From write back stage to decode stage
    new_bus(wb.mem_or_reg->connector_out(), wb.j_reg_write_val->new_connector(coreview::Connector::AX_X))->setAxes({CON_AXIS_Y(710)});
    new_bus(wb.j_reg_write_val->new_connector(coreview::Connector::AX_X), regs->connector_write())->setAxes({CON_AXIS_Y(172)});

    // Control unit labels
    new_label("JumpReg", 300, 86);
    new_label("PcToR31", 300, 92);
    new_label("MemToReg", 300, 106);
    new_label("MemWrite", 300, 112);
    new_label("MemRead", 300, 119);
    new_label("AluCtrl", 300, 125);
    new_label("RegDest", 300, 132);
    new_label("AluSrc", 300, 138);
    new_label("Branch", 300, 145);

    // Fetch stage values
    if (!machine->config().predictor())
        NEW_V(this, 25, 440,  fetch_branch_value, false, 1);
    NEW_V(this, 360, 93,  fetch_jump_reg_value, false, 1);

    // Decode stage values
    NEW_V(this, 200, 200, decode_instruction_value); // Instruction
    NEW_V(this, 360, 250, decode_reg1_value); // Register output 1
    NEW_V(this, 360, 270, decode_reg2_value); // Register output 2
    NEW_V(this, 335, 413, decode_immediate_value); // Sign extended immediate value
    NEW_V(this, 370, 99,  decode_regd31_value, false, 1);
    NEW_V(this, 370, 113, decode_memtoreg_value, false, 1);
    NEW_V(this, 360, 120, decode_memwrite_value, false, 1);
    NEW_V(this, 370, 127, decode_memread_value, false, 1);
    NEW_V(this, 360, 140, decode_regdest_value, false, 1);
    NEW_V(this, 370, 148, decode_alusrc_value, false, 1);

    // Execute stage
    NEW_V(this, 450, 230, execute_reg1_value, true); // Register 1
    NEW_V(this, 450, 310, execute_reg2_value, true); // Register 2
    NEW_V(this, 527, 280, execute_alu_value, true); // Alu output
    NEW_V(this, 480, 413, execute_immediate_value); // Immediate value
    NEW_V(this, 470, 113, execute_memtoreg_value, false, 1);
    NEW_V(this, 460, 120, execute_memwrite_value, false, 1);
    NEW_V(this, 470, 127, execute_memread_value, false, 1);
    NEW_V(this, 470, 127, execute_memread_value, false, 1);
    NEW_V(this, 485, 345, execute_regdest_value, false, 1);
    NEW_V(this, 475, 280, execute_alusrc_value, false, 1);

    // Memory stage
    NEW_V(this, 560, 260, memory_alu_value, true); // Alu output
    NEW_V(this, 560, 345, memory_rt_value, true); // rt
    NEW_V(this, 650, 290, memory_mem_value, true); // Memory output
    NEW_V(this, 570, 113, memory_memtoreg_value, false, 1);
    NEW_V(this, 630, 220, memory_memwrite_value, false, 1);
    NEW_V(this, 620, 220, memory_memread_value, false, 1);
    // Write back stage
    NEW_V(this, 710, 330, writeback_value, true); // Write back value

    new_label("RsD", 215, 241);
    NEW_V(this, 205, 250, decode_rs_num_value, false, 2, 0, 10, ' ');
    new_label("RtD", 215, 261);
    NEW_V(this, 205, 270, decode_rt_num_value, false, 2, 0, 10, ' ');

    new_label("RtD", 297, 372);
    NEW_V(this, 320, 380, decode_rt_num_value, false, 2, 0, 10, ' ');
    new_label("RdD", 297, 380);
    NEW_V(this, 320, 390, decode_rd_num_value, false, 2, 0, 10, ' ');
    NEW_V(this, 320, 500, writeback_regw_num_value, false, 2, 0, 10, ' ');

    new_label("Cycles", 440, SC_HEIGHT - 14);
    NEW_V(this, 500, SC_HEIGHT - 9, cycle_c_value, false, 10, 0, 10, ' ', false);
    new_label("Stalls", 570, SC_HEIGHT - 14);
    NEW_V(this, 630, SC_HEIGHT - 9, stall_c_value, false, 10, 0, 10, ' ', false);

    setBackgroundBrush(QBrush(Qt::white));

    connect(regs, SIGNAL(open_registers()), this, SIGNAL(request_registers()));
    connect(mem_program, SIGNAL(open_mem()), this, SIGNAL(request_program_memory()));
    connect(mem_data, SIGNAL(open_mem()), this, SIGNAL(request_data_memory()));
    connect(mem_program, SIGNAL(open_cache()), this, SIGNAL(request_l1_cache_program()));
    connect(mem_data, SIGNAL(open_cache()), this, SIGNAL(request_l1_cache_data()));
    connect(peripherals, SIGNAL(open_block()), this, SIGNAL(request_peripherals()));
    connect(terminal, SIGNAL(open_block()), this, SIGNAL(request_terminal()));
}

CoreViewScene::~CoreViewScene() {
    // We add all items to scene and they are removed in QGraphicsScene descructor so we don't have to care about them here
}

coreview::Connection *CoreViewScene::new_connection(const coreview::Connector *a, const coreview::Connector *b) {
    coreview::Connection *c = new coreview::Connection(a, b);
    addItem(c);
    c->setZValue(-1); // hide connections under neath the items
    return c;
}

coreview::Bus *CoreViewScene::new_bus(const coreview::Connector *a, const coreview::Connector *b, unsigned width) {
    coreview::Bus *c = new coreview::Bus(a, b, width);
    addItem(c);
    c->setZValue(-2); // hide connections under neath the items
    return c;
}

coreview::Signal *CoreViewScene::new_signal(const coreview::Connector *a, const coreview::Connector *b) {
    coreview::Signal *c = new coreview::Signal(a, b);
    addItem(c);
    c->setZValue(-3); // hide connections under neath the items
    return c;
}

QGraphicsSimpleTextItem *CoreViewScene::new_label(const QString &str, qreal x, qreal y) {
    QGraphicsSimpleTextItem *i= new QGraphicsSimpleTextItem(str);
    QFont f;
    f.setPointSize(FontSize::SIZE5);
    i->setFont(f);
    addItem(i);
    i->setPos(x, y);
    return i;
}

template<typename T>
void initCoreViewScene(T *coreviewScene, machine::QtMipsMachine *machine) {
    struct coreview::Latch::ConnectorPair pc_latch_pair;
    const coreview::Connector *dc_con_sign_ext;

    // Fetch stage
    NEW(coreviewScene, ProgramCounter, coreviewScene->ft.pc, 2, 280, machine);
    NEW(coreviewScene, Latch, coreviewScene->ft.latch, 55, 250, machine, 20);
    NEW(coreviewScene, Adder, coreviewScene->ft.adder, 100, 320);
    pc_latch_pair = coreviewScene->ft.latch->new_connector(10);
    NEW_B(coreviewScene, Constant, coreviewScene->ft.adder_4, coreviewScene->ft.adder->connector_in_b(), "4");
    NEW(coreviewScene, Junction, coreviewScene->ft.junc_pc, 80, coreviewScene->mem_program->connector_address()->y());
    NEW(coreviewScene, Junction, coreviewScene->ft.junc_pc_4, 130, 380);
    NEW_MUX(coreviewScene, coreviewScene->ft.multiplex, 20, 390, fetch_branch_value, 2);

    // Decode stage
    NEW(coreviewScene, LogicBlock, coreviewScene->dc.ctl_block, 230, 90, {"Control", "unit"});
    coreviewScene->dc.ctl_block->setSize(35, 70);
    NEW(coreviewScene, LogicBlock, coreviewScene->dc.sign_ext, 250, 400, {"Sign", "extension"});
    NEW(coreviewScene, LogicBlock, coreviewScene->dc.shift2, 310, 428, "<<2");
    NEW(coreviewScene, Adder, coreviewScene->dc.add, 340, 428);
    dc_con_sign_ext = coreviewScene->dc.sign_ext->new_connector(1, 0);
    NEW(coreviewScene, Junction, coreviewScene->dc.j_sign_ext, 290, dc_con_sign_ext->y());
    NEW(coreviewScene, LogicBlock, coreviewScene->dc.cmp, 312, 200, "=");
    NEW(coreviewScene, And, coreviewScene->dc.and_branch, 350, 190);
    coreviewScene->dc.cmp->setSize(24, 12);
    NEW(coreviewScene, Junction, coreviewScene->dc.j_inst_up, 190, 126);
    NEW(coreviewScene, Junction, coreviewScene->dc.j_inst_down, 190, dc_con_sign_ext->y());
    NEW(coreviewScene, Junction, coreviewScene->dc.j_jalpctor31, 365, 100);
    NEW(coreviewScene, Junction, coreviewScene->dc.j_jump_reg, 355, 94);

    // Connections
    // Fetch stage
    coreviewScene->new_bus(coreviewScene->ft.pc->connector_out(), pc_latch_pair.in);
    coreviewScene->new_bus(pc_latch_pair.out, coreviewScene->ft.junc_pc->new_connector(coreview::Connector::AX_X));
    coreviewScene->new_bus(coreviewScene->ft.junc_pc->new_connector(coreview::Connector::AX_Y), coreviewScene->ft.adder->connector_in_a());
    coreviewScene->new_bus(coreviewScene->ft.junc_pc->new_connector(coreview::Connector::AX_X), coreviewScene->mem_program->connector_address());
    coreviewScene->new_bus(coreviewScene->ft.multiplex->connector_out(), coreviewScene->ft.pc->connector_in());
    coreviewScene->new_bus(coreviewScene->ft.adder->connector_out(), coreviewScene->ft.junc_pc_4->new_connector(coreview::Connector::AX_Y));
    coreviewScene->new_bus(coreviewScene->ft.junc_pc_4->new_connector(coreview::Connector::AX_X), coreviewScene->ft.multiplex->connector_in(0))->setAxes({CON_AXIS_Y(10)});

    // Decode stage
    coreviewScene->new_bus(dc_con_sign_ext, coreviewScene->dc.j_sign_ext->new_connector(coreview::Connector::AX_X));
    coreviewScene->new_bus(coreviewScene->dc.j_sign_ext->new_connector(coreview::Connector::AX_Y), coreviewScene->dc.shift2->new_connector(-1, 0));
    coreviewScene->new_bus(coreviewScene->dc.shift2->new_connector(1, 0), coreviewScene->dc.add->connector_in_a());
    coreviewScene->new_signal(coreviewScene->dc.cmp->new_connector(1, 0), coreviewScene->dc.and_branch->connector_in(1))->setAxes({CON_AXIS_Y(343)});;
    coreviewScene->new_signal(coreviewScene->dc.ctl_block->new_connector(1, 0.8), coreviewScene->dc.and_branch->connector_in(0))->setAxes({CON_AXIS_Y(343)});
    coreviewScene->dc.instr_bus = coreviewScene->new_bus(coreviewScene->dc.j_inst_up->new_connector(coreview::Connector::AX_Y), coreviewScene->dc.j_inst_down->new_connector(coreview::Connector::AX_Y));
    coreviewScene->new_bus(coreviewScene->dc.j_inst_up->new_connector(coreview::Connector::AX_X), coreviewScene->dc.ctl_block->new_connector(-1, 0), 2);
    coreviewScene->new_bus(coreviewScene->dc.j_inst_down->new_connector(coreview::Connector::AX_X), coreviewScene->dc.sign_ext->new_connector(-1, 0), 2);
    coreviewScene->new_bus(coreviewScene->dc.instr_bus->new_connector(coreviewScene->regs->connector_read1_reg()->point()), coreviewScene->regs->connector_read1_reg(), 2);
    coreviewScene->new_bus(coreviewScene->dc.instr_bus->new_connector(coreviewScene->regs->connector_read2_reg()->point()), coreviewScene->regs->connector_read2_reg(), 2);
    coreviewScene->new_signal(coreviewScene->dc.ctl_block->new_connector(1, -0.8), coreviewScene->dc.j_jalpctor31->new_connector(coreview::Connector::AX_X));
    coreviewScene->new_signal(coreviewScene->dc.ctl_block->new_connector(1, -10), coreviewScene->dc.j_jump_reg->new_connector(coreview::Connector::AX_X));

    coreviewScene->connect(coreviewScene->ft.pc, SIGNAL(open_program()), coreviewScene, SIGNAL(request_program_memory()));
    coreviewScene->connect(coreviewScene->ft.pc, SIGNAL(jump_to_pc(std::uint32_t)), coreviewScene, SIGNAL(request_jump_to_program_counter(std::uint32_t)));
}

CoreViewSceneSimple::CoreViewSceneSimple(machine::QtMipsMachine *machine) : CoreViewScene(machine) {
    coreview::Connection *con;
    coreview::Value *val;
    coreview::Bus *regs_bus1, *regs_bus2;
    const coreview::Connector *regs_bus_con;
    struct coreview::Latch::ConnectorPair lp_ft_pc, lp_ft_inst;

    initCoreViewScene(this, machine);

    NEW_I(this, inst_prim, 230, 60, instruction_executed, QColor(255, 173, 230));
    if (machine->config().branch_unit() == machine::MachineConfig::BU_DELAY_SLOT) {
        NEW(this, Latch, latch_if_id, 158, 250, machine, 220);
        NEW_I(this, inst_fetch,  79, 60, instruction_fetched, QColor(255, 173, 173));
    }

    // Fetch stage
    if (machine->config().branch_unit() == machine::MachineConfig::BU_DELAY_SLOT) {
        lp_ft_inst = latch_if_id->new_connector(mem_program->connector_instruction()->y() - latch_if_id->y());
        new_bus(mem_program->connector_instruction(), lp_ft_inst.in);
        lp_ft_pc = latch_if_id->new_connector(210);
        new_bus(ft.junc_pc_4->new_connector(coreview::Connector::AX_Y), lp_ft_pc.in);
        new_bus(lp_ft_inst.out, dc.instr_bus->new_connector(lp_ft_inst.out->point()));
        new_bus(lp_ft_pc.out, dc.add->connector_in_b())->setAxes({CON_AXIS_Y(270)});
    } else {
        new_bus(mem_program->connector_instruction(), dc.instr_bus->new_connector(mem_program->connector_instruction()->point()));
        new_bus(ft.junc_pc_4->new_connector(coreview::Connector::AX_Y), dc.add->connector_in_b())->setAxes({CON_AXIS_Y(270)});
    }

    // Decode stage
    regs_bus1 = new_bus(regs->connector_read1(), alu->connector_in_a());
    regs_bus_con = dc.cmp->new_connector(-0.5, 1);
    new_bus(regs_bus1->new_connector(regs_bus_con->point(), coreview::Connector::AX_Y), regs_bus_con);
    regs_bus2 = new_bus(regs->connector_read2(), ex.mux_imm->connector_in(0));
    regs_bus2->setAxes({CON_AXIS_Y(450)});
    regs_bus_con = dc.cmp->new_connector(0.5, 1);
    new_bus(regs_bus2->new_connector(regs_bus_con->point(), coreview::Connector::AX_Y), regs_bus_con);
    con = new_bus(dc.j_sign_ext->new_connector(coreview::Connector::AX_X), ex.mux_imm->connector_in(1));
    con->setAxes({CON_AXIS_Y(465)});
	new_signal(dc.ctl_block->new_connector(1, -0.6), regs->connector_ctl_write());

    // Execute
    new_bus(alu->connector_out(), mm.j_addr->new_connector(CON_AX_X));
    con = new_bus(ex.j_mux->new_connector(CON_AX_Y), mem_data->connector_data_in());
    con->setAxes({CON_AXIS_X(360), CON_AXIS_Y(560)});

    // Memory
    con = new_bus(mm.j_addr->new_connector(CON_AX_Y), wb.mem_or_reg->connector_in(0));
    con->setAxes({CON_AXIS_X(250), CON_AXIS_Y(678)});
    con = new_bus(mem_data->connector_data_out(), wb.mem_or_reg->connector_in(1));
    con->setAxes({CON_AXIS_Y(678)});

    // WriteBack

    // From decode stage to fetch stage
    con = new_signal(dc.and_branch->connector_out(), ft.multiplex->connector_ctl());
    con->setAxes({CON_AXIS_Y(370), CON_AXIS_X(490)});
    con = new_bus(dc.add->connector_out(), ft.multiplex->connector_in(1));
    con->setAxes({CON_AXIS_Y(360), CON_AXIS_X(480), CON_AXIS_Y(10)});

    // From decode to execute stage
    new_signal(dc.ctl_block->new_connector(1, 0.6), ex.mux_imm->connector_ctl());
	new_signal(dc.ctl_block->new_connector(1, 0.2), alu->connector_ctl());
    new_signal(dc.ctl_block->new_connector(1, 0.4), ex.mux_regdest->connector_ctl());
    new_bus(dc.instr_bus->new_connector(ex.mux_regdest->connector_in(0)->point()), ex.mux_regdest->connector_in(0), 2);
    new_bus(dc.instr_bus->new_connector(ex.mux_regdest->connector_in(1)->point()), ex.mux_regdest->connector_in(1), 2);

    // From decode to memory stage
	new_signal(dc.ctl_block->new_connector(1, 0.0), mem_data->connector_req_write());
	new_signal(dc.ctl_block->new_connector(1, -0.2), mem_data->connector_req_read());

    // From decode to write back stage
	new_signal(dc.ctl_block->new_connector(1, -0.4), wb.mem_or_reg->connector_ctl());

    // From execute to decode stage
    con = new_bus(ex.mux_regdest->connector_out(), regs->connector_write_reg(), 2);
    con->setAxes({CON_AXIS_Y(520), CON_AXIS_X(500), CON_AXIS_Y(210)});

    // Control unit labels
    new_label("RegWrite", 260, 99);

    // Label for write back stage
    NEW_V(this, 280, 200, writeback_regw_value, false, 1);
}

CoreViewScenePipelined::CoreViewScenePipelined(machine::QtMipsMachine *machine) : CoreViewScene(machine) {
    coreview::Value *val;
    struct coreview::Latch::ConnectorPair lp_ft_inst, lp_ft_pc, lp_dc_rs, lp_dc_rt, lp_dc_immed;
    struct coreview::Latch::ConnectorPair regdest_dc_rs, regdest_dc_rt, regdest_dc_rd, regdest_ex, regdest_mem;
    struct coreview::Latch::ConnectorPair ctl_alu_de, ctl_imm_de, ctl_regdest_de, ctl_memw_de, ctl_memr_de;
    struct coreview::Latch::ConnectorPair ctl_mr_de, ctl_rgw_de, ctl_memr_ex, ctl_mr_ex, ctl_rgw_ex;
    struct coreview::Latch::ConnectorPair ctl_memw_ex, ctl_mr_mem, ctl_rgw_mem;
    struct coreview::Latch::ConnectorPair lp_ex_alu, lp_ex_dt, lp_mem_alu, lp_mem_mem;
    coreview::Bus *regs_bus1, *regs_bus2;
    const coreview::Connector *regs_bus_con;
    const coreview::Connector *ctl_cnt;

    initCoreViewScene(this, machine);

    NEW(this, Latch, latch_if_id, 158, 70, machine, 400);
    latch_if_id->setTitle("IF/ID");
    NEW(this, Latch, latch_id_ex, 392, 70, machine, 400);
    latch_id_ex->setTitle("ID/EX");
    NEW(this, Latch, latch_ex_mem, 536, 70, machine, 400);
    latch_ex_mem->setTitle("EX/MEM");
    NEW(this, Latch, latch_mem_wb, 660, 70, machine, 400);
    latch_mem_wb->setTitle("MEM/WB");

    NEW_I(this, inst_fetch, 79, 2, instruction_fetched, QColor(255, 173, 173));
    NEW_I(this, inst_dec, 275, 2, instruction_decoded, QColor(255, 212, 173));
    NEW_I(this, inst_exec, 464, 2, instruction_executed, QColor(193, 255, 173));
    NEW_I(this, inst_mem, 598, 2, instruction_memory, QColor(173, 255, 229));
    NEW_I(this, inst_wrb, 660, 18, instruction_writeback, QColor(255, 173, 230));

    if (machine->config().hazard_unit() != machine::MachineConfig::HU_NONE) {
        NEW(this, LogicBlock, hazard_unit, SC_WIDTH/2, SC_HEIGHT - 15, "Hazard Unit");
        hazard_unit->setSize(SC_WIDTH - 100, 12);
        static QMap<std::uint32_t, QString> stall_map = {{0, "NORMAL"},{1, "STALL"},{2, "FORWARD"}};
        NEW_MULTI(this, hu.multi_stall, 480, 447, execute_stall_forward_value, stall_map);
        NEW_MULTI(this, hu.multi_stall, 310, 340, branch_forward_value, stall_map);
        NEW_MULTI(this, hu.multi_stall, 250, SC_HEIGHT - 15, hu_stall_value, stall_map);
    }

    // Fetch stage
    lp_ft_inst = latch_if_id->new_connector(mem_program->connector_instruction()->y() - latch_if_id->y());
    new_bus(mem_program->connector_instruction(), lp_ft_inst.in);
    lp_ft_pc = latch_if_id->new_connector(390);
    new_bus(ft.junc_pc_4->new_connector(coreview::Connector::AX_Y), lp_ft_pc.in);

    // Decode stage
    new_bus(lp_ft_inst.out, dc.instr_bus->new_connector(lp_ft_inst.out->point()));
    new_bus(lp_ft_pc.out, dc.add->connector_in_b())->setAxes({CON_AXIS_Y(270)});

    lp_dc_rs = latch_id_ex->new_connector(regs->connector_read1()->y() - latch_id_ex->y());
    lp_dc_rt = latch_id_ex->new_connector(regs->connector_read2()->y() - latch_id_ex->y());
    regs_bus1 = new_bus(regs->connector_read1(), lp_dc_rs.in);
    regs_bus2 = new_bus(regs->connector_read2(), lp_dc_rt.in);

    if (machine->config().hazard_unit() != machine::MachineConfig::HU_STALL_FORWARD) {
        regs_bus_con = dc.cmp->new_connector(-0.5, 1);
        new_bus(regs_bus1->new_connector(regs_bus_con->point(), coreview::Connector::AX_Y), regs_bus_con);
        regs_bus_con = dc.cmp->new_connector(0.5, 1);
        new_bus(regs_bus2->new_connector(regs_bus_con->point(), coreview::Connector::AX_Y), regs_bus_con);
    }

    lp_dc_immed = latch_id_ex->new_connector(dc.j_sign_ext->y() - latch_id_ex->y());
    new_bus(dc.j_sign_ext->new_connector(coreview::Connector::AX_X), lp_dc_immed.in);
    regdest_dc_rt = latch_id_ex->new_connector(ex.mux_regdest->connector_in(0)->point().y() - latch_id_ex->y());
    regdest_dc_rd = latch_id_ex->new_connector(ex.mux_regdest->connector_in(1)->point().y() - latch_id_ex->y());
    new_bus(dc.instr_bus->new_connector(ex.mux_regdest->connector_in(0)->point()), regdest_dc_rt.in, 2);
    new_bus(dc.instr_bus->new_connector(ex.mux_regdest->connector_in(1)->point()), regdest_dc_rd.in, 2);
    ctl_cnt = dc.ctl_block->new_connector(1, 0.2);

    ctl_alu_de = latch_id_ex->new_connector(ctl_cnt->point().y() - latch_id_ex->y());
    new_signal(ctl_cnt, ctl_alu_de.in);
    ctl_cnt = dc.ctl_block->new_connector(1, 0.6);
    ctl_imm_de = latch_id_ex->new_connector(ctl_cnt->point().y() - latch_id_ex->y());
    new_signal(ctl_cnt, ctl_imm_de.in);
    ctl_cnt = dc.ctl_block->new_connector(1, 0.4);
    ctl_regdest_de = latch_id_ex->new_connector(ctl_cnt->point().y() - latch_id_ex->y());
    new_signal(ctl_cnt, ctl_regdest_de.in);
    ctl_cnt = dc.ctl_block->new_connector(1, 0.0);
    ctl_memw_de = latch_id_ex->new_connector(ctl_cnt->point().y() - latch_id_ex->y());
    new_signal(ctl_cnt, ctl_memw_de.in);
    ctl_cnt = dc.ctl_block->new_connector(1, -0.2);
    ctl_memr_de = latch_id_ex->new_connector(ctl_cnt->point().y() - latch_id_ex->y());
    new_signal(ctl_cnt, ctl_memr_de.in);
    ctl_cnt = dc.ctl_block->new_connector(1, -0.4);
    ctl_mr_de = latch_id_ex->new_connector(ctl_cnt->point().y() - latch_id_ex->y());
    new_signal(ctl_cnt, ctl_mr_de.in);
    ctl_cnt = dc.ctl_block->new_connector(1, -0.6);
    ctl_rgw_de = latch_id_ex->new_connector(ctl_cnt->point().y() - latch_id_ex->y());
    new_signal(ctl_cnt, ctl_rgw_de.in);

    // Execute stage.
    if (machine->config().hazard_unit() != machine::MachineConfig::HU_STALL_FORWARD) {
        new_bus(lp_dc_rs.out, alu->connector_in_a())->setAxes({CON_AXIS_Y(445)});;
        new_bus(lp_dc_rt.out, ex.mux_imm->connector_in(0))->setAxes({CON_AXIS_Y(450)});
    }
    new_bus(lp_dc_immed.out, ex.mux_imm->connector_in(1))->setAxes({CON_AXIS_Y(465)});;
    lp_ex_alu = latch_ex_mem->new_connector(alu->connector_out()->y() - latch_ex_mem->y());
    lp_ex_dt = latch_ex_mem->new_connector(290);
    new_bus(alu->connector_out(), lp_ex_alu.in);
    new_bus(ex.j_mux->new_connector(CON_AX_Y), lp_ex_dt.in);
    new_bus(regdest_dc_rt.out, ex.mux_regdest->connector_in(0), 2);
    new_bus(regdest_dc_rd.out, ex.mux_regdest->connector_in(1), 2);
    regdest_ex = latch_ex_mem->new_connector(ex.mux_regdest->connector_out()->point().y() - latch_ex_mem->y());
    new_bus(ex.mux_regdest->connector_out(), regdest_ex.in, 2);
    new_signal(ctl_alu_de.out, alu->connector_ctl());
    new_signal(ctl_imm_de.out, ex.mux_imm->connector_ctl());
    new_signal(ctl_regdest_de.out, ex.mux_regdest->connector_ctl());
    ctl_memr_ex = latch_ex_mem->new_connector(ctl_memr_de.out->point().y() - latch_id_ex->y());
    new_signal(ctl_memr_de.out, ctl_memr_ex.in);
    ctl_memw_ex = latch_ex_mem->new_connector(ctl_memw_de.out->point().y() - latch_id_ex->y());
    new_signal(ctl_memw_de.out, ctl_memw_ex.in);
    ctl_mr_ex = latch_ex_mem->new_connector(ctl_mr_de.out->point().y() - latch_ex_mem->y());
    new_signal(ctl_mr_de.out, ctl_mr_ex.in);
    ctl_rgw_ex = latch_ex_mem->new_connector(ctl_rgw_de.out->point().y() - latch_ex_mem->y());
    new_signal(ctl_rgw_de.out, ctl_rgw_ex.in);

    // Memory stage.
    new_bus(lp_ex_alu.out, mm.j_addr->new_connector(CON_AX_X));
    new_bus(lp_ex_dt.out, mem_data->connector_data_in())->setAxes({CON_AXIS_Y(560)});;
    lp_mem_alu = latch_mem_wb->new_connector(180);
    lp_mem_mem = latch_mem_wb->new_connector(mem_data->connector_data_out()->y() - latch_mem_wb->y());
    new_bus(mm.j_addr->new_connector(CON_AX_Y), lp_mem_alu.in);
    new_bus(mem_data->connector_data_out(), lp_mem_mem.in);
    regdest_mem = latch_mem_wb->new_connector(ex.mux_regdest->connector_out()->point().y() - latch_mem_wb->y());
    new_bus(regdest_ex.out, regdest_mem.in, 2);
    new_signal(ctl_memw_ex.out, mem_data->connector_req_write());
    new_signal(ctl_memr_ex.out, mem_data->connector_req_read());
    ctl_mr_mem = latch_mem_wb->new_connector(ctl_mr_ex.out->point().y() - latch_mem_wb->y());
    new_signal(ctl_mr_ex.out, ctl_mr_mem.in);
    ctl_rgw_mem = latch_mem_wb->new_connector(ctl_rgw_ex.out->point().y() - latch_mem_wb->y());
    new_signal(ctl_rgw_ex.out, ctl_rgw_mem.in);

    // WriteBack stage.
    new_bus(lp_mem_alu.out, wb.mem_or_reg->connector_in(0))->setAxes({CON_AXIS_Y(678)});
    new_bus(lp_mem_mem.out, wb.mem_or_reg->connector_in(1))->setAxes({CON_AXIS_Y(678)});
    new_signal(ctl_mr_mem.out, wb.mem_or_reg->connector_ctl());

    // From decode stage to fetch stage.
    new_signal(dc.and_branch->connector_out(), ft.multiplex->connector_ctl())->setAxes({CON_AXIS_Y(370), CON_AXIS_X(490)});
    new_bus(dc.add->connector_out(), ft.multiplex->connector_in(1))->setAxes({CON_AXIS_Y(360), CON_AXIS_X(480), CON_AXIS_Y(10)});

    // From writeback to decode.
    new_bus(regdest_mem.out, regs->connector_write_reg(), 2)->setAxes({CON_AXIS_Y(680), CON_AXIS_X(500), CON_AXIS_Y(210)});
    new_signal(ctl_rgw_mem.out, regs->connector_ctl_write())->setAxes({CON_AXIS_Y(700), CON_AXIS_X(45)});

    // Control unit labels.
    new_label("RegWrite", 300, 99);

    // Label for write back stage.
    NEW_V(this, 460, 45, writeback_regw_value, false, 1);
    NEW_V(this, 360, 105, decode_regw_value, false, 1);
    NEW_V(this, 460, 105, execute_regw_value, false, 1);
    NEW_V(this, 560, 105, memory_regw_value, false, 1);

    new_label("RtE", 427, 372);
    NEW_V(this, 450, 380, execute_rt_num_value, false, 2, 0, 10, ' ');
    new_label("RdE", 427, 380);
    NEW_V(this, 450, 390, execute_rd_num_value, false, 2, 0, 10, ' ');
    NEW_V(this, 510, 385, execute_regw_num_value, false, 2, 0, 10, ' ');
    NEW_V(this, 610, 385, memory_regw_num_value, false, 2, 0, 10, ' ');

    if (machine->config().hazard_unit() == machine::MachineConfig::HU_STALL_FORWARD) {
        NEW_MUX(this, hu.mux_alu_reg_a, 430, 232, execute_reg1_ff_value, 3, false);
        NEW_MUX(this, hu.mux_alu_reg_b, 430, 285, execute_reg2_ff_value, 3, false);
        NEW_MINIMUX(this, hu.mux_branch_reg_a, 296, 228, forward_m_d_rs_value, 2, false);
        NEW_MINIMUX(this, hu.mux_branch_reg_b, 314, 228, forward_m_d_rt_value, 2, false);
        NEW(this, Junction, hu.j_alu_out, 420, 490);

        new_bus(lp_dc_rs.out, hu.mux_alu_reg_a->connector_in(0))->setAxes({CON_AXIS_Y(403)});
        new_bus(hu.mux_alu_reg_a->connector_out(), alu->connector_in_a());
        new_bus(lp_dc_rt.out, hu.mux_alu_reg_b->connector_in(0))->setAxes({CON_AXIS_Y(403)});;
        new_bus(hu.mux_alu_reg_b->connector_out(), ex.mux_imm->connector_in(0));

        new_bus(wb.j_reg_write_val->new_connector(coreview::Connector::AX_Y), hu.mux_alu_reg_a->connector_in(1));
        new_bus(wb.j_reg_write_val->new_connector(coreview::Connector::AX_Y), hu.mux_alu_reg_b->connector_in(1));

        new_bus(mm.j_addr->new_connector(CON_AX_Y), hu.j_alu_out->new_connector(CON_AX_X));
        new_bus(hu.j_alu_out->new_connector(CON_AX_Y), hu.mux_alu_reg_a->connector_in(2));
        new_bus(hu.j_alu_out->new_connector(CON_AX_Y), hu.mux_alu_reg_b->connector_in(2));

        new_bus(hu.j_alu_out->new_connector(CON_AX_X), hu.mux_branch_reg_a->connector_in(1))->setAxes({CON_AXIS_Y(380), CON_AXIS_X(330)});;
        new_bus(hu.j_alu_out->new_connector(CON_AX_X), hu.mux_branch_reg_b->connector_in(1))->setAxes({CON_AXIS_Y(380), CON_AXIS_X(330)});

        new_bus(regs_bus1->new_connector(hu.mux_branch_reg_a->connector_in(0)->point(), coreview::Connector::AX_Y), hu.mux_branch_reg_a->connector_in(0));
        new_bus(regs_bus2->new_connector(hu.mux_branch_reg_b->connector_in(0)->point(), coreview::Connector::AX_Y), hu.mux_branch_reg_b->connector_in(0));

        new_bus(hu.mux_branch_reg_a->connector_out(), dc.cmp->new_connector(-0.75, 1));
        new_bus(hu.mux_branch_reg_b->connector_out(), dc.cmp->new_connector(0.75, 1));

        regdest_dc_rs = latch_id_ex->new_connector(ex.mux_regdest->connector_in(0)->point().y() - latch_id_ex->y() - 8);
        new_bus(dc.instr_bus->new_connector(0, ex.mux_regdest->connector_in(0)->y() - 8), regdest_dc_rs.in, 2);
        new_label("RsE", 427, 364);
        NEW_V(this, 450, 370, execute_rs_num_value, false, 2, 0, 10, ' ');
        NEW(this, Junction, ex.j_rs_num, 442, 372);
        new_bus(regdest_dc_rs.out, ex.j_rs_num->new_connector(coreview::Connector::AX_X), 2);

        NEW_V(this, 434, 227, execute_reg1_ff_value, false, 1); // Register 1 forward to ALU
        NEW_V(this, 434, 280, execute_reg2_ff_value, false, 1); // Register 2 forward to ALU

        NEW_V(this, 291, 230, forward_m_d_rs_value, false, 1); // Register 1 forward for bxx and jr, jalr
        NEW_V(this, 333, 230, forward_m_d_rt_value, false, 1); // Register 2 forward for beq, bne
    }
}

CoreViewScenePipelinedPredictor::CoreViewScenePipelinedPredictor(machine::QtMipsMachine *machine) : CoreViewScene(machine) {
    const coreview::Connector *dc_con_sign_ext;
    coreview::Value *val;
    struct coreview::Latch::ConnectorPair pc_latch_pair;
    struct coreview::Latch::ConnectorPair lp_ft_inst, lp_dc_rs, lp_dc_rt, lp_dc_immed;
    struct coreview::Latch::ConnectorPair regdest_dc_rs, regdest_dc_rt, regdest_dc_rd, regdest_ex, regdest_mem;
    struct coreview::Latch::ConnectorPair ctl_alu_de, ctl_imm_de, ctl_regdest_de, ctl_memw_de, ctl_memr_de;
    struct coreview::Latch::ConnectorPair ctl_mr_de, ctl_rgw_de, ctl_memr_ex, ctl_mr_ex, ctl_rgw_ex;
    struct coreview::Latch::ConnectorPair ctl_memw_ex, ctl_mr_mem, ctl_rgw_mem;
    struct coreview::Latch::ConnectorPair lp_ex_alu, lp_ex_dt, lp_mem_alu, lp_mem_mem;
    coreview::Bus *regs_bus1, *regs_bus2;
    const coreview::Connector *regs_bus_con;
    const coreview::Connector *ctl_cnt;

    NEW(this, Latch, latch_if_id, 158, 70, machine, 400);
    latch_if_id->setTitle("IF/ID");
    NEW(this, Latch, latch_id_ex, 392, 70, machine, 400);
    latch_id_ex->setTitle("ID/EX");
    NEW(this, Latch, latch_ex_mem, 536, 70, machine, 400);
    latch_ex_mem->setTitle("EX/MEM");
    NEW(this, Latch, latch_mem_wb, 660, 70, machine, 400);
    latch_mem_wb->setTitle("MEM/WB");

    NEW_I(this, inst_fetch, 79, 2, instruction_fetched, QColor(255, 173, 173));
    NEW_I(this, inst_dec, 275, 2, instruction_decoded, QColor(255, 212, 173));
    NEW_I(this, inst_exec, 464, 2, instruction_executed, QColor(193, 255, 173));
    NEW_I(this, inst_mem, 598, 2, instruction_memory, QColor(173, 255, 229));
    NEW_I(this, inst_wrb, 660, 18, instruction_writeback, QColor(255, 173, 230));

    if (machine->config().hazard_unit() != machine::MachineConfig::HU_NONE) {
        NEW(this, LogicBlock, hazard_unit, SC_WIDTH/2, SC_HEIGHT - 15, "Hazard Unit");
        hazard_unit->setSize(SC_WIDTH - 100, 12);
        static QMap<std::uint32_t, QString> stall_map = {{0, "NORMAL"},{1, "STALL"},{2, "FORWARD"}};
        NEW_MULTI(this, hu.multi_stall, 480, 447, execute_stall_forward_value, stall_map);
        NEW_MULTI(this, hu.multi_stall, 310, 340, branch_forward_value, stall_map);
        NEW_MULTI(this, hu.multi_stall, 250, SC_HEIGHT - 15, hu_stall_value, stall_map);
    }

    // Fetch stage.
    NEW(this, ProgramCounter, ft.pc, -18, 280, machine);
    NEW(this, Predictor, ft.pred, ft.pc->x() + 15, 160);
    NEW(this, Latch, ft.latch, 35, 250, machine, 20);
    NEW(this, Adder, ft.add[0], 80, 320);
    NEW(this, Adder, ft.add[1], 125, 433);
    pc_latch_pair = ft.latch->new_connector(10);
    NEW_B(this, Constant, ft.adder_4, ft.add[0]->connector_in_b(), "4");
    NEW_MUX(this, ft.mul[0], -35, 380, fetch_branch_value, 2, true);
    NEW_MUX(this, ft.mul[1], 5, 430, fetch_branch_value, 2);
    NEW(this, LogicBlock, ft.sign_ext, 50, 405, {"Sign", "extension"});
    NEW(this, LogicBlock, ft.shift2, 95, ft.add[1]->y(), "<<2");
    NEW(this, Junction, ft.j_pc, 60, mem_program->connector_address()->y());
    NEW(this, Junction, ft.j_adder, ft.add[0]->x() + 35, 430);
    NEW(this, Junction, ft.j_mul_cmp, -15, 520);
    ft.con_pc_pred = new coreview::Connector();
    ft.con_pc_pred->setPos(ft.pc->x() + ft.pc->width() / 2, ft.pc->y());

    // Decode stage.
    NEW(this, LogicBlock, dc.ctl_block, 230, 90, {"Control", "unit"});
    dc.ctl_block->setSize(35, 70);
    NEW(this, LogicBlock, dc.cmp[0], 250, 483, "!=");
    NEW(this, LogicBlock, dc.cmp[1], 312, 200, "=");
    dc.cmp[0]->setSize(24, 10);
    dc.cmp[1]->setSize(24, 12);
    NEW(this, And, dc.and_branch, 350, 190);
    NEW_MUX(this, dc.mul, 370, 430, fetch_branch_value, 2, true);
    NEW(this, Junction, dc.j_inst_up, 190, 126);
    dc_con_sign_ext = ft.sign_ext->new_connector(1, 0);
    NEW(this, Junction, dc.j_inst_down, 190, dc_con_sign_ext->y());
    NEW(this, Junction, dc.j_jalpctor31, 365, 100);
    NEW(this, Junction, dc.j_jump_reg, 355, 94);
    NEW(this, Junction, dc.j_add_mul, 150, 510);
    NEW(this, Junction, dc.j_mul_cmp, 385, 520);
    NEW(this, Junction, dc.j_sign_ext, 290, dc_con_sign_ext->y());

    // From decode to fetch stage.
    NEW(this, Junction, j_dc_mul_ft_mul, ft.add[1]->x(), ft.add[1]->y() + 42);

    // Connections
    // Fetch stage.
    new_bus(ft.pc->connector_out(), pc_latch_pair.in);
    new_bus(pc_latch_pair.out, ft.j_pc->new_connector(coreview::Connector::AX_X));
    new_bus(ft.j_pc->new_connector(coreview::Connector::AX_Y), ft.add[0]->connector_in_a());
    new_bus(ft.j_pc->new_connector(coreview::Connector::AX_X), mem_program->connector_address());
    new_bus(ft.mul[0]->connector_out(), ft.mul[1]->connector_in(0))->setAxes({CON_AXIS_Y(-15)});
    new_signal(ft.mul[0]->connector_ctl(), ft.pred->connector_sig_out());
    new_bus(ft.mul[1]->connector_out(), ft.pc->connector_in());
    new_bus(ft.add[0]->connector_out(), ft.j_adder->new_connector(coreview::Connector::AX_Y));
    new_bus(ft.con_pc_pred, ft.pred->connector_pc_in());
    new_bus(ft.j_adder->new_connector(coreview::Connector::AX_Y), ft.add[1]->connector_in_b());
    new_bus(ft.j_adder->new_connector(), dc.mul->connector_in(0))->setAxes({CON_AXIS_X(10), CON_AXIS_Y(360)});
    new_bus(ft.j_adder->new_connector(coreview::Connector::AX_Y), ft.mul[0]->connector_in(0))->setAxes({CON_AXIS_X(370), CON_AXIS_Y(-40)});
    new_bus(ft.shift2->new_connector(1, 0), ft.add[1]->connector_in_a());
    dc.j_sign_ext->setPos(80, dc.j_sign_ext->y());
    new_bus(dc.j_sign_ext->new_connector(coreview::Connector::AX_Y), ft.shift2->new_connector(-1, 0));
    new_bus(ft.j_mul_cmp->new_connector(), ft.mul[0]->connector_out())->setAxes({CON_AXIS_Y(-15)});
    new_bus(ft.j_mul_cmp->new_connector(), dc.cmp[0]->new_connector(-0.5, 1));

    lp_ft_inst = latch_if_id->new_connector(mem_program->connector_instruction()->y() - latch_if_id->y());
    new_bus(mem_program->connector_instruction(), lp_ft_inst.in);

    // Decode stage.
    new_bus(dc_con_sign_ext, dc.j_sign_ext->new_connector(coreview::Connector::AX_X));
    new_bus(dc.mul->connector_out(), dc.j_mul_cmp->new_connector())->setAxes({CON_AXIS_Y(385)});
    new_bus(dc.j_mul_cmp->new_connector(), dc.cmp[0]->new_connector(0.5, 1));
    new_signal(dc.mul->connector_ctl(), dc.and_branch->connector_out());
    new_bus(dc_con_sign_ext, dc.j_sign_ext->new_connector(coreview::Connector::AX_X));
    new_signal(dc.cmp[1]->new_connector(1, 0), dc.and_branch->connector_in(1))->setAxes({CON_AXIS_Y(343)});
    new_signal(dc.ctl_block->new_connector(1, 0.8), dc.and_branch->connector_in(0))->setAxes({CON_AXIS_Y(343)});
    dc.instr_bus = new_bus(dc.j_inst_up->new_connector(coreview::Connector::AX_Y), dc.j_inst_down->new_connector(coreview::Connector::AX_Y));
    new_bus(dc.j_inst_up->new_connector(coreview::Connector::AX_X), dc.ctl_block->new_connector(-1, 0), 2);
    new_bus(dc.instr_bus->new_connector(regs->connector_read1_reg()->point()), regs->connector_read1_reg(), 2);
    new_bus(dc.instr_bus->new_connector(regs->connector_read2_reg()->point()), regs->connector_read2_reg(), 2);
    new_signal(dc.ctl_block->new_connector(1, -0.8), dc.j_jalpctor31->new_connector(coreview::Connector::AX_X));
    new_signal(dc.ctl_block->new_connector(1, -10), dc.j_jump_reg->new_connector(coreview::Connector::AX_X));


    new_bus(lp_ft_inst.out, dc.instr_bus->new_connector(lp_ft_inst.out->point()));
    lp_dc_rs = latch_id_ex->new_connector(regs->connector_read1()->y() - latch_id_ex->y());
    lp_dc_rt = latch_id_ex->new_connector(regs->connector_read2()->y() - latch_id_ex->y());
    regs_bus1 = new_bus(regs->connector_read1(), lp_dc_rs.in);
    regs_bus2 = new_bus(regs->connector_read2(), lp_dc_rt.in);

    if (machine->config().hazard_unit() != machine::MachineConfig::HU_STALL_FORWARD) {
        regs_bus_con = dc.cmp[1]->new_connector(-0.5, 1);
        new_bus(regs_bus1->new_connector(regs_bus_con->point(), coreview::Connector::AX_Y), regs_bus_con);
        regs_bus_con = dc.cmp[1]->new_connector(0.5, 1);
        new_bus(regs_bus2->new_connector(regs_bus_con->point(), coreview::Connector::AX_Y), regs_bus_con);
    }

    lp_dc_immed = latch_id_ex->new_connector(dc.j_sign_ext->y() - latch_id_ex->y());
    new_bus(dc.j_sign_ext->new_connector(coreview::Connector::AX_X), lp_dc_immed.in);
    regdest_dc_rt = latch_id_ex->new_connector(ex.mux_regdest->connector_in(0)->point().y() - latch_id_ex->y());
    regdest_dc_rd = latch_id_ex->new_connector(ex.mux_regdest->connector_in(1)->point().y() - latch_id_ex->y());
    new_bus(dc.instr_bus->new_connector(ex.mux_regdest->connector_in(0)->point()), regdest_dc_rt.in, 2);
    new_bus(dc.instr_bus->new_connector(ex.mux_regdest->connector_in(1)->point()), regdest_dc_rd.in, 2);
    ctl_cnt = dc.ctl_block->new_connector(1, 0.2);

    ctl_alu_de = latch_id_ex->new_connector(ctl_cnt->point().y() - latch_id_ex->y());
    new_signal(ctl_cnt, ctl_alu_de.in);
    ctl_cnt = dc.ctl_block->new_connector(1, 0.6);
    ctl_imm_de = latch_id_ex->new_connector(ctl_cnt->point().y() - latch_id_ex->y());
    new_signal(ctl_cnt, ctl_imm_de.in);
    ctl_cnt = dc.ctl_block->new_connector(1, 0.4);
    ctl_regdest_de = latch_id_ex->new_connector(ctl_cnt->point().y() - latch_id_ex->y());
    new_signal(ctl_cnt, ctl_regdest_de.in);
    ctl_cnt = dc.ctl_block->new_connector(1, 0.0);
    ctl_memw_de = latch_id_ex->new_connector(ctl_cnt->point().y() - latch_id_ex->y());
    new_signal(ctl_cnt, ctl_memw_de.in);
    ctl_cnt = dc.ctl_block->new_connector(1, -0.2);
    ctl_memr_de = latch_id_ex->new_connector(ctl_cnt->point().y() - latch_id_ex->y());
    new_signal(ctl_cnt, ctl_memr_de.in);
    ctl_cnt = dc.ctl_block->new_connector(1, -0.4);
    ctl_mr_de = latch_id_ex->new_connector(ctl_cnt->point().y() - latch_id_ex->y());
    new_signal(ctl_cnt, ctl_mr_de.in);
    ctl_cnt = dc.ctl_block->new_connector(1, -0.6);
    ctl_rgw_de = latch_id_ex->new_connector(ctl_cnt->point().y() - latch_id_ex->y());
    new_signal(ctl_cnt, ctl_rgw_de.in);

    // Execute stage.
    if (machine->config().hazard_unit() != machine::MachineConfig::HU_STALL_FORWARD) {
        new_bus(lp_dc_rs.out, alu->connector_in_a())->setAxes({CON_AXIS_Y(445)});;
        new_bus(lp_dc_rt.out, ex.mux_imm->connector_in(0))->setAxes({CON_AXIS_Y(450)});
    }
    new_bus(lp_dc_immed.out, ex.mux_imm->connector_in(1))->setAxes({CON_AXIS_Y(465)});;
    lp_ex_alu = latch_ex_mem->new_connector(alu->connector_out()->y() - latch_ex_mem->y());
    lp_ex_dt = latch_ex_mem->new_connector(290);
    new_bus(alu->connector_out(), lp_ex_alu.in);
    new_bus(ex.j_mux->new_connector(CON_AX_Y), lp_ex_dt.in);
    new_bus(regdest_dc_rt.out, ex.mux_regdest->connector_in(0), 2);
    new_bus(regdest_dc_rd.out, ex.mux_regdest->connector_in(1), 2);
    regdest_ex = latch_ex_mem->new_connector(ex.mux_regdest->connector_out()->point().y() - latch_ex_mem->y());
    new_bus(ex.mux_regdest->connector_out(), regdest_ex.in, 2);
    new_signal(ctl_alu_de.out, alu->connector_ctl());
    new_signal(ctl_imm_de.out, ex.mux_imm->connector_ctl());
    new_signal(ctl_regdest_de.out, ex.mux_regdest->connector_ctl());
    ctl_memr_ex = latch_ex_mem->new_connector(ctl_memr_de.out->point().y() - latch_id_ex->y());
    new_signal(ctl_memr_de.out, ctl_memr_ex.in);
    ctl_memw_ex = latch_ex_mem->new_connector(ctl_memw_de.out->point().y() - latch_id_ex->y());
    new_signal(ctl_memw_de.out, ctl_memw_ex.in);
    ctl_mr_ex = latch_ex_mem->new_connector(ctl_mr_de.out->point().y() - latch_ex_mem->y());
    new_signal(ctl_mr_de.out, ctl_mr_ex.in);
    ctl_rgw_ex = latch_ex_mem->new_connector(ctl_rgw_de.out->point().y() - latch_ex_mem->y());
    new_signal(ctl_rgw_de.out, ctl_rgw_ex.in);

    // Memory stage.
    new_bus(lp_ex_alu.out, mm.j_addr->new_connector(CON_AX_X));
    new_bus(lp_ex_dt.out, mem_data->connector_data_in())->setAxes({CON_AXIS_Y(560)});;
    lp_mem_alu = latch_mem_wb->new_connector(180);
    lp_mem_mem = latch_mem_wb->new_connector(mem_data->connector_data_out()->y() - latch_mem_wb->y());
    new_bus(mm.j_addr->new_connector(CON_AX_Y), lp_mem_alu.in);
    new_bus(mem_data->connector_data_out(), lp_mem_mem.in);
    regdest_mem = latch_mem_wb->new_connector(ex.mux_regdest->connector_out()->point().y() - latch_mem_wb->y());
    new_bus(regdest_ex.out, regdest_mem.in, 2);
    new_signal(ctl_memw_ex.out, mem_data->connector_req_write());
    new_signal(ctl_memr_ex.out, mem_data->connector_req_read());
    ctl_mr_mem = latch_mem_wb->new_connector(ctl_mr_ex.out->point().y() - latch_mem_wb->y());
    new_signal(ctl_mr_ex.out, ctl_mr_mem.in);
    ctl_rgw_mem = latch_mem_wb->new_connector(ctl_rgw_ex.out->point().y() - latch_mem_wb->y());
    new_signal(ctl_rgw_ex.out, ctl_rgw_mem.in);

    // WriteBack stage.
    new_bus(lp_mem_alu.out, wb.mem_or_reg->connector_in(0))->setAxes({CON_AXIS_Y(678)});
    new_bus(lp_mem_mem.out, wb.mem_or_reg->connector_in(1))->setAxes({CON_AXIS_Y(678)});
    new_signal(ctl_mr_mem.out, wb.mem_or_reg->connector_ctl());

    // From writeback to decode.
    new_bus(regdest_mem.out, regs->connector_write_reg(), 2)->setAxes({CON_AXIS_Y(680), CON_AXIS_X(500), CON_AXIS_Y(210)});
    new_signal(ctl_rgw_mem.out, regs->connector_ctl_write())->setAxes({CON_AXIS_Y(700), CON_AXIS_X(45)});

    // From decode to fetch stage.
    new_signal(dc.cmp[0]->new_connector(-1, 0), ft.mul[1]->connector_ctl());
    new_bus(ft.add[1]->connector_out(), dc.mul->connector_in(1))->setAxes({CON_AXIS_X(10), CON_AXIS_Y(360)});
    new_bus(dc.j_add_mul->new_connector(coreview::Connector::AX_Y), ft.add[1]->connector_out());
    new_bus(dc.j_add_mul->new_connector(coreview::Connector::AX_Y), ft.mul[0]->connector_in(1))->setAxes({CON_AXIS_X(510), CON_AXIS_Y(-40)});
    new_bus(dc.mul->connector_out(), j_dc_mul_ft_mul->new_connector())->setAxes({CON_AXIS_Y(385)});
    new_bus(j_dc_mul_ft_mul->new_connector(), ft.mul[1]->connector_in(1))->setAxes({CON_AXIS_X(510), CON_AXIS_Y(-5)});

    NEW_V(this, ft.mul[0]->x() + 5, ft.mul[0]->y() - 25,  fetch_predictor_value, false, 1);
    NEW_V(this, ft.mul[1]->x() + 4, ft.mul[1]->y() + 35,  fetch_nequal_value, false, 1);
    NEW_V(this, dc.mul->x() + 5, dc.mul->y() - 25, fetch_branch_value, false, 1);

    // Control unit labels.
    new_label("RegWrite", 300, 99);

    // Label for write back stage.
    NEW_V(this, 460, 45, writeback_regw_value, false, 1);
    NEW_V(this, 360, 105, decode_regw_value, false, 1);
    NEW_V(this, 460, 105, execute_regw_value, false, 1);
    NEW_V(this, 560, 105, memory_regw_value, false, 1);

    new_label("RtE", 427, 372);
    NEW_V(this, 450, 380, execute_rt_num_value, false, 2, 0, 10, ' ');
    new_label("RdE", 427, 380);
    NEW_V(this, 450, 390, execute_rd_num_value, false, 2, 0, 10, ' ');
    NEW_V(this, 510, 385, execute_regw_num_value, false, 2, 0, 10, ' ');
    NEW_V(this, 610, 385, memory_regw_num_value, false, 2, 0, 10, ' ');

    if (machine->config().hazard_unit() == machine::MachineConfig::HU_STALL_FORWARD) {
        NEW_MUX(this, hu.mux_alu_reg_a, 430, 232, execute_reg1_ff_value, 3, false);
        NEW_MUX(this, hu.mux_alu_reg_b, 430, 285, execute_reg2_ff_value, 3, false);
        NEW_MINIMUX(this, hu.mux_branch_reg_a, 296, 228, forward_m_d_rs_value, 2, false);
        NEW_MINIMUX(this, hu.mux_branch_reg_b, 314, 228, forward_m_d_rt_value, 2, false);
        NEW(this, Junction, hu.j_alu_out, 420, 490);

        new_bus(lp_dc_rs.out, hu.mux_alu_reg_a->connector_in(0))->setAxes({CON_AXIS_Y(403)});
        new_bus(hu.mux_alu_reg_a->connector_out(), alu->connector_in_a());
        new_bus(lp_dc_rt.out, hu.mux_alu_reg_b->connector_in(0))->setAxes({CON_AXIS_Y(403)});;
        new_bus(hu.mux_alu_reg_b->connector_out(), ex.mux_imm->connector_in(0));

        new_bus(wb.j_reg_write_val->new_connector(coreview::Connector::AX_Y), hu.mux_alu_reg_a->connector_in(1));
        new_bus(wb.j_reg_write_val->new_connector(coreview::Connector::AX_Y), hu.mux_alu_reg_b->connector_in(1));

        new_bus(mm.j_addr->new_connector(CON_AX_Y), hu.j_alu_out->new_connector(CON_AX_X));
        new_bus(hu.j_alu_out->new_connector(CON_AX_Y), hu.mux_alu_reg_a->connector_in(2));
        new_bus(hu.j_alu_out->new_connector(CON_AX_Y), hu.mux_alu_reg_b->connector_in(2));

        new_bus(hu.j_alu_out->new_connector(CON_AX_X), hu.mux_branch_reg_a->connector_in(1))->setAxes({CON_AXIS_Y(402), CON_AXIS_X(330)});;
        new_bus(hu.j_alu_out->new_connector(CON_AX_X), hu.mux_branch_reg_b->connector_in(1))->setAxes({CON_AXIS_Y(402), CON_AXIS_X(330)});

        new_bus(regs_bus1->new_connector(hu.mux_branch_reg_a->connector_in(0)->point(), coreview::Connector::AX_Y), hu.mux_branch_reg_a->connector_in(0));
        new_bus(regs_bus2->new_connector(hu.mux_branch_reg_b->connector_in(0)->point(), coreview::Connector::AX_Y), hu.mux_branch_reg_b->connector_in(0));

        new_bus(hu.mux_branch_reg_a->connector_out(), dc.cmp[1]->new_connector(-0.75, 1));
        new_bus(hu.mux_branch_reg_b->connector_out(), dc.cmp[1]->new_connector(0.75, 1));

        regdest_dc_rs = latch_id_ex->new_connector(ex.mux_regdest->connector_in(0)->point().y() - latch_id_ex->y() - 8);
        new_bus(dc.instr_bus->new_connector(0, ex.mux_regdest->connector_in(0)->y() - 8), regdest_dc_rs.in, 2);
        new_label("RsE", 427, 364);
        NEW_V(this, 450, 370, execute_rs_num_value, false, 2, 0, 10, ' ');
        NEW(this, Junction, ex.j_rs_num, 442, 372);
        new_bus(regdest_dc_rs.out, ex.j_rs_num->new_connector(coreview::Connector::AX_X), 2);

        NEW_V(this, 434, 227, execute_reg1_ff_value, false, 1); // Register 1 forward to ALU
        NEW_V(this, 434, 280, execute_reg2_ff_value, false, 1); // Register 2 forward to ALU

        NEW_V(this, 291, 230, forward_m_d_rs_value, false, 1); // Register 1 forward for bxx and jr, jalr
        NEW_V(this, 333, 230, forward_m_d_rt_value, false, 1); // Register 2 forward for beq, bne
    }

    connect(ft.pred, SIGNAL(open_predictor()), this, SIGNAL(request_predictor()));
}
