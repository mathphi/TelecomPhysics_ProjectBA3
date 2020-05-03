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

// ---------------------------------------------------------------------------------------------- //

// +++++++++++++++++++++++++++++++++ DATA CONVERSION FUNCTIONS ++++++++++++++++++++++++++++++++++ //

/**
 * @brief Emitter::convertPowerToWatts
 * @param power_dbm
 * @return
 *
 * This function returns the converted power in watts from a dBm value
 * (formula from the specifications document of the project)
 */
double SimulationData::convertPowerToWatts(double power_dbm) {
    // Compute the power in Watts from the dBm
    return pow(10.0, power_dbm/10.0) / 1000.0;
}

/**
 * @brief Emitter::convertPowerToWatts
 * @param power_dbm
 * @return
 *
 * This function returns the converted power in dBm from a Watts value
 */
double SimulationData::convertPowerTodBm(double power_watts) {
    // Compute the power in dBm from the Watts
    return 10 * log10(power_watts / 0.001);
}

/**
 * @brief SimulationData::ratioToColor
 * @param ratio
 * @return
 *
 * This function converts a ratio [0,1] to a color from red to blue
 */
QRgb SimulationData::ratioToColor(qreal ratio) {
    int r, g, b;

    // Ratio limited from 0 to 1
    ratio = max(0.0, min(1.0, ratio));

    if (ratio < 0.25) {
        r = 255;
        g = 255 * ratio/0.25;
        b = 0;
    }
    else if (ratio < 0.5) {
        r = 255 * (2 - ratio/0.25);
        g = 255;
        b = 0;
    }
    else if (ratio < 0.75) {
        r = 0;
        g = 255;
        b = 255 * (ratio/0.25 - 2);
    }
    else {
        r = 0;
        g = 255 * (4 - ratio/0.25);
        b = 255;
    }

    return qRgb(r, g, b);
}

// ---------------------------------------------------------------------------------------------- //

// +++++++++++++++++++ WALLS / EMITTERS / RECEIVER LISTS MANAGEMENT FUNCTIONS +++++++++++++++++++ //

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
    if (cnt < 0 || cnt > 99) {
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

// ---------------------------------------------------------------------------------------------- //

// +++++++++++++++++++++++++++ SIMULATION DATA FILE WRITING FUNCTIONS +++++++++++++++++++++++++++ //

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

// ---------------------------------------------------------------------------------------------- //
