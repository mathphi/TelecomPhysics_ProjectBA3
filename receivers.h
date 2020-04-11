#ifndef RECEIVER_H
#define RECEIVER_H

#include <QGraphicsItem>

#include "simulationitem.h"

class Receiver : public SimulationItem
{
public:
    Receiver();

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
};

// Operator overload to write objects from the Receiver class into a files
QDataStream &operator>>(QDataStream &in, Receiver *&r);
QDataStream &operator<<(QDataStream &out, Receiver *r);

#endif // RECEIVER_H
