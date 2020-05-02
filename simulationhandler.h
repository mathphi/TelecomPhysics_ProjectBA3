#ifndef SIMULATIONHANDLER_H
#define SIMULATIONHANDLER_H

#include <QObject>
#include <QElapsedTimer>

#include "simulationdata.h"
#include "simulationitem.h"
#include "simulationscene.h"
#include "constants.h"
#include "raypath.h"

class SimulationHandler : QObject
{
    Q_OBJECT
public:
    SimulationHandler();

    SimulationData *simulationData();
    QList<RayPath*> getRayPathsList();

    static QPointF mirror(QPointF source, Wall *wall);

    complex<double> computeReflection(Emitter *em, Wall *w, QLineF in_ray);
    complex<double> computeTransmissons(Emitter *em, QLineF ray, Wall *origin_wall = nullptr, Wall *target_wall = nullptr);
    complex<double> computeNominalElecField(Emitter *em, QLineF ray, double dn);

    double computeRayPower(Emitter *em, QLineF ray, complex<double> En);

    RayPath *computeRayPath(
            Emitter *emitter,
            Receiver *receiver,
            QList<QPointF> images = QList<QPointF>(),
            QList<Wall*> walls = QList<Wall*>());

    void recursiveReflection(
            Emitter *emitter,
            Receiver *receiver,
            Wall *reflect_wall,
            QList<QPointF> images = QList<QPointF>(),
            QList<Wall*> walls = QList<Wall*>(),
            int level = 1);

    void computeAllRays();
    void computeReceiversPower();

    void computePointReceivers();
    void computeAreaReceivers(QRectF area);

    void resetComputedData();

signals:
    void simulationStarted();
    void simulationFinished();

private:
    SimulationData *m_simulation_data;
    QList<Receiver*> m_receivers_list;

    QElapsedTimer m_computation_timer;
};

#endif // SIMULATIONHANDLER_H

