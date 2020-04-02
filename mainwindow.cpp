#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "walls.h"

#include <QDebug>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QGraphicsLineItem>

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

    connect(m_scene, SIGNAL(mousePressed(QPoint)),  this, SLOT(graphicsScenePressed(QPoint)));
    connect(m_scene, SIGNAL(mouseReleased(QPoint)), this, SLOT(graphicsSceneReleased(QPoint)));
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
}

/**
 * @brief MainWindow::addConcreteWall
 * Slot called when the button "Add concrete wall" is clicked
 */
void MainWindow::addConcreteWall(){
    m_draw_action = DrawActions::ConcreteWall;
    m_drawing_item = nullptr;
}

/**
 * @brief MainWindow::addPartitionWall
 * Slot called when the button "Add partition" is clicked
 */
void MainWindow::addPartitionWall(){
    m_draw_action = DrawActions::PartitionWall;
    m_drawing_item = nullptr;
}

/**
 * @brief MainWindow::graphicsScenePressed
 * @param pos
 *
 * Slot called when the user click on the graphics scene
 */
void MainWindow::graphicsScenePressed(QPoint pos) {
    Q_UNUSED(pos); // To avoid a 'unused' warning from the compiler
}

/**
 * @brief MainWindow::graphicsSceneReleased
 * @param pos
 *
 * Slot called when the user release his click on the graphics scene
 */
void MainWindow::graphicsSceneReleased(QPoint pos) {

    if (m_drawing_item == nullptr) {

        // Actions to do on the first click
        switch (m_draw_action) {
        case DrawActions::BrickWall: {
            // Add a line to the scene
            QLine line(pos, pos);
            m_drawing_item = new QGraphicsLineItem(line);
            m_scene->addItem(m_drawing_item);
            break;
        }
        default:
            break;
        }
    }
    else {
        // Action to do when we are placing an item (second click)

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
    if (m_drawing_item == nullptr){
        return;
    }

    switch (m_draw_action) {
    case DrawActions::BrickWall: {
        // Cast the current drawing item as a QGraphicsLineItem
        QGraphicsLineItem *line_item = (QGraphicsLineItem*) m_drawing_item;

        // Get the current line's coordinates
        QLine line = line_item->line().toLine();

        // Replace the target point of the line by the position of the mouse
        line_item->setLine(QLine(line.p1(), pos));
        break;
    }
    default:
        break;
    }
}
