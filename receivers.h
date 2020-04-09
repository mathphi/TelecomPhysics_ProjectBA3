#ifndef RECEIVER_H
#define RECEIVER_H

#include <QGraphicsItem>

class Receiver : public QGraphicsItem
{
public:
    Receiver();

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
};

#endif // RECEIVER_H
