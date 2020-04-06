#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "walls.h"

#include <QDebug>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QGraphicsLineItem>

#define ALIGN_THRESHOLD 10
#define ERASER_SIZE 15


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // This attribute will store the type of item we are drawing (a wall, an emitter,...)
    m_draw_action = DrawActions::None;

    // This attribute will store the item we are drawing (a line, a rectangle,...)
    m_drawing_item = nullptr;

    // Create the graphics scene
    m_scene = new SimulationScene();
    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->setMouseTracking(true);

    // Dimensions of the scene
    QRect scene_rect(QPoint(0,0), ui->graphicsView->size());
    ui->graphicsView->setSceneRect(scene_rect);

    connect(ui->button_addBrickWall,    SIGNAL(clicked()), this, SLOT(addBrickWall()));
    connect(ui->button_addConcreteWall, SIGNAL(clicked()), this, SLOT(addConcreteWall()));
    connect(ui->button_addPartition,    SIGNAL(clicked()), this, SLOT(addPartitionWall()));
    connect(ui->button_eraseObject,     SIGNAL(toggled(bool)), this, SLOT(toggleEraseMode(bool)));

    connect(m_scene, SIGNAL(mouseRightReleased(QPoint)),  this, SLOT(graphicsSceneRightReleased(QPoint)));
    connect(m_scene, SIGNAL(mouseLeftReleased(QPoint)), this, SLOT(graphicsSceneLeftReleased(QPoint)));
    connect(m_scene, SIGNAL(mouseMoved(QPoint)),    this, SLOT(graphicsSceneMouseMoved(QPoint)));
}

MainWindow::~MainWindow() {
    delete ui;
}

/**
 * @brief MainWindow::addBrickWall
 * Slot called when the button "Add brick wall" is clicked
 */
void MainWindow::addBrickWall(){
    m_draw_action = DrawActions::BrickWall;
    m_drawing_item = nullptr;

    //design mouse
    ui->graphicsView->setCursor(Qt::CrossCursor);
}

/**
 * @brief MainWindow::addConcreteWall
 * Slot called when the button "Add concrete wall" is clicked
 */
void MainWindow::addConcreteWall(){
    m_draw_action = DrawActions::ConcreteWall;
    m_drawing_item = nullptr;

    //design mouse
    ui->graphicsView->setCursor(Qt::CrossCursor);
}

/**
 * @brief MainWindow::addPartitionWall
 * Slot called when the button "Add partition" is clicked
 */
void MainWindow::addPartitionWall(){
    m_draw_action = DrawActions::PartitionWall;
    m_drawing_item = nullptr;

    //design mouse
    ui->graphicsView->setCursor(Qt::CrossCursor);
}

void MainWindow::toggleEraseMode(bool state) {
    if(state){
        m_draw_action = DrawActions::Erase;
        QGraphicsRectItem *rect_item = new QGraphicsRectItem(0,0,ERASER_SIZE,ERASER_SIZE);
        rect_item->hide();
        QPen pen(QBrush(Qt::gray),1,Qt::DashLine);
        rect_item->setPen(pen);
        m_drawing_item = rect_item;
        m_scene->addItem(m_drawing_item);
        ui->graphicsView->setCursor(Qt::BlankCursor);
    }

    else {
        if(m_drawing_item){
            m_scene->removeItem(m_drawing_item);
            delete m_drawing_item;
        }
        m_draw_action = DrawActions::None;
        m_drawing_item = nullptr;
        ui->graphicsView->setCursor(Qt::ArrowCursor);

    }
}

/**
 * @brief MainWindow::graphicsScenePressed
 * @param pos
 *
 * Slot called when the user click on the graphics scene
 */
void MainWindow::graphicsSceneRightReleased(QPoint pos) {
    //right click = undo
    if(m_draw_action == DrawActions::Erase){
        ui->button_eraseObject->setChecked(false);
    }
    if(m_drawing_item){
        m_scene->removeItem(m_drawing_item);
        delete m_drawing_item;
    }

    m_draw_action = DrawActions::None;
    m_drawing_item = nullptr;

    //design mouse
    ui->graphicsView->setCursor(Qt::ArrowCursor);
}

/**
 * @brief MainWindow::graphicsSceneReleased
 * @param pos
 *
 * Slot called when the user release his click on the graphics scene
 */
void MainWindow::graphicsSceneLeftReleased(QPoint pos) {

    if (m_drawing_item == nullptr) {

        // Actions to do on the first click
        switch (m_draw_action) {
        case DrawActions::BrickWall: {
            // Add a brick wall to the scene
            QLine line(pos, pos);
            m_drawing_item = new BrickWall(line);
            m_scene->addItem(m_drawing_item);
            break;
        }
        case DrawActions::ConcreteWall: {
            // Add a concrete wall to the scene
            QLine line(pos, pos);
            m_drawing_item = new ConcreteWall(line);
            m_scene->addItem(m_drawing_item);
            break;
        }
        case DrawActions::PartitionWall: {
            // Add a partition wall to the scene
            QLine line(pos, pos);
            m_drawing_item = new PartitionWall(line);
            m_scene->addItem(m_drawing_item);
            break;
        }

        default:
            break;
        }
    }
    else {
        // Action to do when we are placing an item (second click)


        switch (m_draw_action) {
        case DrawActions::BrickWall:
        case DrawActions::ConcreteWall:
        case DrawActions::PartitionWall: {
            Wall *wall = (Wall*) m_drawing_item;
            m_wall_list.append(wall);
            m_drawing_item = nullptr;
            m_draw_action = DrawActions::None;

            //design mouse
            ui->graphicsView->setCursor(Qt::ArrowCursor);
            break;
        }
        case DrawActions::Erase: {
            QGraphicsRectItem *rect_item = (QGraphicsRectItem*) m_drawing_item;

            QRectF rect (rect_item->pos(), rect_item->rect().size());
            QList<QGraphicsItem*> trash = m_scene->items(rect);
            trash.removeAll(rect_item);
            foreach (QGraphicsItem *item, trash) {
                m_scene->removeItem(item);

                if((Wall*) item) {
                    m_wall_list.removeAll((Wall*)item);
                }
                delete item;
            }
            break;
        }
        default:
            break;
        }

    }
}

/**
 * @brief MainWindow::graphicsSceneMouseMoved
 * @param pos
 *
 * Slot called when the mouse move over the graphics scene
 */
void MainWindow::graphicsSceneMouseMoved(QPoint pos){
    // Nothing to do if we are not placing an item
    if (m_drawing_item == nullptr) {
        return;
    }

    switch (m_draw_action) {
    case DrawActions::BrickWall:
    case DrawActions::ConcreteWall:
    case DrawActions::PartitionWall: {
        // Cast the current drawing item as a Wall
        Wall *wall_item = (Wall*) m_drawing_item;

        // Get the current line's coordinates
        QLine line = wall_item->line().toLine();

        QLine new_line = QLine(line.p1(), moveAligned(line.p1(), pos));

        // Replace the target point of the line by the position of the mouse
        wall_item->setLine(new_line);
        break;
    }
    case DrawActions::Erase:{
        m_drawing_item->setPos(pos - QPoint(ERASER_SIZE/2,ERASER_SIZE/2));
        m_drawing_item->show();
        break;
    }
    default:
        break;
    }
}


QPoint MainWindow::moveAligned(QPoint start, QPoint actual){
    QPoint delta = actual - start;
    QPoint end = actual;

    if (abs(delta.x()) < ALIGN_THRESHOLD)
    {
        end.setX(start.x());
    }

    if (abs(delta.y()) < ALIGN_THRESHOLD)
    {
        end.setY(start.y());
    }
    return end;

}
