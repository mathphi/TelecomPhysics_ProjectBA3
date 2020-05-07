#include "constants.h"
#include "receiver.h"
#include "simulationscene.h"
#include "simulationdata.h"

#include <QPainter>

// We want a receiver that is a square of 1 meter side
#define RECEIVER_SIZE (1.0 * simulationScene()->simulationScale())
#define RECEIVER_CIRCLE_SIZE 8 // Size of the circle at the center (in pixels)

Receiver::Receiver(Antenna *antenna) : SimulationItem()
{
    // The default angle for the emitter is PI/2 (incidence to top)
    m_rotation_angle = M_PI_2;

    // Create the associated antenna of right type
    m_antenna = antenna;

    // Over walls
    setZValue(2000);

    // Initially resetted
    reset();
}

Receiver::Receiver(AntennaType::AntennaType antenna_type, double efficiency)
    : Receiver(Antenna::createAntenna(antenna_type, efficiency))
{

}

Receiver::~Receiver()
{
    delete m_antenna;
}

Antenna *Receiver::getAntenna() {
    return m_antenna;
}

void Receiver::setAntenna(AntennaType::AntennaType type, double efficiency) {
    setAntenna(Antenna::createAntenna(type, efficiency));
}

void Receiver::setAntenna(Antenna *a) {
    if (m_antenna != nullptr) {
        delete m_antenna;
    }

    m_antenna = a;

    // Update the graphics
    prepareGeometryChange();
    update();
}

/**
 * @brief Receiver::setRotation
 * @param angle
 *
 * Sets the rotation angle of the emitter (in radians)
 */
void Receiver::setRotation(double angle) {
    m_rotation_angle = angle;
}

/**
 * @brief Receiver::getRotation
 * @return
 *
 * Get the rotation angle of the antenna (in radians)
 */
double Receiver::getRotation() {
    return m_rotation_angle;
}

/**
 * @brief Receiver::getIncidentRayAngle
 * @param ray
 * @return
 *
 * Returns the incidence angle of the ray to the emitter (in radians)
 * This function assumes the ray comes into the emitter.
 */
double Receiver::getIncidentRayAngle(QLineF ray) {
    double ray_angle = ray.angle() / 180.0 * M_PI - M_PI;
    return ray_angle - getRotation();
}

double Receiver::getEfficiency() const {
    return m_antenna->getEfficiency();
}

double Receiver::getResistance() const {
    return m_antenna->getResistance();
}

vector<complex> Receiver::getEffectiveHeight(double phi, double frequency) const {
    return m_antenna->getEffectiveHeight(M_PI_2, phi, frequency);
}

double Receiver::getGain(double phi) const {
    return m_antenna->getGain(M_PI_2, phi);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// --------------------------------- RAYS RECEIVING FUNCTIONS ------------------------------------//
////////////////////////////////////////////////////////////////////////////////////////////////////

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

    // Add the power of this ray to the received power
    m_received_power += rp->getPower();

    // Unlock the mutex to allow others threads to write
    m_mutex.unlock();
}

QList<RayPath*> Receiver::getRayPaths() {
    return m_received_rays;
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


////////////////////////////////////////////////////////////////////////////////////////////////////
// ------------------------------------ GRAPHICS FUNCTIONS ---------------------------------------//
////////////////////////////////////////////////////////////////////////////////////////////////////

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

void Receiver::generateTooltip() {
    // Set the tooltip of the receiver with
    //  - the number of incident rays
    //  - the received power
    //  - the bitrate
    setToolTip(QString("<b><u>Récepteur</u></b><br/>"
                       "<b><i>%1</i></b><br/>"
                       "<b>Rayons incidents&nbsp;:</b> %2<br>"
                       "<b>Puissance&nbsp;:</b> %3&nbsp;dBm<br>"
                       "<b>Débit&nbsp;:</b> %4&nbsp;Mb/s")
               .arg(m_antenna->getAntennaName())
               .arg(getRayPaths().size())
               .arg(SimulationData::convertPowerTodBm(receivedPower()), 0, 'f', 2)
               .arg(getBitRate(), 0, 'f', 2));
}


QDataStream &operator>>(QDataStream &in, Receiver *&r) {
    Antenna *ant;
    QPoint pos;

    in >> ant;
    in >> pos;

    r = new Receiver(ant);
    r->setPos(pos);

    return in;
}

QDataStream &operator<<(QDataStream &out, Receiver *r) {
    out << r->getAntenna();
    out << r->pos().toPoint();

    return out;
}



ReceiversArea::ReceiversArea() : QGraphicsRectItem(), SimulationItem()
{
    QGraphicsRectItem::setZValue(-1);
}

ReceiversArea::~ReceiversArea() {
    deleteReceivers();
}

QList<Receiver*> ReceiversArea::getReceiversList() {
    return m_receivers_list;
}

void ReceiversArea::setArea(QRectF area) {
    // Compute the area as a rect of size multiple of 1m²
    qreal sim_scale = simulationScene()->simulationScale();

    // Compute the 1m² fitted rect
    QSizeF fit_size(ceil(area.width() / sim_scale) * sim_scale,
                    ceil(area.height() / sim_scale) * sim_scale);

    // Center the content in the area
    QSizeF diff_sz = fit_size - area.size();
    QRectF fit_area = area.adjusted(-diff_sz.width()/2, -diff_sz.height()/2,
                                     diff_sz.width()/2,  diff_sz.height()/2);

    // Draw the area rectangle
    setPen(QPen(Qt::darkGray, 1, Qt::DashDotDotLine));
    setBrush(QBrush(qRgba(225, 225, 255, 255), Qt::DiagCrossPattern));
    QGraphicsRectItem::setRect(fit_area);

    // Delete and recreate the receivers list
    deleteReceivers();
    createReceivers(fit_area);
}

void ReceiversArea::createReceivers(QRectF area) {
    if (!simulationScene())
        return;

    // Get the count of receivers in each dimension
    QSize num_rcv = (area.size() / simulationScene()->simulationScale()).toSize();

    // Get the initial position of the receivers
    QPointF init_pos = area.topLeft() + QPointF(RECEIVER_SIZE/2, RECEIVER_SIZE/2);

    // Add a receiver to each m² on the area
    for (int x = 0 ; x < num_rcv.width() ; x++) {
        for (int y = 0 ; y < num_rcv.height() ; y++) {
            QPointF delta_pos(x * RECEIVER_SIZE, y * RECEIVER_SIZE);
            QPointF rcv_pos = init_pos + delta_pos;

            Receiver *rcv = new Receiver();
            simulationScene()->addItem(rcv);
            rcv->setPos(rcv_pos);

            m_receivers_list.append(rcv);
        }
    }
}

void ReceiversArea::deleteReceivers() {
    foreach(Receiver *r, m_receivers_list) {
        delete r;
    }

    m_receivers_list.clear();
}


QRectF ReceiversArea::boundingRect() const {
    return QGraphicsRectItem::boundingRect();
}

QPainterPath ReceiversArea::shape() const {
    return QGraphicsRectItem::shape();
}

void ReceiversArea::paint(QPainter *p, const QStyleOptionGraphicsItem *s, QWidget *w) {
    QGraphicsRectItem::paint(p, s, w);
}
