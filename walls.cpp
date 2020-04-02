#include "walls.h"

// Wall's relative permittivity
#define BRICK_R_PERMITTIVITY 4.6
#define CONCRETE_R_PERMITTIVITY 5
#define PARTITION_R_PERMITTIVITY 2.25

// Wall's conductivity
#define BRICK_CONDUCTIVITY 0.02
#define CONCRETE_CONDUCTIVITY 0.014
#define PARTITION_CONDUCTIVITY 0.04


Wall::Wall(QLine line, int thickness) : QGraphicsLineItem(line) {
    m_thickness = thickness;
}

int Wall::getThickness(){
    return m_thickness;
}


BrickWall::BrickWall(QLine line, int thickness) : Wall(line, thickness) {}

double BrickWall::getRelPermitivity() {
    return BRICK_R_PERMITTIVITY;
}
double BrickWall::getConductivity() {
    return BRICK_CONDUCTIVITY;
}


ConcreteWall::ConcreteWall(QLine line, int thickness) : Wall(line, thickness) {}

double ConcreteWall::getRelPermitivity() {
    return CONCRETE_R_PERMITTIVITY;
}
double ConcreteWall::getConductivity() {
    return CONCRETE_CONDUCTIVITY;
}


PartitionWall::PartitionWall(QLine line, int thickness) : Wall(line, thickness) {}

double PartitionWall::getRelPermitivity() {
    return PARTITION_R_PERMITTIVITY;
}
double PartitionWall::getConductivity() {
    return PARTITION_CONDUCTIVITY;
}
