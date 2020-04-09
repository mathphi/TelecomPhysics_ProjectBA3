#include "simulationscene.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>


SimulationScene::SimulationScene(QObject* parent) : QGraphicsScene (parent) {
    initMouseTracker();
}

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
    if (isMouseTrackerVisible()) {
        setMouseTrackerPosition(event->scenePos().toPoint());
    }

    emit mouseMoved(event->scenePos().toPoint());
}

void SimulationScene::initMouseTracker() {
    // Add two lines to the scene that will track the mouse cursor when visible
    QPen tracker_pen(QBrush(QColor(0, 0, 255, 100)), 1, Qt::DotLine);

    mouse_tracker_x = new QGraphicsLineItem();
    mouse_tracker_y = new QGraphicsLineItem();

    mouse_tracker_x->setPen(tracker_pen);
    mouse_tracker_y->setPen(tracker_pen);

    setMouseTrackerVisible(false);

    addItem(mouse_tracker_x);
    addItem(mouse_tracker_y);
}

bool SimulationScene::isMouseTrackerVisible() {
    return m_mouse_tracker_visible;
}

void SimulationScene::setMouseTrackerVisible(bool visible) {
    m_mouse_tracker_visible = visible;

    mouse_tracker_x->setVisible(visible);
    mouse_tracker_y->setVisible(visible);
}

#include <QDebug>

void SimulationScene::setMouseTrackerPosition(QPoint pos) {
    if (views().size() == 0) {
        // If there is no viewport
        return;
    }

    // Get the viewport dimensions
    QGraphicsView *view = views().first();

    //TODO: get properly the viewport dimensions
    QLine x_line(pos.x(), -view->height(), pos.x(), view->height());
    QLine y_line(-view->width(), pos.y(), view->width(), pos.y());

    mouse_tracker_x->setLine(x_line);
    mouse_tracker_y->setLine(y_line);
}

void SimulationScene::clear() {
    QGraphicsScene::clear();
    initMouseTracker();
}
