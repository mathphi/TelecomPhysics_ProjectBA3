#ifndef RECEIVER_H
#define RECEIVER_H

#include <QGraphicsItem>
#include <QMutex>

#include "simulationitem.h"
#include "raypath.h"
#include "antennas.h"

class Receiver : public SimulationItem
{
public:
    Receiver(Antenna *antenna);
    Receiver(AntennaType::AntennaType antenna_type = AntennaType::HalfWaveDipoleHoriz,
             double efficiency = 1.0);

    ~Receiver();

    Antenna *getAntenna();

    void setRotation(double angle);
    double getRotation();
    double getIncidentRayAngle(QLineF ray);

    double getEfficiency() const;
    double getResistance() const;
    vector<complex> getEffectiveHeight(double phi, double frequency) const;
    double getGain(double phi) const;

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
    double m_rotation_angle;
    Antenna *m_antenna;

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
