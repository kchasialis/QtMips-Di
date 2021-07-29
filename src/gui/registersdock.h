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

#ifndef REGISTERSDOCK_H
#define REGISTERSDOCK_H

#include <QDockWidget>
#include <QLabel>
#include <QFormLayout>
#include <QScrollArea>
#include <QPropertyAnimation>
#include <QPalette>
#include "qtmipsmachine.h"
#include "statictable.h"

class RegistersDock : public QDockWidget {
    Q_OBJECT
public:
    RegistersDock(QWidget *parent);
    ~RegistersDock();

    void setup(machine::QtMipsMachine *machine);

private slots:
    void pc_changed(std::uint32_t val);
    void gp_changed(std::uint8_t i, std::uint32_t val);
    void hi_lo_changed(bool hi, std::uint32_t val);
    void gp_read(std::uint8_t i, std::uint32_t val);
    void hi_lo_read(bool hi, std::uint32_t val);
    void clear_highlights();

private:
    StaticTable *widg;
    QScrollArea *scrollarea;

    QLabel *pc;
    QLabel *hi;
    QLabel *lo;
    QLabel *gp[32];

    std::uint32_t gp_highlighted;
    bool hi_highlighted;
    bool lo_highlighted;

    QPalette pal_normal;
    QPalette pal_updated;
    QPalette pal_read;

    void labelVal(QLabel *label, std::uint32_t val);
};

#endif // REGISTERSDOCK_H
