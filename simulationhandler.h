#ifndef SIMULATIONHANDLER_H
#define SIMULATIONHANDLER_H

#include <QObject>

#include "simulationdata.h"
#include "simulationitem.h"
#include "simulationscene.h"

class SimulationHandler : QObject
{
    Q_OBJECT
public:
    SimulationHandler();

    SimulationData *simulationData();

private:
    SimulationData *m_simulation_data;
};

#endif // SIMULATIONHANDLER_H
