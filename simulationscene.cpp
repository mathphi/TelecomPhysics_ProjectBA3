#include "simulationscene.h"
#include "simulationitem.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>


#define SIMULATION_SCALE 50.0

/**
 * @brief SimulationScene::simulationScale
 * @return
 *
 * This function returns the number of pixels per meter
 */
qreal SimulationScene::simulationScale() const {
    return SIMULATION_SCALE;
}
#include <QDebug>
/**
 * @brief SimulationScene::simulationBoundingRect
 * @return
 *
 * This function returns the bounding rect containing all simulation items of the scene
 */
QRectF SimulationScene::simulationBoundingRect() {
    QRectF bounding_rect;

    foreach (QGraphicsItem *item, items()) {
        SimulationItem *s_i = dynamic_cast<SimulationItem*>(item);

        if (s_i) {
            // Bounding rect is a rectangle containing bounding rects of all items
            bounding_rect = bounding_rect.united(s_i->boundingRect().translated(s_i->pos()));
        }
    }

    return bounding_rect;
}

void SimulationScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit mouseLeftPressed(event);
    }
    else if (event->button() == Qt::RightButton) {
        emit mouseRightPressed(event);
    }
}

void SimulationScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit mouseLeftReleased(event);
    }
    else if (event->button() == Qt::RightButton) {
        emit mouseRightReleased(event);
    }
}

void SimulationScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    emit mouseMoved(event);
}


void SimulationScene::wheelEvent(QGraphicsSceneWheelEvent *event) {
    emit mouseWheelEvent(event);
}


void SimulationScene::keyPressEvent(QKeyEvent *event) {
    emit keyPressed(event);
}

void SimulationScene::keyReleaseEvent(QKeyEvent *event) {
    emit keyReleased(event);
}
