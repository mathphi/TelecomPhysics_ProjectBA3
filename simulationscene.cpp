#include "simulationscene.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>


void SimulationScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit mouseLeftPressed(event->scenePos().toPoint());
    }
    else if (event->button() == Qt::RightButton) {

        emit mouseRightPressed(event->scenePos().toPoint());
    }
}

void SimulationScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit mouseLeftReleased(event->scenePos().toPoint());
    }
    else if (event->button() == Qt::RightButton) {
        emit mouseRightReleased(event->scenePos().toPoint());
    }
}

void SimulationScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    emit mouseMoved(event->scenePos().toPoint());
}
