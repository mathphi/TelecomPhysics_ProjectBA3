#include "walls.h"

#include <QPen>
#include <QPainter>

// Wall's relative permittivity
#define BRICK_R_PERMITTIVITY      4.6
#define CONCRETE_R_PERMITTIVITY   5
#define PARTITION_R_PERMITTIVITY  2.25

// Wall's conductivity
#define BRICK_CONDUCTIVITY      0.02
#define CONCRETE_CONDUCTIVITY   0.014
#define PARTITION_CONDUCTIVITY  0.04

// Wall's default thickness (cm)
#define BRICK_THICKNESS_DEFAULT      15
#define CONCRETE_THICKNESS_DEFAULT   30
#define PARTITION_THICKNESS_DEFAULT  10

// Wall's visual thickness
#define VISUAL_THICKNESS 4

// Text showing the length of the wall while placing
#define WALL_TEXT_WIDTH 50
#define WALL_TEXT_HEIGHT 20


Wall::Wall(QLineF line, int thickness) : SimulationItem() {
    m_line = line;
    m_thickness = thickness;
}

QLineF Wall::getLine() {
    return m_line;
}

void Wall::setLine(QLineF line) {
    prepareGeometryChange();
    m_line = line;
    update();
}

/**
 * @brief Wall::getRealLine
 * @return
 *
 * This function returns the real line of the wall (in meters)
 */
QLineF Wall::getRealLine() {
    qreal scale = simulationScene()->simulationScale();
    return QLineF(m_line.p1() / scale, m_line.p2() / scale);
}

int Wall::getThickness(){
    return m_thickness;
}

QPen Wall::getPen() {
    return m_pen;
}

void Wall::setPen(QPen pen) {
    prepareGeometryChange();
    m_pen = pen;
    update();
}

QRectF Wall::getLengthTextRect() const {
    const qreal line_length = m_line.length();

    // Place the text at the center and beside the line (line_length + 0.1 to avoid to divide by 0)
    return QRectF(m_line.center().x() - WALL_TEXT_WIDTH/2 + WALL_TEXT_WIDTH*0.7 * m_line.dy() / (line_length + 0.1),
                  m_line.center().y() - WALL_TEXT_HEIGHT/2 - WALL_TEXT_HEIGHT*0.8 * m_line.dx() / (line_length + 0.1),
                  WALL_TEXT_WIDTH,
                  WALL_TEXT_HEIGHT);
}

QRectF Wall::boundingRect() const {
    // Return the rectangle containing the line
    return shape().controlPointRect();
}

QPainterPath Wall::shape() const {
    QPainterPath path;
    path.moveTo(m_line.p1());
    path.lineTo(m_line.p2());

    // Add the wall's length text if we are in placing mode
    if (placingMode()) {
        path.addRect(getLengthTextRect());
    }

    // Take care or the width of the pen
    QPainterPathStroker ps;
    ps.setWidth(m_pen.widthF());
    ps.setJoinStyle(m_pen.joinStyle());
    ps.setMiterLimit(m_pen.miterLimit());

    QPainterPath p = ps.createStroke(path);
    p.addPath(path);
    return p;
}

void Wall::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    if (placingMode()) {
        // Draw the wall half-transparent while placing
        painter->setOpacity(0.6);
    }

    // No line to paint if his length is null
    if (m_line.p1() == m_line.p2())
        return;

    // Draw a circle over a line, with the origin at the end of the line
    painter->setPen(m_pen);
    //painter->setBrush(Qt::red);
    painter->drawLine(m_line);

    if (placingMode()) {
        // Draw the text at full opacity level
        painter->setOpacity(1);

        // Reset the pen to black for the text
        painter->setPen(Qt::black);

        // Show the true length in meters with an accuracy of the centimeter
        QString length_str = QString("%1 m").arg(getRealLine().length(), 0, 'f', 2);

        // Align left when the text is on the right side of the line, align right else
        Qt::AlignmentFlag align_flag = (m_line.dy() >= 0 ? Qt::AlignLeft : Qt::AlignRight);

        // Draw length beside the wall
        painter->drawText(getLengthTextRect(),
                          align_flag | Qt::AlignVCenter | Qt::TextDontClip,
                          length_str);
    }
}



BrickWall::BrickWall(QLineF line, int thickness) : Wall(line, thickness) {
    QPen pen(QBrush(QColor(201, 63, 24)), VISUAL_THICKNESS, Qt::SolidLine);
    setPen(pen);
}

// This constructor is equivalent to the main constructor but using a default thickness
BrickWall::BrickWall(QLineF line) : BrickWall(line, BRICK_THICKNESS_DEFAULT) {}


double BrickWall::getRelPermitivity() {
    return BRICK_R_PERMITTIVITY;
}
double BrickWall::getConductivity() {
    return BRICK_CONDUCTIVITY;
}
WallType::WallType BrickWall::getWallType(){
    return WallType::BrickWall;
}


ConcreteWall::ConcreteWall(QLineF line, int thickness) : Wall(line, thickness) {
    QPen pen(QBrush(QColor(156, 155, 154)), VISUAL_THICKNESS, Qt::DashLine);
    setPen(pen);
}

// This constructor is equivalent to the main constructor but using a default thickness
ConcreteWall::ConcreteWall(QLineF line) : ConcreteWall(line, CONCRETE_THICKNESS_DEFAULT) {}


double ConcreteWall::getRelPermitivity() {
    return CONCRETE_R_PERMITTIVITY;
}
double ConcreteWall::getConductivity() {
    return CONCRETE_CONDUCTIVITY;
}
WallType::WallType ConcreteWall::getWallType(){
    return WallType::ConcreteWall;
}


PartitionWall::PartitionWall(QLineF line, int thickness) : Wall(line, thickness) {
    QPen pen(QBrush(QColor(168, 125, 67)), VISUAL_THICKNESS, Qt::DotLine);
    setPen(pen);
}

// This constructor is equivalent to the main constructor but using a default thickness
PartitionWall::PartitionWall(QLineF line) : PartitionWall(line, PARTITION_THICKNESS_DEFAULT) {}

double PartitionWall::getRelPermitivity() {
    return PARTITION_R_PERMITTIVITY;
}
double PartitionWall::getConductivity() {
    return PARTITION_CONDUCTIVITY;
}
WallType::WallType PartitionWall::getWallType(){
    return WallType::PartitionWall;
}


QDataStream &operator>>(QDataStream &in, Wall *&w) {
    int type;
    int thickness;
    QLine line;

    in >> type;
    in >> thickness;
    in >> line;

    switch (type) {
    case WallType::BrickWall:
        w = new BrickWall(line, thickness);
        break;
    case WallType::ConcreteWall:
        w = new ConcreteWall(line, thickness);
        break;
    case WallType::PartitionWall:
        w = new PartitionWall(line, thickness);
        break;
    }

    return in;
}

QDataStream &operator<<(QDataStream &out, Wall *w) {
    out << w->getWallType();
    out << w->getThickness();
    out << w->getLine().toLine();

    return out;
}
