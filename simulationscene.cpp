#include "simulationscene.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>


#define SIMULATION_SCALE 50.0

/**
 * @brief SimulationScene::simulationScale
 * @return
 *
 * This function returns the number of pixels per meter
 */
qreal SimulationScene::simulationScale() {
    return SIMULATION_SCALE;
}

void SimulationScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit mouseLeftPressed(event->scenePos().toPoint(), event->modifiers());
    }
    else if (event->button() == Qt::RightButton) {

        emit mouseRightPressed(event->scenePos().toPoint(), event->modifiers());
    }
}

void SimulationScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit mouseLeftReleased(event->scenePos().toPoint(), event->modifiers());
    }
    else if (event->button() == Qt::RightButton) {
        emit mouseRightReleased(event->scenePos().toPoint(), event->modifiers());
    }
}

void SimulationScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    emit mouseMoved(event->scenePos().toPoint(), event->modifiers());
}


void SimulationScene::keyPressEvent(QKeyEvent *event) {
    emit keyPressed(event);
}

void SimulationScene::keyReleaseEvent(QKeyEvent *event) {
    emit keyReleased(event);
}
