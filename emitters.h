#ifndef EMITTER_H
#define EMITTER_H

#include "simulationitem.h"

#include <QGraphicsItem>
#include <complex>

namespace EmitterType {
enum EmitterType {
    HalfWaveDipole  = 1,
};
}

class Emitter : public SimulationItem
{
public:
    Emitter (double frequency, double power, double efficiency, double resistance);

    virtual Emitter* clone() = 0;

    virtual EmitterType::EmitterType getEmitterType() = 0;
    virtual QString getEmitterLabel() = 0;

    virtual std::complex<double> getEffectiveHeight(double theta, double phi) = 0;
    virtual double getGain (double theta, double phi) = 0;

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

    double getPower();
    double getFrequency();
    double getEfficiency();
    double getResistance();

    static double convertPowerToWatts(double power_dbm);
    static double convertPowerTodBm(double power_watts);

private:
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

    EmitterType::EmitterType getEmitterType() override;
    QString getEmitterLabel() override;

    std::complex<double> getEffectiveHeight(double theta, double phi) override;
    double getGain(double theta, double phi) override;
};

// Operator overload to write objects from the Emitter class into a files
QDataStream &operator>>(QDataStream &in, Emitter *&e);
QDataStream &operator<<(QDataStream &out, Emitter *e);

#endif // EMITTER_H
