#ifndef SIMULATIONSCENE_H
#define SIMULATIONSCENE_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsItem>

#include "simulationitem.h"


class SimulationScene : public QGraphicsScene
{
    Q_OBJECT

public:
    SimulationScene(QObject* parent = nullptr) : QGraphicsScene (parent) {}

    qreal simulationScale() const;

    QRectF simulationBoundingRect();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void wheelEvent(QGraphicsSceneWheelEvent *event) override;

    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;

signals:
    void mouseLeftPressed(QGraphicsSceneMouseEvent *event);
    void mouseLeftReleased(QGraphicsSceneMouseEvent *events);
    void mouseRightPressed(QGraphicsSceneMouseEvent *event);
    void mouseRightReleased(QGraphicsSceneMouseEvent *event);
    void mouseMoved(QGraphicsSceneMouseEvent *event);

    void mouseWheelEvent(QGraphicsSceneWheelEvent *event);

    void keyPressed(QKeyEvent *e);
    void keyReleased(QKeyEvent *e);
};

#endif // SIMULATIONSCENE_H
