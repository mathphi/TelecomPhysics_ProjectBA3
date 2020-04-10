#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>

#include "simulationscene.h"
#include "simulationhandler.h"

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

    void actionOpen();
    void actionSave();

    void clearAllItems();
    void cancelCurrentDrawing();

    void setMouseTrackerVisible(bool visible);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void initMouseTracker();
    void setMouseTrackerPosition(QPoint pos);

    void graphicsSceneRightReleased(QPoint mouse_pos, Qt::KeyboardModifiers mod_keys);
    void graphicsSceneLeftReleased(QPoint mouse_pos, Qt::KeyboardModifiers mod_keys);
    void graphicsSceneMouseMoved(QPoint mouse_pos, Qt::KeyboardModifiers mod_keys);

    void keyPressed(QKeyEvent *e);

private:
    QPoint moveAligned(QPoint start, QPoint actual);
    QPoint attractivePoint(QPoint actual);

    Ui::MainWindow *ui;

    SimulationScene *m_scene;
    SimulationHandler *m_simulation_handler;

    DrawActions::DrawActions m_draw_action;
    QGraphicsItem *m_drawing_item;

    bool m_mouse_tracker_visible;
    QGraphicsLineItem *m_mouse_tracker_x;
    QGraphicsLineItem *m_mouse_tracker_y;
};
#endif // MAINWINDOW_H
