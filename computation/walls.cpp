#include "walls.h"
#include "constants.h"
#include "interface/simulationscene.h"

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

// Wall's default thickness (meter)
#define BRICK_THICKNESS_DEFAULT      0.35
#define CONCRETE_THICKNESS_DEFAULT   0.30
#define PARTITION_THICKNESS_DEFAULT  0.15

// Wall's visual thickness (px)
#define VISUAL_THICKNESS 6

// Text showing the length of the wall while placing
#define WALL_TEXT_WIDTH 50
#define WALL_TEXT_HEIGHT 20


Wall::Wall(QLineF line, double thickness) : SimulationItem() {
    m_line = line;
    m_thickness = thickness;

    m_text_scale = 1.0;
}

void Wall::setupTooltip() {
    // Setup the tooltip with the wall's info
    QString tip("<b><i>%1</i></b><br/>"
                "<b>Épaisseur:</b> %2 cm");

    tip = tip.arg(getTypeName())
             .arg(int(m_thickness * 100));

    setToolTip(tip);
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

/**
 * @brief Wall::getNormalAngleTo
 * @param line
 * @return
 *
 * This function returns the angle made by the 'line' to the normal of the wall.
 * This angle is defined as 0 <= theta <= PI/2 (in radians)
 */
double Wall::getNormalAngleTo(QLineF line) {
    double theta = fabs(M_PI_2 - m_line.angleTo(line) / 180.0 * M_PI);

    // If the angle is > PI/2 -> use the normal of the wall in the other direction
    if(theta > M_PI_2) {
       theta = fabs(theta - M_PI);
    }

    return theta;
}

double Wall::getThickness(){
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

QBrush Wall::getBrush() {
    return m_brush;
}

void Wall::setBrush(QBrush b) {
    prepareGeometryChange();
    m_brush = b;
    update();
}

QRectF Wall::getLengthTextRect() const {
    // Length of the line (in pixels)
    const qreal line_length = m_line.length();

    // Unscaled WALL_TEXT_WIDTH and WALL_TEXT_HEIGHT
    const qreal text_width = WALL_TEXT_WIDTH * m_text_scale;
    const qreal text_height = WALL_TEXT_HEIGHT * m_text_scale;

    // Place the text at the center and beside the line (line_length + 0.1 to avoid to divide by 0)
    return QRectF(m_line.center().x() - text_width/2 + text_width*0.7 * m_line.dy() / (line_length + 0.1),
                  m_line.center().y() - text_height/2 - text_height*0.8 * m_line.dx() / (line_length + 0.1),
                  text_width,
                  text_height);
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

    // Take care of the width of the pen
    QPainterPathStroker ps;
    ps.setWidth(VISUAL_THICKNESS + m_pen.widthF());
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

    // Virtually trace the wall path
    QPainterPath path;
    path.moveTo(m_line.p1());
    path.lineTo(m_line.p2());

    // Get the bounding path of the line representing the wall
    QPainterPathStroker ps;
    ps.setWidth(VISUAL_THICKNESS);
    QPainterPath p = ps.createStroke(path);

    // The brush pattern must follow the wall's scale and rotation
    QTransform tr;
    tr.rotate(-m_line.angle());
    tr.scale(1.0/painter->transform().m11(),1.0/painter->transform().m22());
    m_brush.setTransform(tr);

    // Set the wall's specific brush and pen
    painter->setBrush(m_brush);
    painter->setPen(m_pen);

    // Set an opaque white background
    painter->setBackgroundMode(Qt::OpaqueMode);
    painter->setBackground(QColor(Qt::white));

    // Draw the path representing the wall
    painter->drawPath(p);

    // Reset the transparent background
    painter->setBackgroundMode(Qt::TransparentMode);

    if (placingMode()) {
        // Draw the text at full opacity level
        painter->setOpacity(1);

        // Reset the pen to black for the text
        painter->setPen(Qt::black);
        painter->setBrush(QBrush());

        // Store the new text scale (inverse of the current view scale)
        m_text_scale = 1.0 / painter->transform().m11();

        // Scale the font size to keep a readable measure reagardless of the zoom
        QFont f = painter->font();
        f.setPointSizeF(f.pointSizeF() * m_text_scale);
        painter->setFont(f);

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



BrickWall::BrickWall(QLineF line, double thickness) : Wall(line, thickness) {
    setPen(QPen(Qt::black, 1));
    setBrush(QBrush(Qt::black, Qt::SolidPattern));
    setZValue(1002);

    setupTooltip();
}

// This constructor is equivalent to the main constructor but using a default thickness
BrickWall::BrickWall(QLineF line) : BrickWall(line, BRICK_THICKNESS_DEFAULT) {}

QString BrickWall::getTypeName() const {
    return "Mur en briques";
}

double BrickWall::getRelPermitivity() const {
    return BRICK_R_PERMITTIVITY;
}
double BrickWall::getConductivity() const {
    return BRICK_CONDUCTIVITY;
}
WallType::WallType BrickWall::getWallType() const{
    return WallType::BrickWall;
}


ConcreteWall::ConcreteWall(QLineF line, double thickness) : Wall(line, thickness) {
    setPen(QPen(Qt::black, 1));
    setBrush(QBrush(Qt::black, Qt::BDiagPattern));
    setZValue(1001);

    setupTooltip();
}

// This constructor is equivalent to the main constructor but using a default thickness
ConcreteWall::ConcreteWall(QLineF line) : ConcreteWall(line, CONCRETE_THICKNESS_DEFAULT) {}

QString ConcreteWall::getTypeName() const {
    return "Mur en béton";
}

double ConcreteWall::getRelPermitivity() const {
    return CONCRETE_R_PERMITTIVITY;
}
double ConcreteWall::getConductivity() const {
    return CONCRETE_CONDUCTIVITY;
}
WallType::WallType ConcreteWall::getWallType() const {
    return WallType::ConcreteWall;
}


PartitionWall::PartitionWall(QLineF line, double thickness) : Wall(line, thickness) {
    setPen(QPen(Qt::black, 1));
    setBrush(QBrush(Qt::white, Qt::SolidPattern));
    setZValue(1000);

    setupTooltip();
}

// This constructor is equivalent to the main constructor but using a default thickness
PartitionWall::PartitionWall(QLineF line) : PartitionWall(line, PARTITION_THICKNESS_DEFAULT) {}

QString PartitionWall::getTypeName() const {
    return "Cloison";
}

double PartitionWall::getRelPermitivity() const {
    return PARTITION_R_PERMITTIVITY;
}
double PartitionWall::getConductivity() const {
    return PARTITION_CONDUCTIVITY;
}
WallType::WallType PartitionWall::getWallType() const {
    return WallType::PartitionWall;
}


QDataStream &operator>>(QDataStream &in, Wall *&w) {
    int type;
    double thickness;
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
