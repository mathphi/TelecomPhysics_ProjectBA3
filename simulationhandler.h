#ifndef SIMULATIONHANDLER_H
#define SIMULATIONHANDLER_H

#include <QObject>

#include "simulationdata.h"
#include "simulationitem.h"
#include "simulationscene.h"
#include "constants.h"

class SimulationHandler : QObject
{
    Q_OBJECT
public:
    SimulationHandler(SimulationScene *scene);

    SimulationData *simulationData();

    static QPointF mirror(QPointF source, Wall *wall);

    void recursiveReflection(Emitter *emitter,
                             Receiver *receiver,
                             Wall *reflect_wall,
                             QList<QPointF> images = QList<QPointF>(),
                             QList<Wall*> walls = QList<Wall*>(),
                             int level = 0);

    void computeRayPath(Emitter *emitter,
                        Receiver *receiver,
                        QList<QPointF> images = QList<QPointF>(),
                        QList<Wall*> walls = QList<Wall*>());

    void computeAllRays();

    complex<double> complexPermittivity(double e_r, double sigma, double omega);
    complex<double> characteristicImpedance(complex<double> e);
    complex<double> propagationConstant(double omega, complex<double> e);
    double air_nb_wave(double omega);

private:
    SimulationData *m_simulation_data;
    SimulationScene *m_simulation_scene;
};

#endif // SIMULATIONHANDLER_H

