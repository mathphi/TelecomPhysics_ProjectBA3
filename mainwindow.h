#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include "simulationscene.h"
#include "walls.h"

namespace DrawActions {
enum DrawActions{
    None,
    BrickWall,
    ConcreteWall,
    PartitionWall,
    Erase,
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

    void graphicsSceneRightReleased(QPoint mouse_pos);
    void graphicsSceneLeftReleased(QPoint mouse_pos);
    void graphicsSceneMouseMoved(QPoint mouse_pos);


private:
    QPoint moveAligned(QPoint start, QPoint actual);

    Ui::MainWindow *ui;
    SimulationScene *m_scene;

    DrawActions::DrawActions m_draw_action;
    QGraphicsItem *m_drawing_item;
    QList<Wall*> m_wall_list; //list of all walls on the map
};
#endif // MAINWINDOW_H
