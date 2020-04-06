#ifndef SIMULATIONSCENE_H
#define SIMULATIONSCENE_H

#include <QObject>
#include <QGraphicsScene>


class SimulationScene : public QGraphicsScene
{
    Q_OBJECT
public:
    SimulationScene(QObject* parent = nullptr) : QGraphicsScene (parent){}


protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;


signals:
    void mouseLeftPressed(QPoint pos);
    void mouseLeftReleased(QPoint pos);
    void mouseRightPressed(QPoint pos);
    void mouseRightReleased(QPoint pos);
    void mouseMoved(QPoint pos);


};

#endif // SIMULATIONSCENE_H
