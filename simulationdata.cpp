#include "simulationdata.h"

#define MAX_REFLECTIONS_COUNT_DEFAULT 3


SimulationData::SimulationData() : QObject()
{
    m_reflections_count = MAX_REFLECTIONS_COUNT_DEFAULT;
    m_simulation_type = SimType::PointReceiver;
}

SimulationData::SimulationData(QList<Wall*> w_l, QList<Emitter*> e_l, QList<Receiver*> r_l) : SimulationData()
{
    setInitData(w_l, e_l, r_l);
}

void SimulationData::setInitData(QList<Wall*> w_l, QList<Emitter*> e_l, QList<Receiver*> r_l) {
    m_wall_list = w_l;
    m_emitter_list = e_l;
    m_receiver_list = r_l;
}

// ++++++++++++++++++++++++ WALLS / EMITTERS / RECEIVER LISTS MANAGEMENT FUNCTIONS +++++++++++++++++++++++++ //

void SimulationData::attachWall(Wall *w) {
    m_wall_list.append(w);
}

void SimulationData::attachEmitter(Emitter *e) {
    m_emitter_list.append(e);
}

void SimulationData::attachReceiver(Receiver *r) {
    m_receiver_list.append(r);
}

void SimulationData::detachWall(Wall *w) {
    m_wall_list.removeAll(w);
}

void SimulationData::detachEmitter(Emitter *e) {
    m_emitter_list.removeAll(e);
}

void SimulationData::detachReceiver(Receiver *r) {
    m_receiver_list.removeAll(r);
}

void SimulationData::reset() {
    m_wall_list.clear();
    m_emitter_list.clear();
    m_receiver_list.clear();
}

QList<Wall*> SimulationData::getWallsList() {
    return m_wall_list;
}

QList<Emitter*> SimulationData::getEmittersList() {
    return m_emitter_list;
}

QList<Receiver*> SimulationData::getReceiverList() {
    return m_receiver_list;
}

int SimulationData::maxReflectionsCount() {
    return m_reflections_count;
}

void SimulationData::setReflectionsCount(int cnt) {
    if (cnt < 1 || cnt > 99) {
        cnt = 3;
    }

    m_reflections_count = cnt;
}

SimType::SimType SimulationData::simulationType() {
    return m_simulation_type;
}

void SimulationData::setSimulationType(SimType::SimType t) {
    m_simulation_type = t;
}

// ------------------------------------------------------------------------------------------------- //

// ++++++++++++++++++++++++++++ SIMULATION DATA FILE WRITING FUNCTIONS +++++++++++++++++++++++++++++ //

QDataStream &operator>>(QDataStream &in, SimulationData *sd) {
    sd->reset();

    QList<Wall*> walls_list;
    QList<Emitter*> emitters_list;
    QList<Receiver*> receiver_list;
    int max_refl_count;
    int sim_type;

    in >> walls_list;
    in >> emitters_list;
    in >> receiver_list;
    in >> max_refl_count;
    in >> sim_type;

    sd->setInitData(walls_list, emitters_list, receiver_list);
    sd->setReflectionsCount(max_refl_count);
    sd->setSimulationType((SimType::SimType) sim_type);

    return in;
}

QDataStream &operator<<(QDataStream &out, SimulationData *sd) {
    out << sd->getWallsList();
    out << sd->getEmittersList();
    out << sd->getReceiverList();
    out << sd->maxReflectionsCount();
    out << (int) sd->simulationType();

    return out;
}

// ------------------------------------------------------------------------------------------------- //
