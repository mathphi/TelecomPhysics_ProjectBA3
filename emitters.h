#ifndef EMITTER_H
#define EMITTER_H

#include <QGraphicsItem>
#include <complex>

namespace EmitterType {
enum EmitterType {
    HalfWaveDipole  = 1,
};
}

class Emitter : public QGraphicsItem
{
public:
    Emitter (double frequency, double power, double efficiency);

    virtual EmitterType::EmitterType getEmitterType() = 0;
    virtual std::complex<double> getEffectiveHeight(double theta, double phi) = 0;
    virtual double getGain (double theta, double phi) = 0;

    double getPower();
    double getFrequency();
    double getEfficiency();

private:
    double m_frequency;
    double m_power;
    double m_efficiency;
};


class HalfWaveDipole : public Emitter
{
public:
    HalfWaveDipole(double frequency, double power, double efficiency);

    EmitterType::EmitterType getEmitterType() override;
    std::complex<double> getEffectiveHeight(double theta, double phi) override;
    double getGain(double theta, double phi) override;

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
};

// Operator overload to write objects from the Emitter class into a files
QDataStream &operator>>(QDataStream &in, Emitter *&e);
QDataStream &operator<<(QDataStream &out, Emitter *e);

#endif // EMITTER_H
