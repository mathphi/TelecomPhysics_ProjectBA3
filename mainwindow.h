#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include "simulationscene.h"
#include "walls.h"
#include "emitters.h"
#include "receivers.h"

namespace DrawActions {
enum DrawActions{
    None,
    BrickWall,
    ConcreteWall,
    PartitionWall,
    Erase,
    Emitter,
    Receiver,
};
}


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void addBrickWall();
    void addConcreteWall();
    void addPartitionWall();
    void toggleEraseMode(bool state);
    void eraseAll();
    void addEmitter();
    void addReceiver();

    void graphicsSceneRightReleased(QPoint mouse_pos);
    void graphicsSceneLeftReleased(QPoint mouse_pos);
    void graphicsSceneMouseMoved(QPoint mouse_pos);

    void actionOpen();
    void actionSave();

    void clearAllItems();
    void cancelCurrentDrawing();

    void setMouseTrackerVisible(bool visible);

private slots:
    void initMouseTracker();
    void setMouseTrackerPosition(QPoint pos);

private:
    QPoint moveAligned(QPoint start, QPoint actual);
    QPoint attractivePoint(QPoint actual);

    Ui::MainWindow *ui;
    SimulationScene *m_scene;

    DrawActions::DrawActions m_draw_action;
    QGraphicsItem *m_drawing_item;

    QList<Wall*> m_wall_list; // List of all walls on the map
    QList<Emitter*> m_emitter_list;
    QList<Receiver*> m_receiver_list;

    bool m_mouse_tracker_visible;
    QGraphicsLineItem *m_mouse_tracker_x;
    QGraphicsLineItem *m_mouse_tracker_y;
};
#endif // MAINWINDOW_H
