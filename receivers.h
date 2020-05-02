#ifndef RECEIVER_H
#define RECEIVER_H

#include <QGraphicsItem>
#include <QMutex>

#include "simulationitem.h"
#include "raypath.h"

class Receiver : public SimulationItem
{
public:
    Receiver();

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

    void reset();

    void addRayPath(RayPath *rp);
    QList<RayPath*> getRayPaths();

    void setReceivedPower(double pwr);
    double receivedPower();

    double getBitRate();

private:
    QList<RayPath*> m_received_rays;
    double m_received_power;

    QMutex m_mutex;
};

// Operator overload to write objects from the Receiver class into a files
QDataStream &operator>>(QDataStream &in, Receiver *&r);
QDataStream &operator<<(QDataStream &out, Receiver *r);

#endif // RECEIVER_H
