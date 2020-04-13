#include "simulationhandler.h"

SimulationHandler::SimulationHandler()
{
    m_simulation_data = new SimulationData();
}

SimulationData *SimulationHandler::simulationData() {
    return m_simulation_data;
}

QPointF SimulationHandler::mirror(QPointF origin, Wall *wall){
    double theta = wall->getLine().angle()*M_PI/180.0 -M_PI/2;
    double x = origin.x();
    double y = origin.y();
    double a = x*cos(theta) + y*sin(theta);
    double b = -x*sin(theta) + y*cos(theta);

    return QPointF(-a*cos(theta) + b*sin(theta), a*sin(theta) + b*cos(theta));

}
