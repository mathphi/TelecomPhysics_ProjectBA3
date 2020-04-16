#include "emitters.h"
#include "constants.h"

#include <QPainter>



#define EMITTER_WIDTH 8
#define EMITTER_HEIGHT 18
#define EMITTER_TEXT_WIDTH 24
#define EMITTER_TEXT_HEIGHT 20

#define HALF_WAVE_LABEL "λ/2"

// Defines the rectangle where to place the emitter's label
const QRectF TEXT_RECT(
        -EMITTER_TEXT_WIDTH/2,
        -EMITTER_HEIGHT - EMITTER_TEXT_HEIGHT,
        EMITTER_TEXT_WIDTH,
        EMITTER_TEXT_HEIGHT);

Emitter::Emitter (double frequency, double power, double efficiency, double resistance) : SimulationItem()
{
    m_frequency  = frequency;
    m_power      = power;
    m_efficiency = efficiency;
    m_resistance = resistance;

    // Setup the tooltip with all emitter's info
    QString tip("<b>Fréquence:</b> %1 GHz<br/>"
                "<b>Puissance:</b> %2 dBm<br/>"
                "<b>Résistance:</b> %3 Ω<br/>"
                "<b>Rendement:</b> %4 %");

    tip = tip.arg(frequency, 0, 'f', 2)
            .arg(convertPowerTodBm(power), 0, 'f', 2)
            .arg(resistance, 0, 'f', 2)
            .arg(efficiency, 0, 'f', 1);

    setToolTip(tip);
}

/**
 * @brief Emitter::convertPowerToWatts
 * @param power_dbm
 * @return
 *
 * This function returns the converted power in watts from a dBm value
 * (formula from the specifications document of the project)
 */
double Emitter::convertPowerToWatts(double power_dbm) {
    // Compute the power in Watts from the dBm
    return pow(10.0, power_dbm/10.0) / 1000.0;
}

/**
 * @brief Emitter::convertPowerToWatts
 * @param power_dbm
 * @return
 *
 * This function returns the converted power in dBm from a Watts value
 */
double Emitter::convertPowerTodBm(double power_watts) {
    // Compute the power in dBm from the Watts
    return 10 * log10(power_watts / 0.001);
}


QRectF Emitter::boundingRect() const {
    QRectF emitter_rect(-EMITTER_WIDTH/2 - 2, -EMITTER_HEIGHT - 2,
                        EMITTER_WIDTH + 4, EMITTER_HEIGHT + 4);

    // Bounding rect contains the emitter and his text
    return emitter_rect.united(TEXT_RECT);
}

QPainterPath Emitter::shape() const {
    QPainterPath path;
    path.addRect(-EMITTER_WIDTH/2 - 2, -EMITTER_HEIGHT - 2,
                 EMITTER_WIDTH + 4, EMITTER_HEIGHT + 4);
    path.addRect(TEXT_RECT);
    return path;
}

void Emitter::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    // Draw a circle over a line, with the origin at the end of the line
    painter->setPen(QPen(QBrush(Qt::black), 1));
    painter->setBrush(Qt::red);
    painter->drawEllipse(-EMITTER_WIDTH/2, -EMITTER_HEIGHT, EMITTER_WIDTH, EMITTER_WIDTH);
    painter->drawLine(0, 0, 0, -(EMITTER_HEIGHT - EMITTER_WIDTH));

    // Draw the label of the emitter
    painter->drawText(TEXT_RECT, Qt::AlignHCenter | Qt::AlignTop, getEmitterLabel());
}

double Emitter::getEfficiency() {
    return m_efficiency;
}

double Emitter::getFrequency() {
    return m_frequency;
}

double Emitter::getPower() {
    return m_power;
}

double Emitter::getResistance() {
    return  m_resistance;
}



HalfWaveDipole::HalfWaveDipole(double frequency, double power, double efficiency, double resistance)
    : Emitter(frequency, power, efficiency, resistance)
{

}

/**
 * @brief clone
 * @return
 *
 * This function returns a new HalfWaveDipole with the same properties
 */
Emitter* HalfWaveDipole::clone() {
    return new HalfWaveDipole(getFrequency(), getPower(), getEfficiency(), getResistance());
}

EmitterType::EmitterType HalfWaveDipole::getEmitterType() {
    return EmitterType::HalfWaveDipole;
}

QString HalfWaveDipole::getEmitterLabel() {
    return HALF_WAVE_LABEL;
}

double HalfWaveDipole::getGain(double theta, double phi) {
    Q_UNUSED(phi);
    double eta = getEfficiency();
    return eta*16.0/(3*M_PI)*pow(sin(theta),3);
}

std::complex <double> HalfWaveDipole::getEffectiveHeight(double theta, double phi) {
    Q_UNUSED(phi);
    double lambda = LIGHT_SPEED/getFrequency();
    return -lambda/M_PI * cos(M_PI/2 * cos(theta))/pow(sin(theta),2);
}


QDataStream &operator>>(QDataStream &in, Emitter *&e) {
    int type;
    double power;
    double frequency;
    double efficiency;
    double resistivity;
    QPoint pos;

    in >> type;
    in >> power;
    in >> frequency;
    in >> efficiency;
    in >> resistivity;
    in >> pos;

    switch (type) {
    case EmitterType::HalfWaveDipole:
        e = new HalfWaveDipole(frequency, power, efficiency, resistivity);
        break;
    }

    e->setPos(pos);

    return in;
}

QDataStream &operator<<(QDataStream &out, Emitter *e) {
    out << e->getEmitterType();
    out << e->getPower();
    out << e->getFrequency();
    out << e->getEfficiency();
    out << e->getResistance();
    out << e->pos().toPoint();

    return out;
}
