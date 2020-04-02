#include "simulationscene.h"
#include <QGraphicsSceneMouseEvent>

void SimulationScene::mousePressEvent(QGraphicsSceneMouseEvent *event){
    emit mousePressed(event->scenePos().toPoint());

}
void SimulationScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    emit mouseReleased(event->scenePos().toPoint());

}
void SimulationScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    emit mouseMoved(event->scenePos().toPoint());

}
