#include "simulationhandler.h"

SimulationHandler::SimulationHandler()
{
    m_simulation_data = new SimulationData();
}

SimulationData *SimulationHandler::simulationData() {
    return m_simulation_data;
}
