#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include "simulationscene.h"


namespace DrawActions {
enum DrawActions{
    None,
    BrickWall,
    ConcreteWall,
    PartitionWall,
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

    void graphicsScenePressed(QPoint mouse_pos);
    void graphicsSceneReleased(QPoint mouse_pos);
    void graphicsSceneMouseMoved(QPoint mouse_pos);


private:
    Ui::MainWindow *ui;
    SimulationScene *m_scene;

    DrawActions::DrawActions m_draw_action;
    QGraphicsItem *m_drawing_item;
};
#endif // MAINWINDOW_H
