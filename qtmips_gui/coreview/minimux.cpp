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

#include "minimux.h"
#include <cmath>

using namespace coreview;

//////////////////////
#define WIDTH 6
#define HEIGHT 8
#define GAP 8
#define PENW 1
//////////////////////

MiniMux::MiniMux(unsigned size, bool ctl_up): QGraphicsObject(nullptr) {
    this->size = size;
    seton = -1;
    ctlfrom = ctl_up;
    con_ctl = new Connector(Connector::AX_X);
    con_out = new Connector(Connector::AX_Y);
    con_in = new Connector*[size];
    for (unsigned i = 0; i < size; i++)
        con_in[i] = new Connector(Connector::AX_Y);
    setPos(x(), y()); // Set connectors possitions
}

MiniMux::~MiniMux() {
    delete con_ctl;
    delete con_out;
    for (unsigned i = 0; i < size; i++)
        delete con_in[i];
    delete[] con_in;
}

#define C_WIDTH (WIDTH + (GAP * (size -1 )))

QRectF MiniMux::boundingRect() const {
    return QRectF(-PENW / 2, -PENW / 2, C_WIDTH + PENW, WIDTH + PENW);
}

void MiniMux::paint(QPainter *painter, const QStyleOptionGraphicsItem *option __attribute__((unused)), QWidget *widget __attribute__((unused))) {
    painter->setPen(QPen(QColor(200, 200, 200), 2));
    if (seton >= 0 && seton < (int)size)
        painter->drawLine((WIDTH / 2) + (seton * GAP), HEIGHT, C_WIDTH / 2, 0);

    painter->setPen(QColor(0, 0, 0));
    const QPointF poly[] = {
        QPointF(0, HEIGHT),
        QPointF(HEIGHT / 2, 0),
        QPointF(C_WIDTH - (HEIGHT / 2), 0),
        QPointF(C_WIDTH, HEIGHT)
    };
    painter->drawPolygon(poly, sizeof(poly) / sizeof(QPointF));
}

void MiniMux::setPos(qreal x, qreal y) {
    QGraphicsItem::setPos(x, y);
    /*if (ctlfrom)
        con_ctl->setPos(x + (WIDTH / 2), y + (WIDTH / 4));
    else
        con_ctl->setPos(x + (WIDTH / 2), y + C_HEIGHT - (WIDTH / 4));
    */
    con_out->setPos(x + (C_WIDTH / 2), y);
    for (unsigned i = 0; i < size; i++)
        con_in[i]->setPos(x + (WIDTH / 2) + (i * GAP), y + HEIGHT);
}

const Connector *MiniMux::connector_ctl() const {
    return con_ctl;
}

const Connector *MiniMux::connector_out() const {
    return con_out;
}

const Connector *MiniMux::connector_in(unsigned i) const {
    SANITY_ASSERT(i < size, "MiniMux: requested out of range input connector");
    return con_in[i];
}

void MiniMux::set(std::uint32_t i) {
    if (seton != (int)i) {
        seton = (int)i;
        update();
    }
}

