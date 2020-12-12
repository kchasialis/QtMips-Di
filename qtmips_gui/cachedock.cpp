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

#include "cachedock.h"

CacheDock::CacheDock(QWidget *parent, const QString &type) : QDockWidget(parent) {
    top_widget = new QWidget(this);
    setWidget(top_widget);
    layout_box = new QVBoxLayout(top_widget);

    top_form = new QWidget(top_widget);
    top_form->setVisible(false);
    layout_box->addWidget(top_form);
    layout_top_form = new QFormLayout(top_form);

    l_hit = new QLabel("0", top_form);
    layout_top_form->addRow("Hit:", l_hit);
    l_miss = new QLabel("0", top_form);
    layout_top_form->addRow("Miss:", l_miss);
    l_m_reads = new QLabel("0", top_form);
    layout_top_form->addRow("Memory reads:", l_m_reads);
    l_m_writes = new QLabel("0", top_form);
    layout_top_form->addRow("Memory writes:", l_m_writes);
    l_stalled = new QLabel("0", top_form);
    layout_top_form->addRow("Memory stall cycles:", l_stalled);
    l_hit_rate = new QLabel("0.000%", top_form);
    layout_top_form->addRow("Hit rate:", l_hit_rate);
    l_speed  = new QLabel("100%", top_form);
    layout_top_form->addRow("Improved speed:", l_speed);

    graphicsview = new GraphicsView(top_widget);
    graphicsview->setVisible(false);
    layout_box->addWidget(graphicsview);
    cachescene = nullptr;

    no_cache = new QLabel("No " + type + " Cache configured", top_widget);
    layout_box->addWidget(no_cache);

    setObjectName(type + "Cache");
    setWindowTitle(type + " Cache");
}

void CacheDock::setup(const machine::Cache *cache) {
    l_hit->setText("0");
    l_miss->setText("0");
    l_stalled->setText("0");
    l_m_reads->setText("0");
    l_m_writes->setText("0");
    l_hit_rate->setText("0.000%");
    l_speed->setText("100%");
    if (cache != nullptr) {
        connect(cache, SIGNAL(hit_update(uint)), this, SLOT(hit_update(uint)));
        connect(cache, SIGNAL(miss_update(uint)), this, SLOT(miss_update(uint)));
        connect(cache, SIGNAL(memory_reads_update(uint)), this, SLOT(memory_reads_update(uint)));
        connect(cache, SIGNAL(memory_writes_update(uint)), this, SLOT(memory_writes_update(uint)));
        connect(cache, SIGNAL(statistics_update(uint,double,double)), this, SLOT(statistics_update(uint,double,double)));
    }
    top_form->setVisible(cache != nullptr);
    no_cache->setVisible(!cache->config().enabled());

    if (cachescene)
        delete cachescene;
    cachescene = new CacheViewScene(cache);
    graphicsview->setScene(cachescene);
    graphicsview->setVisible(cache->config().enabled());
}

void CacheDock::hit_update(unsigned val) {
    l_hit->setText(QString::number(val));
}

void CacheDock::miss_update(unsigned val) {
    l_miss->setText(QString::number(val));
}

void CacheDock::memory_reads_update(unsigned val) {
    l_m_reads->setText(QString::number(val));
}

void CacheDock::memory_writes_update(unsigned val) {
    l_m_writes->setText(QString::number(val));
}

void CacheDock::statistics_update(unsigned stalled_cycles, double speed_improv, double hit_rate) {
    l_stalled->setText(QString::number(stalled_cycles));
    l_hit_rate->setText(QString::number(hit_rate, 'f', 3) + QString("%"));
    l_speed->setText(QString::number(speed_improv, 'f', 0) + QString("%"));
}
