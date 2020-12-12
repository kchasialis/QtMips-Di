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

#include "logicblock.h"
#include "coreview_colors.h"
#include "fontsize.h"
#include <cmath>

using namespace coreview;

//////////////////////
#define GAP 3
#define RADIUS GAP
#define LINE_OFFSET 1
#define PENW 1
//////////////////////

LogicBlock::LogicBlock(QString name) : LogicBlock(QVector<QString>({name})) { }

LogicBlock::LogicBlock(QVector<QString> name) : QGraphicsObject(nullptr) {
    QFont font;
    font.setPixelSize(FontSize::SIZE7);

    qreal h = 0, w = 0;
    for (int i = 0; i < name.size(); i++) {
        QGraphicsSimpleTextItem *t = new QGraphicsSimpleTextItem(name[i], this);
        t->setFont(font);
        text.append(t);
        QRectF t_box = t->boundingRect();
        t->setPos(-t_box.width()/2, h + LINE_OFFSET);
        h += t_box.height() + LINE_OFFSET;
        if (w < t_box.width())
            w = t_box.width();
    }

    box = QRectF(-w/2 - GAP, -GAP, w + (2*GAP), h + (2*GAP));
}

LogicBlock::~LogicBlock() {
    for (int i = 0; i < connectors.size(); i++)
        delete connectors[i].con;
    for (int i = 0; i < text.size(); i++)
        delete text[i];
}

QRectF LogicBlock::boundingRect() const {
    return QRectF(box.x() - PENW/2, box.y() - PENW/2, box.width() + PENW, box.height() + PENW);
}

void LogicBlock::paint(QPainter *painter, const QStyleOptionGraphicsItem *option __attribute__((unused)), QWidget *widget __attribute__((unused))) {
    QPen pen = painter->pen();
    pen.setColor(BLOCK_OUTLINE_COLOR);
    painter->setPen(pen);

    painter->drawRoundedRect(box, RADIUS, RADIUS);
}

void LogicBlock::setPos(qreal x, qreal y) {
    QGraphicsObject::setPos(x, y);
    for (int i = 0; i < connectors.size(); i++) {
        struct Con &c = connectors[i];
        c.con->setPos(x + c.p.x(), y + c.p.y());
    }
}

void LogicBlock::setSize(qreal width, qreal height) {
    box.setX(-width/2 - GAP);
    box.setWidth(width + (2*GAP));
    box.setHeight(height + (2*GAP));
    for (int i = 0; i < connectors.size(); i++) { // Update point for all connectors
        struct Con &c = connectors[i];
        c.p = con_pos(c.x, c.y);
    }
    setPos(x(), y());
}

static qreal sign(qreal v) {
    // This intentionally doesn't return 0 on v == 0
    return (0 <= v) - (0 > v);
}

const Connector *LogicBlock::new_connector(qreal x, qreal y) {
    // stick to closest edge
    if (fabs(x) > fabs(y))
        x = sign(x);
    else
        y = sign(y);

    // Note: we are using here that 0 and M_PI is same angle but different orientation (but we ignore orientation for now)
    Connector *c = new Connector(fabs(x) > fabs(y) ? Connector::AX_X : Connector::AX_Y);
    connectors.append({
        .con = c,
        .x = x,
        .y = y,
        .p = con_pos(x, y)
    });
    setPos(this->x(), this->y()); // Update connector position
    return c;
}

QPointF LogicBlock::con_pos(qreal x, qreal y) {
    qreal px, py;
    px = (box.right() - GAP) * x;
    py = (box.bottom()/2 - GAP) * (y + 1) + GAP;
    if (fabs(x) == 1)
        px += GAP * sign(x);
    if (fabs(y) == 1)
        py += GAP * sign(y);
    return QPointF(px, py);
}

void LogicBlock::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsObject::mouseDoubleClickEvent(event);
    emit open_block();
}
