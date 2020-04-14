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

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void wheelEvent(QGraphicsSceneWheelEvent *event) override;

    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;

signals:
    void mouseLeftPressed(QPoint pos, Qt::KeyboardModifiers mod_keys);
    void mouseLeftReleased(QPoint pos, Qt::KeyboardModifiers mod_keys);
    void mouseRightPressed(QPoint pos, Qt::KeyboardModifiers mod_keys);
    void mouseRightReleased(QPoint pos, Qt::KeyboardModifiers mod_keys);
    void mouseMoved(QPoint pos, Qt::KeyboardModifiers mod_keys);

    void mouseWheelEvent(QPoint pos, int delta, Qt::KeyboardModifiers mod_keys);

    void keyPressed(QKeyEvent *e);
    void keyReleased(QKeyEvent *e);
};

#endif // SIMULATIONSCENE_H
