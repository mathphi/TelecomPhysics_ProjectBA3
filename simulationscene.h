#ifndef SIMULATIONSCENE_H
#define SIMULATIONSCENE_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsItem>


class SimulationScene : public QGraphicsScene
{
    Q_OBJECT
public:
    SimulationScene(QObject* parent = nullptr);

    bool isMouseTrackerVisible();
    void setMouseTrackerVisible(bool visible);

    void removeItem(QGraphicsItem *item);

public slots:
    void clear();

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

private:
    void initMouseTracker();
    void setMouseTrackerPosition(QPoint pos);

    bool m_mouse_tracker_visible;
    QGraphicsLineItem *mouse_tracker_x;
    QGraphicsLineItem *mouse_tracker_y;
};

#endif // SIMULATIONSCENE_H
