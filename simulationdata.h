#ifndef SIMULATIONDATA_H
#define SIMULATIONDATA_H

#include <QObject>

#include "walls.h"
#include "emitters.h"
#include "receivers.h"

class SimulationData : public QObject
{
    Q_OBJECT

public:
    SimulationData(int refl_cnt);
    SimulationData(QList<Wall*> w_l, QList<Emitter*> e_l, QList<Receiver*> r_l, int refl_count);

    void setInitData(QList<Wall*> w_l, QList<Emitter*> e_l, QList<Receiver*> r_l, int refl_count);

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

public slots:
    void setReflectionsCount(int cnt);

private:
    // Lists of all walls/emitters/recivers on the map
    QList<Wall*> m_wall_list;
    QList<Emitter*> m_emitter_list;
    QList<Receiver*> m_receiver_list;

    int m_reflections_count;
};

// Operator overload to write the simulation data into a file
QDataStream &operator>>(QDataStream &in, SimulationData *sd);
QDataStream &operator<<(QDataStream &out, SimulationData *sd);

#endif // SIMULATIONDATA_H
