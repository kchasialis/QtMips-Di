#include "predictor.h"
#include "coreview_colors.h"
#include "fontsize.h"

//////////////////////
#define WIDTH 60
#define HEIGHT 40
#define PENW 1
//////////////////////

using namespace coreview;

Predictor::Predictor() : QGraphicsObject(nullptr), pc_connector(new Connector(Connector::AX_X)), name("Predictor", this) {
    QFont font;
    font.setPixelSize(FontSize::SIZE7);
    name.setFont(font);

    QRectF name_box = name.boundingRect();
    name.setPos(WIDTH/2 - name_box.width()/2, HEIGHT/2 - name_box.height()/2);

    setPos(x(), y());
}

Predictor::~Predictor() {
    delete pc_connector;
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

void Predictor::setPos(qreal x, qreal y) {
    QGraphicsObject::setPos(x, y);
    pc_connector->setPos(x, y + 10);
}
