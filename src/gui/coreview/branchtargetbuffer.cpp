#include "branchtargetbuffer.h"
#include "coreview_colors.h"
#include "fontsize.h"

//////////////////////
#define WIDTH 40
#define HEIGHT 20
#define PENW 1
//////////////////////

using namespace coreview;

BranchTargetBuffer::BranchTargetBuffer() : QGraphicsObject(nullptr), con_in(new Connector(Connector::AX_X)), con_out(new Connector(Connector::AX_X)), name("BranchTargetBuffer", this) {
    QFont font;
    font.setPixelSize(FontSize::SIZE7);
    name.setFont(font);

    QRectF name_box = name.boundingRect();
    name.setText("BTB");
    name.setPos(name_box.width()/6, HEIGHT/2 - name_box.height()/2);

    setPos(x(), y());
}

BranchTargetBuffer::~BranchTargetBuffer() {
    delete con_in;
    delete con_out;
}

QRectF BranchTargetBuffer::boundingRect() const {
    return QRectF(-PENW / 2, -PENW / 2, WIDTH + PENW, HEIGHT + PENW);
}

void BranchTargetBuffer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option __attribute((unused)), QWidget *widget __attribute((unused))) {
    QPen pen = painter->pen();
    pen.setColor(BLOCK_OUTLINE_COLOR);
    painter->setPen(pen);

    painter->drawRect(0, 0, WIDTH, HEIGHT);
}

const Connector *BranchTargetBuffer::connector_in() const {
    return con_in;
}

const Connector *BranchTargetBuffer::connector_out() const {
    return con_out;
}

void BranchTargetBuffer::setPos(qreal x, qreal y) {
    QGraphicsObject::setPos(x, y);
    con_in->setPos(x, y + HEIGHT / 2);
    con_out->setPos(x + WIDTH / 2, y + HEIGHT);
}

void BranchTargetBuffer::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsObject::mouseDoubleClickEvent(event);

    emit open_btb();
}
