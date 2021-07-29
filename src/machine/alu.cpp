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

#include "alu.h"
#include "qtmipsexception.h"
#include "utils.h"

using namespace machine;

#if defined(__GNUC__) && __GNUC__ >= 4

static inline std::uint32_t alu_op_clz(std::uint32_t n)
{
    int intbits = sizeof(int) * CHAR_BIT;
    if (n == 0)
        return 32;
    return __builtin_clz(n) - (intbits - 32);
}

#else /* Fallback for generic compiler */

// see https://en.wikipedia.org/wiki/Find_first_set#CLZ
static const std::uint8_t sig_table_4bit[16] =
                       { 0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4 };

static inline std::uint32_t alu_op_clz(std::uint32_t n)
{
    int len = 32;

    if (n & 0xFFFF0000) {
        len -= 16;
        n >>= 16;
    }
    if (n & 0xFF00) {
        len -= 8;
        n >>= 8;
    }
    if (n & 0xF0) {
        len -= 4;
        n >>= 4;
    }
    len -= sig_table_4bit[n];
    return len;
}

#endif /* end of mips_clz */

static inline std::uint64_t alu_read_hi_lo_64bit(Registers *regs)
{
    std::uint64_t val;
    val = regs->read_hi_lo(false);
    val |= (std::uint64_t)regs->read_hi_lo(true) << 32;
    return val;
}

static inline void alu_write_hi_lo_64bit(Registers *regs, std::uint64_t val)
{
    regs->write_hi_lo(false, (std::uint32_t)(val & 0xffffffff));
    regs->write_hi_lo(true,  (std::uint32_t)(val >> 32));
}


std::uint32_t machine::alu_operate(enum AluOp operation, std::uint32_t s,
                                   std::uint32_t t, std::uint8_t sa, std::uint8_t sz,
                                   Registers *regs, bool &discard,
                                   ExceptionCause &excause) {
    std::int64_t s64_val;
    std::uint64_t u64_val;
    std::uint32_t u32_val;
    discard = false;

    switch(operation) {
        case ALU_OP_NOP:
            return 0;
        case ALU_OP_SLL:
            return t << sa;
        case ALU_OP_SRL:
            return t >> sa;
        case ALU_OP_ROTR:
            if (!sa)
                return t;
            return (t >> sa) | (t << (32 - sa));
        case ALU_OP_SRA:
            // Note: This might be broken with some compilers but works with gcc
            return (std::int32_t)t >> sa;
        case ALU_OP_SLLV:
            return t << (s & 0x1f);
        case ALU_OP_SRLV:
            return t >> (s & 0x1f);
        case ALU_OP_ROTRV:
            u32_val = s & 0x1f;
            if (!u32_val)
                return t;
            return (t >> u32_val) | (t << (32 - u32_val));
        case ALU_OP_SRAV:
            // Note: same note as in case of SRA
            return (std::int32_t)t >> (s & 0x1f);
        case ALU_OP_MOVZ:
            // Signal discard of result when condition is not true
            discard = t != 0;
            return discard ? 0: s;
        case ALU_OP_MOVN:
            // Same note as for MOVZ applies here
            discard = t == 0;
            return discard ? 0: s;
        case ALU_OP_MFHI:
            return regs->read_hi_lo(true);
        case ALU_OP_MTHI:
            regs->write_hi_lo(true, s);
            return 0x0;
        case ALU_OP_MFLO:
            return regs->read_hi_lo(false);
        case ALU_OP_MTLO:
            regs->write_hi_lo(false, s);
            return 0x0;
        case ALU_OP_MULT:
            s64_val = (std::int64_t)(std::int32_t)s * (std::int32_t)t;
            alu_write_hi_lo_64bit(regs, (std::uint64_t)s64_val);
            return 0x0;
        case ALU_OP_MULTU:
            u64_val = (std::uint64_t)s * t;
            alu_write_hi_lo_64bit(regs, u64_val);
            return 0x0;
        case ALU_OP_DIV:
            if (t == 0) {
                regs->write_hi_lo(false, 0);
                regs->write_hi_lo(true, 0);
                return 0;
            }
            regs->write_hi_lo(false, (std::uint32_t)((std::int32_t)s / (std::int32_t)t));
            regs->write_hi_lo(true,  (std::uint32_t)((std::int32_t)s % (std::int32_t)t));
            return 0x0;
        case ALU_OP_DIVU:
            if (t == 0) {
                regs->write_hi_lo(false, 0);
                regs->write_hi_lo(true, 0);
                return 0;
            }
            regs->write_hi_lo(false, s / t);
            regs->write_hi_lo(true, s % t);
            return 0x0;
        case ALU_OP_ADD:
            /* s(31) ^ ~t(31) ... same signs on input  */
            /* (s + t)(31) ^ s(31)  ... different sign on output */
            if (((s ^ ~t) & ((s + t) ^ s)) & 0x80000000)
                excause = EXCAUSE_OVERFLOW;
            FALLTROUGH
        case ALU_OP_ADDU:
            return s + t;
        case ALU_OP_SUB:
            /* s(31) ^ t(31) ... differnt signd on input */
            /* (s - t)(31) ^ ~s(31)  <> 0 ... otput sign differs from s  */
            if (((s ^ t) & ((s - t) ^ s)) & 0x80000000)
                excause = EXCAUSE_OVERFLOW;
            FALLTROUGH
        case ALU_OP_SUBU:
            return s - t;
        case ALU_OP_AND:
            return s & t;
        case ALU_OP_OR:
            return s | t;
        case ALU_OP_XOR:
            return s ^ t;
        case ALU_OP_NOR:
            return ~(s | t);
        case ALU_OP_SLT:
            // Note: this is in two's complement so there is difference in unsigned and signed compare
            return ((std::int32_t)s < (std::int32_t)t) ? 1 : 0;
        case ALU_OP_SLTU:
            return (s < t) ? 1 : 0;
        case ALU_OP_MUL:
            return (std::uint32_t)((std::int32_t)s * (std::int32_t)t);
        case ALU_OP_MADD:
            s64_val = (std::int64_t)alu_read_hi_lo_64bit(regs);
            s64_val += (std::int64_t)(std::int32_t)s * (std::int32_t)t;
            alu_write_hi_lo_64bit(regs, (std::uint64_t)s64_val);
            return 0x0;
        case ALU_OP_MADDU:
            u64_val = alu_read_hi_lo_64bit(regs);
            u64_val += (std::uint64_t)s * t;
            alu_write_hi_lo_64bit(regs, u64_val);
            return 0x0;
        case ALU_OP_MSUB:
            s64_val = (std::int64_t)alu_read_hi_lo_64bit(regs);
            s64_val -= (std::int64_t)(std::int32_t)s * (std::int32_t)t;
            alu_write_hi_lo_64bit(regs, (std::uint64_t)s64_val);
            return 0x0;
        case ALU_OP_MSUBU:
            u64_val = alu_read_hi_lo_64bit(regs);
            u64_val -= (std::uint64_t)s * t;
            alu_write_hi_lo_64bit(regs, u64_val);
            return 0x0;
        case ALU_OP_TGE:
            if ((std::int32_t)s >= (std::int32_t)t)
                excause = EXCAUSE_TRAP;
            return 0;
        case ALU_OP_TGEU:
            if (s >= t)
                excause = EXCAUSE_TRAP;
            return 0;
        case ALU_OP_TLT:
            if ((std::int32_t)s < (std::int32_t)t)
                excause = EXCAUSE_TRAP;
            return 0;
        case ALU_OP_TLTU:
            if (s < t)
                excause = EXCAUSE_TRAP;
            return 0;
        case ALU_OP_TEQ:
            if (s == t)
                excause = EXCAUSE_TRAP;
            return 0;
        case ALU_OP_TNE:
            if (s != t)
                excause = EXCAUSE_TRAP;
            return 0;
        case ALU_OP_LUI:
            return t << 16;
        case ALU_OP_WSBH:
            return ((t << 8) & 0xff00ff00) | ((t >> 8) & 0x00ff00ff);
        case ALU_OP_SEB:
            return (uint32_t)(int32_t)(int8_t)t;
        case ALU_OP_SEH:
            return (uint32_t)(int32_t)(int16_t)t;
        case ALU_OP_EXT:
            return (s >> sa) & ((1 << (sz + 1)) - 1);
        case ALU_OP_INS:
            u32_val = (1 << (sz + 1)) - 1;
            return ((s & u32_val) << sa) | (t & ~(u32_val << sa));
        case ALU_OP_CLZ:
            return alu_op_clz(s);
        case ALU_OP_CLO:
           return alu_op_clz(~s);
        case ALU_OP_PASS_T: // Pass s argument without change for JAL
            return t;
        case ALU_OP_BREAK:
        case ALU_OP_SYSCALL:
        case ALU_OP_RDHWR:
        case ALU_OP_MTC0:
        case ALU_OP_MFC0:
        case ALU_OP_MFMC0:
        case ALU_OP_ERET:
            return 0;
        default:
            throw QTMIPS_EXCEPTION(UnsupportedAluOperation, "Unknown ALU operation", QString::number(operation, 16));
    }
}
