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

    static QPointF mirror(QPointF origin, Wall *wall);

    void recursiveCompute(int level, QPoint emitter_pos, QPoint receiver_pos, Wall *wall);

private:
    SimulationData *m_simulation_data;
};

#endif // SIMULATIONHANDLER_H

