#include "simulationhandler.h"

#include <QDebug>

SimulationHandler::SimulationHandler(SimulationScene *scene)
{
    m_simulation_data = new SimulationData();
    m_simulation_scene = scene;
}

SimulationData *SimulationHandler::simulationData() {
    return m_simulation_data;
}

/**
 * @brief SimulationHandler::mirror
 * @param source
 * @param wall
 * @return
 *
 * This function returns the coordinates of the "source" point
 * after an axial symmetry through the wall.
 *
 * WARNING: the y axis is upside down in the graphics scene !
 */
QPointF SimulationHandler::mirror(QPointF source, Wall *wall) {
    // Get the angle of the wall to the horizontal axis
    double theta = wall->getRealLine().angle() * M_PI / 180.0 - M_PI / 2.0;

    // Translate the origin of the coordinates system on the base of the wall
    double x = source.x() - wall->getRealLine().p1().x();
    double y = source.y() - wall->getRealLine().p1().y();

    // We use a rotation matrix :
    //   x' =  x*cos(θ) - y*sin(θ)
    //   y' = -x*sin(θ) - y*cos(θ)
    // to set the y' axis along the wall, so the image is at the opposite of the x' coordinate
    // in the new coordinates system.
    double x_p =  x*cos(theta) - y*sin(theta);
    double y_p = -x*sin(theta) - y*cos(theta);

    // Rotate and translate back to the original coordinates system
    return QPointF(-x_p*cos(theta) - y_p*sin(theta), x_p*sin(theta) - y_p*cos(theta)) + wall->getRealLine().p1();
}

void SimulationHandler::recursiveCompute(int level, QPoint emitter_pos, QPoint receiver_pos, Wall *wall){
    QPointF image = mirror(emitter_pos, wall);
    QLineF virtual_ray (image, receiver_pos);
    QPointF th_reflection_pt;
    QLineF::IntersectionType i_t = virtual_ray.intersects(wall->getLine(),&th_reflection_pt);
    if(i_t != QLineF::BoundedIntersection){
        return;
    }
    double dn = virtual_ray.length();
}
