#include "receivers.h"

#include <QPainter>

#define RECEIVER_SIZE 50

Receiver::Receiver()
{

}

QRectF Receiver::boundingRect() const
{
    return QRectF(-RECEIVER_SIZE/2 - 2  , -RECEIVER_SIZE/2 - 2 ,
                  RECEIVER_SIZE + 4 , RECEIVER_SIZE + 4 );
}

QPainterPath Receiver::shape() const
{
    QPainterPath path;
    path.addRect(-RECEIVER_SIZE/2 - 2  , -RECEIVER_SIZE/2 - 2 ,
                 RECEIVER_SIZE + 4 , RECEIVER_SIZE + 4 );
    return path;
}

void Receiver::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *){
    // Draw a circle over a line, with the origin at the end of the line

    painter->setBrush(Qt::transparent);
    painter->setPen(QPen(QBrush(Qt::black), 1,Qt::DashDotLine));
    painter->drawRect(-RECEIVER_SIZE/2,-RECEIVER_SIZE/2,RECEIVER_SIZE,RECEIVER_SIZE);

    painter->drawLine(0, -RECEIVER_SIZE/2 , 0, RECEIVER_SIZE/2);
    painter->drawLine( -RECEIVER_SIZE/2 , 0, RECEIVER_SIZE/2,0);

    painter->setPen(QPen(QBrush(Qt::black), 1));
    painter->setBrush(Qt::green);
    painter->drawEllipse(-8/2, -8/2, 8, 8);

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
