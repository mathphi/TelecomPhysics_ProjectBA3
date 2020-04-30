#ifndef SIMULATIONHANDLER_H
#define SIMULATIONHANDLER_H

#include <QObject>

#include "simulationdata.h"
#include "simulationitem.h"
#include "simulationscene.h"
#include "constants.h"
#include "raypath.h"

class SimulationHandler : QObject
{
    Q_OBJECT
public:
    SimulationHandler(SimulationScene *scene);

    SimulationData *simulationData();

    static QPointF mirror(QPointF source, Wall *wall);

    QList<RayPath*> recursiveReflection(
            Emitter *emitter,
            Receiver *receiver,
            Wall *reflect_wall,
            QList<QPointF> images = QList<QPointF>(),
            QList<Wall*> walls = QList<Wall*>(),
            int level = 0);

    RayPath *computeRayPath(
            Emitter *emitter,
            Receiver *receiver,
            QList<QPointF> images = QList<QPointF>(),
            QList<Wall*> walls = QList<Wall*>());

    double computePowerToReceiver(Receiver *r, QList<RayPath*> *raypaths_list);

    void computeAllRays();

    complex<double> computeReflection(Emitter *em, Wall *w, QLineF in_ray);
    complex<double> computeTransmissons(Emitter *em, QLineF ray, Wall *origin_wall = nullptr, Wall *target_wall = nullptr);
    complex<double> computeNominalElecField(Emitter *em, QLineF ray, double dn);

    double computeAvgPower(Emitter *em, QList<RayPath*> rp_list);


private:
    SimulationData *m_simulation_data;
    SimulationScene *m_simulation_scene;
};

#endif // SIMULATIONHANDLER_H

