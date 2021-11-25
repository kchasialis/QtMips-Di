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

    const Connector *connector_sig_in() const;
    const Connector *connector_sig_out() const;
    const Connector *connector_in(size_t idx) const;
    const Connector *connector_out() const;
    void setPos(qreal x, qreal y);

signals:
    void open_predictor();

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    Connector *con_sig_in;
    Connector *con_sig_out;
    Connector *con_in[2];
    Connector *con_out;
    QGraphicsSimpleTextItem name;
};

}

#endif
