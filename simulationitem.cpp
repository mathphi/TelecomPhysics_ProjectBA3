#include "simulationitem.h"

SimulationItem::SimulationItem() : QGraphicsItem()
{
    m_placing_mode = false;
}

bool SimulationItem::placingMode() const {
    return m_placing_mode;
}

void SimulationItem::setPlacingMode(bool on) {
    prepareGeometryChange();
    m_placing_mode = on;
    update();
}
