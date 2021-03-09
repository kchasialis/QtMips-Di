#ifndef COREVIEW_PREDICTOR_H
#define COREVIEW_PREDICTOR_H

#include <QGraphicsObject>
#include <QPainter>
#include <QGraphicsSimpleTextItem>
#include "connection.h"

namespace coreview {

class Predictor : public QGraphicsObject {
    Q_OBJECT
public:
    Predictor();
    ~Predictor();

    QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setPos(qreal x, qreal y);
private:
    Connector *pc_connector;
    QGraphicsSimpleTextItem name;
};

}

#endif
