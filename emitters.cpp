#include "emitters.h"
#include "constants.h"

#include <QPainter>



#define EMITTER_WIDTH 8
#define EMITTER_HEIGHT 18
#define EMITTER_TEXT_WIDTH 24
#define EMITTER_TEXT_HEIGHT 20

#define EMITTER_POLYGAIN_SIZE 7.0

#define HALF_WAVE_LABEL "λ/2"

// Defines the rectangle where to place the emitter's label
const QRectF TEXT_RECT(
        -EMITTER_TEXT_WIDTH/2,
        -EMITTER_HEIGHT - EMITTER_TEXT_HEIGHT,
        EMITTER_TEXT_WIDTH,
        EMITTER_TEXT_HEIGHT);

Emitter::Emitter (double frequency, double power, double efficiency, double resistance) : SimulationItem()
{
    // The default angle for the emitter is PI/2 (incidence to top)
    m_rotation_angle = M_PI_2;

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
            .arg(efficiency * 100.0, 0, 'f', 1);

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

/**
 * @brief Emitter::getEffectiveHeight
 * @param phi
 * @return
 *
 * Returns the same as getEffectiveHeight(theta, phi), but with the default angle
 * theta to π/2, since the 2D simulation is in the plane θ = π/2
 */
complex<double> Emitter::getEffectiveHeight(double phi) const {
    // The 2D simulation is in the plane θ = π/2
    return getEffectiveHeight(M_PI_2, phi);
}

/**
 * @brief Emitter::getGain
 * @param phi
 * @return
 *
 * Returns the same as getGain(theta, phi), but with the default angle
 * theta to π/2, since the 2D simulation is in the plane θ = π/2
 */
double Emitter::getGain(double phi) const {
    // The 2D simulation is in the plane θ = π/2
    return getGain(M_PI_2, phi);
}


/**
 * @brief Emitter::getPolyGain
 * @return
 *
 * This function returns a polygon that represent the gain of the emitter around the phi angle
 */
QPolygonF Emitter::getPolyGain() const {
    QPolygonF poly_gain;
    QPointF pt;

    for (double phi = -M_PI ; phi < M_PI + 0.2 ; phi += 0.2) {
        pt = QPointF(cos(phi), sin(phi));
        poly_gain.append(pt * getGain(phi) * EMITTER_POLYGAIN_SIZE);
    }

    return poly_gain;
}

QRectF Emitter::boundingRect() const {
    QRectF emitter_rect(-EMITTER_WIDTH/2 - 2, -EMITTER_HEIGHT - 2,
                        EMITTER_WIDTH + 4, EMITTER_HEIGHT + 4);

    QRectF gain_rect = getPolyGain().boundingRect();

    // Bounding rect contains the emitter and his text
    return emitter_rect.united(gain_rect).united(TEXT_RECT);
}

QPainterPath Emitter::shape() const {
    QPainterPath path;
    path.addRect(-EMITTER_WIDTH/2 - 2, -EMITTER_HEIGHT - 2,
                 EMITTER_WIDTH + 4, EMITTER_HEIGHT + 4);
    path.addRect(TEXT_RECT);
    path.addPolygon(getPolyGain());
    return path;
}

void Emitter::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    // Draw the shape of the gain in blue
    //painter->setPen(QPen(QBrush(Qt::blue), 1));
    //painter->setBrush(Qt::transparent);
    //painter->drawPolygon(getPolyGain());

    // Draw a circle over a line, with the origin at the end of the line
    painter->setPen(QPen(QBrush(Qt::black), 1));
    painter->setBrush(Qt::red);
    painter->drawEllipse(-EMITTER_WIDTH/2, -EMITTER_HEIGHT, EMITTER_WIDTH, EMITTER_WIDTH);
    painter->drawLine(0, 0, 0, -(EMITTER_HEIGHT - EMITTER_WIDTH));

    // Draw the label of the emitter
    painter->drawText(TEXT_RECT, Qt::AlignHCenter | Qt::AlignTop, getEmitterLabel());
}

/**
 * @brief Emitter::setRotation
 * @param angle
 *
 * Sets the rotation angle of the emitter (in radians)
 */
void Emitter::setRotation(double angle) {
    m_rotation_angle = angle;
}

/**
 * @brief Emitter::getRotation
 * @return
 *
 * Get the rotation angle of the antenna (in radians)
 */
double Emitter::getRotation() {
    return m_rotation_angle;
}

/**
 * @brief Emitter::getIncidentRayAngle
 * @param ray
 * @return
 *
 * Returns the incidence angle of the ray to the emitter (in radians)
 */
double Emitter::getIncidentRayAngle(QLineF ray) {
    double ray_angle = ray.angle() / 180.0 * M_PI;
    return ray_angle - m_rotation_angle;
}

double Emitter::getEfficiency() const {
    return m_efficiency;
}

double Emitter::getFrequency() const {
    return m_frequency;
}

double Emitter::getPower() const {
    return m_power;
}

double Emitter::getResistance() const {
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

EmitterType::EmitterType HalfWaveDipole::getEmitterType() const {
    return EmitterType::HalfWaveDipoleVert;
}

QString HalfWaveDipole::getEmitterLabel() const {
    return HALF_WAVE_LABEL;
}

double HalfWaveDipole::getGain(double theta, double phi) const {
    Q_UNUSED(phi);
    double eta = getEfficiency();
    return fabs(eta*16.0/(3*M_PI)*pow(sin(theta),3));
}

complex<double> HalfWaveDipole::getEffectiveHeight(double theta, double phi) const {
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
    double rotation;
    QPoint pos;

    in >> type;
    in >> power;
    in >> frequency;
    in >> efficiency;
    in >> resistivity;
    in >> rotation;
    in >> pos;

    switch (type) {
    case EmitterType::HalfWaveDipoleVert:
        e = new HalfWaveDipole(frequency, power, efficiency, resistivity);
        break;
    }

    e->setRotation(rotation);
    e->setPos(pos);

    return in;
}

QDataStream &operator<<(QDataStream &out, Emitter *e) {
    out << e->getEmitterType();
    out << e->getPower();
    out << e->getFrequency();
    out << e->getEfficiency();
    out << e->getResistance();
    out << e->getRotation();
    out << e->pos().toPoint();

    return out;
}
