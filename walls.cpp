#include "walls.h"

#include <QPen>

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


Wall::Wall(QLine line, int thickness) : QGraphicsLineItem(line) {
    m_thickness = thickness;
}

int Wall::getThickness(){
    return m_thickness;
}


BrickWall::BrickWall(QLine line, int thickness) : Wall(line, thickness) {
    QPen pen(QBrush(QColor(201, 63, 24)), VISUAL_THICKNESS, Qt::SolidLine);
    setPen(pen);
}

// This constructor is equivalent to the main constructor but using a default thickness
BrickWall::BrickWall(QLine line) : BrickWall(line, BRICK_THICKNESS_DEFAULT) {}


double BrickWall::getRelPermitivity() {
    return BRICK_R_PERMITTIVITY;
}
double BrickWall::getConductivity() {
    return BRICK_CONDUCTIVITY;
}
WallType::WallType BrickWall::getWallType(){
    return WallType::BrickWall;
}


ConcreteWall::ConcreteWall(QLine line, int thickness) : Wall(line, thickness) {
    QPen pen(QBrush(QColor(156, 155, 154)), VISUAL_THICKNESS, Qt::DashLine);
    setPen(pen);
}

// This constructor is equivalent to the main constructor but using a default thickness
ConcreteWall::ConcreteWall(QLine line) : ConcreteWall(line, CONCRETE_THICKNESS_DEFAULT) {}


double ConcreteWall::getRelPermitivity() {
    return CONCRETE_R_PERMITTIVITY;
}
double ConcreteWall::getConductivity() {
    return CONCRETE_CONDUCTIVITY;
}
WallType::WallType ConcreteWall::getWallType(){
    return WallType::ConcreteWall;
}


PartitionWall::PartitionWall(QLine line, int thickness) : Wall(line, thickness) {
    QPen pen(QBrush(QColor(168, 125, 67)), VISUAL_THICKNESS, Qt::DotLine);
    setPen(pen);
}

// This constructor is equivalent to the main constructor but using a default thickness
PartitionWall::PartitionWall(QLine line) : PartitionWall(line, PARTITION_THICKNESS_DEFAULT) {}

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
        w = new BrickWall(line,thickness);
        break;
    case WallType::ConcreteWall:
        w = new ConcreteWall(line,thickness);
        break;
    case WallType::PartitionWall:
        w = new PartitionWall(line,thickness);
        break;
    }

    return in;
}

QDataStream &operator<<(QDataStream &out, Wall *w) {
    out << w->getWallType();
    out << w->getThickness();
    out << w->line().toLine();

    return out;
}
