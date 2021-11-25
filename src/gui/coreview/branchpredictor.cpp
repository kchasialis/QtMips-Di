#include "branchpredictor.h"
#include "coreview_colors.h"
#include "fontsize.h"
#include "qtmipsexception.h"

//////////////////////
#define WIDTH 45
#define HEIGHT 25
#define PENW 1
#define GAP 5
//////////////////////

using namespace coreview;

Predictor::Predictor() : QGraphicsObject(nullptr), con_sig_in(new Connector()), con_sig_out(new Connector()),
                         con_in{new Connector(), new Connector()}, con_out(new Connector()),
                         name("Predictor", this) {
    QFont font;
    font.setPixelSize(FontSize::SIZE7);
    name.setFont(font);

    QRectF name_box = name.boundingRect();
    name.setPos(WIDTH/2 - name_box.width()/2, HEIGHT/2 - name_box.height()/2);

    setPos(x(), y());
}

Predictor::~Predictor() {
    delete con_sig_in;
    delete con_sig_out;
    delete con_in[0];
    delete con_in[1];
    delete con_out;
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

const Connector *Predictor::connector_sig_in() const {
    return con_sig_in;
}

const Connector *Predictor::connector_sig_out() const {
    return con_sig_out;
}

const Connector *Predictor::connector_in(size_t idx) const {
    SANITY_ASSERT(idx < 2, "Invalid in-connector index for predictor");
    return con_in[idx];
}

const Connector *Predictor::connector_out() const {
    return con_out;
}

void Predictor::setPos(qreal x, qreal y) {
    QGraphicsObject::setPos(x, y);
    con_sig_in->setPos(x + ((double) WIDTH / 2.0), y + HEIGHT);
    con_sig_out->setPos(x + WIDTH, y);
    con_in[0]->setPos(x, y + HEIGHT / 2 - GAP);
    con_in[1]->setPos(x, y + (HEIGHT / 2.0) + GAP);
    con_out->setPos(x + WIDTH, y + (double) HEIGHT / 2);
}

void Predictor::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsObject::mouseDoubleClickEvent(event);

    emit open_predictor();
}
