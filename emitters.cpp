#include "emitters.h"
#include <QPainter>

#define C 299792458 // [m/s]

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
    double lambda = C/getFrequency();
    return -lambda/M_PI * cos(M_PI/2 * cos(theta))/pow(sin(theta),2);
}
/*void HalfWaveDipole::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->drawEllipse(-4,14,8,8);
    painter->drawLine(0,0,0,10);
}*/
