#include "predictor.h"
#include "coreview_colors.h"
#include "fontsize.h"

//////////////////////
#define WIDTH 40
#define HEIGHT 20
#define PENW 1
//////////////////////

using namespace coreview;

Predictor::Predictor() : QGraphicsObject(nullptr), con_pc_in(new Connector(Connector::AX_X)), con_sig_out(new Connector(Connector::AX_X)), name("Predictor", this) {
    QFont font;
    font.setPixelSize(FontSize::SIZE7);
    name.setFont(font);

    QRectF name_box = name.boundingRect();
    name.setPos(WIDTH/2 - name_box.width()/2, HEIGHT/2 - name_box.height()/2);

    setPos(x(), y());
}

Predictor::~Predictor() {
    delete con_pc_in;
    delete con_sig_out;
}

QRectF Predictor::boundingRect() const {
    return QRectF(-PENW / 2, -PENW / 2, WIDTH + PENW, HEIGHT + PENW);
}

void Predictor::paint(QPainter *painter, const QStyleOptionGraphicsItem *option __attribute((unused)), QWidget *widget __attribute((unused))) {
    QPen pen = painter->pen();
    pen.setColor(BLOCK_OUTLINE_COLOR);
    painter->setPen(pen);

    painter->drawRect(0, 0, WIDTH, HEIGHT);
}

const Connector *Predictor::connector_pc_in() const {
    return con_pc_in;
}

const Connector *Predictor::connector_sig_out() const {
    return con_sig_out;
}

void Predictor::setPos(qreal x, qreal y) {
    QGraphicsObject::setPos(x, y);
    con_pc_in->setPos(x + WIDTH / 2, y + HEIGHT);
    con_sig_out->setPos(x, y + HEIGHT / 2);
}

void Predictor::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsObject::mouseDoubleClickEvent(event);

    emit open_predictor();
}
