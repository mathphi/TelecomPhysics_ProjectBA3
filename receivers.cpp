#include "constants.h"
#include "receivers.h"
#include "simulationscene.h"
#include "simulationdata.h"

#include <QPainter>

// We want a receiver that is a square of 1 meter side
#define RECEIVER_SIZE (1.0 * simulationScene()->simulationScale())
#define RECEIVER_CIRCLE_SIZE 8 // Size of the circle at the center (in pixels)

Receiver::Receiver() : SimulationItem()
{
    // Over walls
    setZValue(2000);

    m_received_power = 0;
}

QRectF Receiver::boundingRect() const {
    return QRectF(-RECEIVER_SIZE/2 - 2, -RECEIVER_SIZE/2 - 2,
                  RECEIVER_SIZE + 4, RECEIVER_SIZE + 4);
}

QPainterPath Receiver::shape() const {
    QPainterPath path;
    path.addRect(-RECEIVER_SIZE/2 - 2, -RECEIVER_SIZE/2 - 2,
                 RECEIVER_SIZE + 4, RECEIVER_SIZE + 4);
    return path;
}

void Receiver::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    // Draw a dash-dot lined square with a cross on his center
    painter->setBrush(Qt::transparent);
    painter->setPen(QPen(QBrush(Qt::black), 1,Qt::DashDotLine));
    painter->drawRect(-RECEIVER_SIZE/2, -RECEIVER_SIZE/2, RECEIVER_SIZE, RECEIVER_SIZE);

    painter->drawLine(0, -RECEIVER_SIZE/2, 0, RECEIVER_SIZE/2);
    painter->drawLine(-RECEIVER_SIZE/2, 0, RECEIVER_SIZE/2, 0);

    // Draw a circle on the center of the drawn square
    painter->setPen(QPen(QBrush(Qt::black), 1));
    painter->setBrush(Qt::green);
    painter->drawEllipse(
                -RECEIVER_CIRCLE_SIZE/2,
                -RECEIVER_CIRCLE_SIZE/2,
                RECEIVER_CIRCLE_SIZE,
                RECEIVER_CIRCLE_SIZE);
}

void Receiver::reset() {
    // Delete all RayPaths to this receiver
    foreach (RayPath *rp, m_received_rays) {
        delete rp;
    }

    m_received_rays.clear();
    m_received_power = 0;

    // Disable the tooltip
    setToolTip(QString());
}

void Receiver::addRayPath(RayPath *rp) {
    // Don't add an invalid RayPath
    if (rp == nullptr)
        return;

    // Lock the mutex to ensure that only one thread write in the list at a time
    m_mutex.lock();

    // Append the new ray path to the list
    m_received_rays.append(rp);

    // Unlock the mutex to allow others threads to write
    m_mutex.unlock();
}

QList<RayPath*> Receiver::getRayPaths() {
    return m_received_rays;
}

void Receiver::setReceivedPower(double pwr) {
    m_received_power = pwr;

    // Set the tooltip of the receiver with the computed power
    setToolTip(QString("<b>Puissance&nbsp;:</b> %1&nbsp;dBm<br>"
                       "<b>Débit&nbsp;:</b> %2&nbsp;Mb/s")
               .arg(SimulationData::convertPowerTodBm(pwr), 0, 'f', 2)
               .arg(getBitRate(), 0, 'f', 2));
}

double Receiver::receivedPower() {
    return m_received_power;
}

double Receiver::getBitRate() {
    double bit_rate = 0;
    double dbm_power = SimulationData::convertPowerTodBm(m_received_power);

    // Under -82 dBm, the bitrate is 0 Mb/s
    if (dbm_power >= -82) {
        // Limit the power to -51 dBm (the bit rate cannot be greater)
        dbm_power = min(dbm_power, -51.0);

        // Linearisation between the two boundary values :
        //   -82 dBm        54 Mb/s
        //   -51 dBm        433 Mb/s
        bit_rate = (433.0 - 54.0) / (-51.0 + 82.0) * (dbm_power + 51.0) + 433.0;
    }

    return bit_rate;
}


//TODO: also write the received power ?
QDataStream &operator>>(QDataStream &in, Receiver *&r) {
    QPoint pos;
    in >> pos;

    r = new Receiver();
    r->setPos(pos);

    return in;
}

QDataStream &operator<<(QDataStream &out, Receiver *r) {
    out << r->pos().toPoint();

    return out;
}
