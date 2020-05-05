#ifndef SIMULATIONDATA_H
#define SIMULATIONDATA_H

#include <QObject>

#include "walls.h"
#include "emitter.h"
#include "receiver.h"


namespace SimType {
enum SimType {
    PointReceiver = 0,
    AreaReceiver  = 1
};
}

class SimulationData : public QObject
{
    Q_OBJECT

public:
    SimulationData();
    SimulationData(QList<Wall*> w_l, QList<Emitter*> e_l, QList<Receiver*> r_l);

    void setInitData(QList<Wall*> w_l, QList<Emitter*> e_l, QList<Receiver*> r_l);

    static double convertPowerToWatts(double power_dbm);
    static double convertPowerTodBm(double power_watts);

    static QRgb ratioToColor(qreal ratio);

    void attachWall(Wall *w);
    void attachEmitter(Emitter *e);
    void attachReceiver(Receiver *r);

    void detachWall(Wall *w);
    void detachEmitter(Emitter *e);
    void detachReceiver(Receiver *r);

    void reset();

    QList<Wall*> getWallsList();
    QList<Emitter*> getEmittersList();
    QList<Receiver*> getReceiverList();

    int maxReflectionsCount();
    SimType::SimType simulationType();

public slots:
    void setReflectionsCount(int cnt);
    void setSimulationType(SimType::SimType t);

private:
    // Lists of all walls/emitters/recivers on the map
    QList<Wall*> m_wall_list;
    QList<Emitter*> m_emitter_list;
    QList<Receiver*> m_receiver_list;

    int m_reflections_count;
    SimType::SimType m_simulation_type;
};

// Operator overload to write the simulation data into a file
QDataStream &operator>>(QDataStream &in, SimulationData *sd);
QDataStream &operator<<(QDataStream &out, SimulationData *sd);

#endif // SIMULATIONDATA_H
