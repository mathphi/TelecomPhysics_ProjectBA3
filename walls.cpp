#include "walls.h"

#define BRICK_R_PERMITTIVITY 4.6
#define CONCRETE_R_PERMITTIVITY 5
#define PARTITION_R_PERMITTIVITY 2.25




Wall::Wall(QLine line, int thickness) : QGraphicsLineItem(line)
{
m_thickness = thickness;

}

int Wall::getThickness(){
    return m_thickness;
}


BrickWall::BrickWall(QLine line, int thickness) : Wall(line, thickness){
    m_rpermittivity = BRICK_R_PERMITTIVITY;
}
