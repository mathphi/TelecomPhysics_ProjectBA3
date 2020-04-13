#include "simulationhandler.h"

#include <QDebug>

SimulationHandler::SimulationHandler()
{
    m_simulation_data = new SimulationData();
}

SimulationData *SimulationHandler::simulationData() {
    return m_simulation_data;
}

QPointF SimulationHandler::mirror(QPointF origin, Wall *wall) {
    //WARNING: the y axis is upside down in the graphics scene
    double theta = wall->getLine().angle() * M_PI / 180.0 - M_PI / 2.0;

    // Place the coordinates on the base of the wall
    double x = origin.x() - wall->getLine().p1().x();
    double y = origin.y() - wall->getLine().p1().y();

    // We use a rotation matrix :
    //      x*cos(θ) - y*sin(θ)
    //      -x*sin(θ) - y*cos(θ)
    double a = x*cos(theta) - y*sin(theta);
    double b = -x*sin(theta) - y*cos(theta);

    return QPointF(-a*cos(theta) - b*sin(theta), a*sin(theta) - b*cos(theta)) + wall->getLine().p1();
}

void SimulationHandler::recursiveCompute(int level, QPoint emitter_pos, QPoint receiver_pos, Wall *wall){
    QPointF image = mirror(emitter_pos, wall);
    QLineF virtual_ray (image, receiver_pos);
    QPointF th_reflexion_pt;
    QLineF::IntersectionType i_t = virtual_ray.intersects(wall->getLine(),&th_reflexion_pt);
    if(i_t != QLineF::BoundedIntersection){
        return;
    }
    double dn = virtual_ray.length();
}
