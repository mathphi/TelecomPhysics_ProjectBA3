#include "emitters.h"
#include <QPainter>

#define LIGHT_SPEED 299792458.0 // [m/s]

#define EMITTER_WIDTH 8
#define EMITTER_HEIGHT 18

Emitter::Emitter (double frequency, double power, double efficiency):QGraphicsItem(nullptr)
{
    m_frequency = frequency;
    m_power = power;
    m_efficiency = efficiency;
}


double Emitter::getEfficiency(){
    return m_efficiency;
}

double Emitter::getFrequency(){
    return m_frequency;
}

double Emitter::getPower(){
    return m_power;
}



HalfWaveDipole::HalfWaveDipole(double frequency, double power, double efficiency):Emitter(frequency, power, efficiency)
{

}

double HalfWaveDipole::getGain(double theta, double phi){
    Q_UNUSED(phi);
    double eta = getEfficiency();
    return eta*16.0/(3*M_PI)*pow(sin(theta),3);
}

std::complex <double> HalfWaveDipole::getEffectiveHeight(double theta, double phi){
    Q_UNUSED(phi);
    double lambda = LIGHT_SPEED/getFrequency();
    return -lambda/M_PI * cos(M_PI/2 * cos(theta))/pow(sin(theta),2);
}


QRectF HalfWaveDipole::boundingRect() const
{
    return QRectF(-EMITTER_WIDTH/2 - 2, -EMITTER_HEIGHT - 2,
                  EMITTER_WIDTH + 4, EMITTER_HEIGHT + 4);
}

QPainterPath HalfWaveDipole::shape() const
{
    QPainterPath path;
    path.addRect(-EMITTER_WIDTH/2 - 2, -EMITTER_HEIGHT - 2,
                 EMITTER_WIDTH + 4, EMITTER_HEIGHT + 4);
    return path;
}

void HalfWaveDipole::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *){
    // Draw a circle over a line, with the origin at the end of the line
    //painter->setPen(QPen(QBrush(Qt::black), 1));
    painter->setBrush(Qt::red);
    painter->drawEllipse(-EMITTER_WIDTH/2, -EMITTER_HEIGHT,
                         EMITTER_WIDTH, EMITTER_WIDTH);
    painter->drawLine(0, 0, 0, -(EMITTER_HEIGHT - EMITTER_WIDTH));
}
