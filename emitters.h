#ifndef EMITTER_H
#define EMITTER_H

#include <QGraphicsItem>
#include <complex>

class Emitter : public QGraphicsItem
{
public:
    Emitter (double frequency, double power, double efficiency);

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

    std::complex<double> getEffectiveHeight(double theta, double phi) override;
    double getGain(double theta, double phi) override;

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;


};

#endif // EMITTER_H
