#ifndef SIMULATIONHANDLER_H
#define SIMULATIONHANDLER_H

#include <QObject>
#include <QElapsedTimer>
#include <QThreadPool>

#include "simulationdata.h"
#include "simulationitem.h"
#include "simulationscene.h"
#include "constants.h"
#include "raypath.h"
#include "computationunit.h"

class SimulationHandler : public QObject
{
    Q_OBJECT
public:
    SimulationHandler();

    SimulationData *simulationData();
    QList<RayPath*> getRayPathsList();

    bool isRunning();

    static QPointF mirror(QPointF source, Wall *wall);

    complex<double> computeReflection(Emitter *em, Wall *w, QLineF in_ray);
    complex<double> computeTransmissons(Emitter *em, QLineF ray, Wall *origin_wall = nullptr, Wall *target_wall = nullptr);
    complex<double> computeNominalElecField(Emitter *em, QLineF ray, double dn);

    double computeRayPower(Emitter *em, Receiver *re, QLineF ray, complex<double> En);

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

    void recursiveReflectionThreaded( Emitter *e, Receiver *r, Wall *w);

    void generateReceiversTooltip();

    void startSimulationComputation(QList<Receiver *> rcv_list);
    void stopSimulationComputation();
    void resetComputedData();

signals:
    void simulationStarted();
    void simulationFinished();
    void simulationCancelled();
    void simulationProgress(double);

private slots:
    void computationUnitFinished();

private:
    SimulationData *m_simulation_data;
    QList<Receiver*> m_receivers_list;

    QElapsedTimer m_computation_timer;

    QThreadPool m_threadpool;
    QList<ComputationUnit*> m_computation_units;
    QMutex m_mutex;

    bool m_sim_started;
    bool m_sim_cancelling;
    int m_init_cu_count;
};

#endif // SIMULATIONHANDLER_H

