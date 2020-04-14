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
    SimulationHandler(SimulationScene *scene);

    SimulationData *simulationData();

    static QPointF mirror(QPointF source, Wall *wall);

    void recursiveCompute(int level, QPoint emitter_pos, QPoint receiver_pos, Wall *wall);

private:
    SimulationData *m_simulation_data;
    SimulationScene *m_simulation_scene;
};

#endif // SIMULATIONHANDLER_H

