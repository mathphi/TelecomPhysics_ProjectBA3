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

    complex<double>computeReflexion(Emitter *e, Wall *w, QLineF ray_in);
    complex<double>computeTransmissons(Emitter *e, QLineF ray, Wall *origin_wall=nullptr, Wall *target_wall=nullptr);

    complex<double>computeNominal_elec_field(Emitter *e, QLineF ray, double dn);
    complex<double> complexPermittivity(double e_r, double sigma, double omega);
    complex<double> characteristicImpedance(complex<double> e);
    complex<double> propagationConstant(double omega, complex<double> e);
    double air_nb_wave(double omega);
   // complex<double> reflexion_m(double theta, complex<double> Z1,Z2,complex<double>);

private:
    SimulationData *m_simulation_data;
    SimulationScene *m_simulation_scene;
};

#endif // SIMULATIONHANDLER_H

