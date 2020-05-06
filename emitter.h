#ifndef EMITTER_H
#define EMITTER_H

#include "simulationitem.h"
#include "constants.h"
#include "antennas.h"

#include <QGraphicsItem>
#include <complex>

class Emitter : public SimulationItem
{
public:
    Emitter(double frequency,
            double power,
            Antenna *antenna);

    Emitter(double frequency,
            double power,
            double efficiency,
            AntennaType::AntennaType antenna_type);

    ~Emitter();

    Emitter* clone();

    QPolygonF getPolyGain() const;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

    void setRotation(double angle);
    double getRotation();
    double getIncidentRayAngle(QLineF ray);

    void setAntenna(AntennaType::AntennaType type, double efficiency);
    void setAntenna(Antenna *a);
    void setPower(double power);
    void setFrequency(double freq);

    Antenna *getAntenna();
    double getPower() const;
    double getFrequency() const;
    double getEfficiency() const;

    double getResistance() const;
    vector<complex> getEffectiveHeight(double phi) const;
    double getGain(double phi) const;
    vector<double> getPolarization() const;

private:
    double m_rotation_angle;

    double m_frequency;
    double m_power;

    Antenna *m_antenna;
};


// Operator overload to write objects from the Emitter class into a files
QDataStream &operator>>(QDataStream &in, Emitter *&e);
QDataStream &operator<<(QDataStream &out, Emitter *e);

#endif // EMITTER_H