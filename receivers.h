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

    double receivedPower();
    double getBitRate();

    void generateTooltip();

private:
    QList<RayPath*> m_received_rays;
    double m_received_power;

    QMutex m_mutex;
};

// Operator overload to write objects from the Receiver class into a files
QDataStream &operator>>(QDataStream &in, Receiver *&r);
QDataStream &operator<<(QDataStream &out, Receiver *r);


class ReceiversArea : public QGraphicsRectItem, public SimulationItem
{
public:
    ReceiversArea();
    ~ReceiversArea();

    QList<Receiver*> getReceiversList();
    void setArea(QRectF area);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *s, QWidget *w) override;

private:
    void createReceivers(QRectF area);
    void deleteReceivers();

    QList<Receiver*> m_receivers_list;
};

#endif // RECEIVER_H
