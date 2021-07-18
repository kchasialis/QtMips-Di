#include "cycle.h"
#include "coreview_colors.h"
#include "fontsize.h"

using namespace coreview;

//////////////////////
#define GAP 3
#define RADIUS GAP
#define LINE_OFFSET 1
#define PENW 1
//////////////////////

Cycle::Cycle() : QGraphicsObject(nullptr) {
    QFont font;
    qreal h = 0, w = 0;

    font.setPixelSize(FontSize::SIZE7);

    text = new QGraphicsSimpleTextItem(QString::number(INT_MAX), this);
    text->setFont(font);

    QRectF t_box = text->boundingRect();
    text->setPos(-t_box.width()/2, LINE_OFFSET);
    h += t_box.height() + LINE_OFFSET;
    if (w < t_box.width())
        w = t_box.width();

    box = QRectF(-w/2 - GAP, -GAP, w + (2*GAP), h + (2*GAP));
}

Cycle::~Cycle() {
    delete text;
}

QRectF Cycle::boundingRect() const {
    return QRectF(box.x() - PENW/2, box.y() - PENW/2, box.width() + PENW, box.height() + PENW);
}

void Cycle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option __attribute__((unused)), QWidget *widget __attribute__((unused))) {
    QPen pen = painter->pen();
    pen.setColor(BLOCK_OUTLINE_COLOR);
    painter->setPen(pen);
    painter->drawRoundedRect(box, RADIUS, RADIUS);
}

void Cycle::cycle_update(std::uint32_t cycle) {
    text->setText(QString::number(cycle));
}

