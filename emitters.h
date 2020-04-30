#ifndef EMITTER_H
#define EMITTER_H

#include "simulationitem.h"
#include "constants.h"

#include <QGraphicsItem>
#include <complex>

namespace EmitterType {
enum EmitterType {
    HalfWaveDipoleVert  = 1,
};
}

class Emitter : public SimulationItem
{
public:
    Emitter (double frequency, double power, double efficiency, double resistance);

    virtual Emitter* clone() = 0;

    virtual EmitterType::EmitterType getEmitterType() const = 0;
    virtual QString getEmitterLabel() const = 0;

    virtual complex<double> getEffectiveHeight(double theta, double phi) const = 0;
    virtual double getGain(double theta, double phi) const = 0;

    complex<double> getEffectiveHeight(double phi) const;
    double getGain(double phi) const;

    QPolygonF getPolyGain() const;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

    void setRotation(double angle);
    double getRotation();

    double getIncidentRayAngle(QLineF ray);

    double getPower() const;
    double getFrequency() const;
    double getEfficiency() const;
    double getResistance() const;

    static double convertPowerToWatts(double power_dbm);
    static double convertPowerTodBm(double power_watts);

private:
    double m_rotation_angle;

    double m_frequency;
    double m_power;
    double m_efficiency;
    double m_resistance;
};


class HalfWaveDipole : public Emitter
{
public:
    HalfWaveDipole(double frequency, double power, double efficiency, double resistance);

    Emitter* clone() override;

    EmitterType::EmitterType getEmitterType() const override;
    QString getEmitterLabel() const override;

    complex<double> getEffectiveHeight(double theta, double phi) const override;
    double getGain(double theta, double phi) const override;
};

// Operator overload to write objects from the Emitter class into a files
QDataStream &operator>>(QDataStream &in, Emitter *&e);
QDataStream &operator<<(QDataStream &out, Emitter *e);

#endif // EMITTER_H
