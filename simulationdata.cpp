#include "simulationdata.h"

SimulationData::SimulationData()
{

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

// ------------------------------------------------------------------------------------------------- //

// ++++++++++++++++++++++++++++ SIMULATION DATA FILE WRITING FUNCTIONS +++++++++++++++++++++++++++++ //

QDataStream &operator>>(QDataStream &in, SimulationData *sd) {
    sd->reset();

    QList<Wall*> walls_list;
    QList<Emitter*> emitters_list;
    QList<Receiver*> receiver_list;

    in >> walls_list;
    in >> emitters_list;
    in >> receiver_list;

    sd->setInitData(walls_list, emitters_list, receiver_list);

    return in;
}

QDataStream &operator<<(QDataStream &out, SimulationData *sd) {
    out << sd->getWallsList();
    out << sd->getEmittersList();
    out << sd->getReceiverList();

    return out;
}

// ------------------------------------------------------------------------------------------------- //
