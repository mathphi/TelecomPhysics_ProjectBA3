#include "receivers.h"
#include "simulationscene.h"

#include <QPainter>

// We want a receiver that is a square of 1 meter side
#define RECEIVER_SIZE 1.0 * simulationScene()->simulationScale()
#define RECEIVER_CIRCLE_SIZE 8 // Size of the circle at the center (in pixels)

Receiver::Receiver() : SimulationItem()
{

}

QRectF Receiver::boundingRect() const {
    return QRectF(-RECEIVER_SIZE/2 - 2, -RECEIVER_SIZE/2 - 2,
                  RECEIVER_SIZE + 4, RECEIVER_SIZE + 4);
}

QPainterPath Receiver::shape() const {
    QPainterPath path;
    path.addRect(-RECEIVER_SIZE/2 - 2, -RECEIVER_SIZE/2 - 2,
                 RECEIVER_SIZE + 4, RECEIVER_SIZE + 4);
    return path;
}

void Receiver::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    // Draw a dash-dot lined square with a cross on his center
    painter->setBrush(Qt::transparent);
    painter->setPen(QPen(QBrush(Qt::black), 1,Qt::DashDotLine));
    painter->drawRect(-RECEIVER_SIZE/2, -RECEIVER_SIZE/2, RECEIVER_SIZE, RECEIVER_SIZE);

    painter->drawLine(0, -RECEIVER_SIZE/2, 0, RECEIVER_SIZE/2);
    painter->drawLine(-RECEIVER_SIZE/2, 0, RECEIVER_SIZE/2, 0);

    // Draw a circle on the center of the drawn square
    painter->setPen(QPen(QBrush(Qt::black), 1));
    painter->setBrush(Qt::green);
    painter->drawEllipse(
                -RECEIVER_CIRCLE_SIZE/2,
                -RECEIVER_CIRCLE_SIZE/2,
                RECEIVER_CIRCLE_SIZE,
                RECEIVER_CIRCLE_SIZE);
}


QDataStream &operator>>(QDataStream &in, Receiver *&r) {
    QPoint pos;
    in >> pos;

    r = new Receiver();
    r->setPos(pos);

    return in;
}

QDataStream &operator<<(QDataStream &out, Receiver *r) {
    out << r->pos().toPoint();

    return out;
}
