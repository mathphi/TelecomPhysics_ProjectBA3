#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QGraphicsLineItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_draw_action = DrawActions::None;
    m_drawing_item= nullptr;

    // Create the graphics scene
    m_scene = new SimulationScene();
    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->setMouseTracking(true);

    //Dimensions of the scene
    QRect scene_rect(QPoint(0,0), ui->graphicsView->size());
    ui->graphicsView->setSceneRect(scene_rect);

    connect(ui->button_addBrickWall,SIGNAL(clicked()),this,SLOT(addBrickWall()));
    connect(m_scene, SIGNAL(mousePressed(QPoint)), this, SLOT(graphicsScenePressed(QPoint)));
    connect(m_scene, SIGNAL(mouseReleased(QPoint)), this, SLOT(graphicsSceneReleased(QPoint)));
    connect(m_scene, SIGNAL(mouseMoved(QPoint)), this, SLOT(graphicsSceneMouseMoved(QPoint)));
}

MainWindow::~MainWindow()
{
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
 * @brief MainWindow::graphicsScenePressed
 * @param pos
 */
void MainWindow::graphicsScenePressed(QPoint pos){

}
void MainWindow::graphicsSceneReleased(QPoint pos){
    switch (m_draw_action) {
    case DrawActions::BrickWall: {
        //if first click
        if (m_drawing_item == nullptr){
            QLine line(pos,pos);
            m_drawing_item = new QGraphicsLineItem(line);
            m_scene->addItem(m_drawing_item);
        }
        else{

        }

        break;
    }
    default:
        break;
    }
}
void MainWindow::graphicsSceneMouseMoved(QPoint pos){
    if (m_drawing_item == nullptr){
        return;
    }
    switch (m_draw_action) {
    case DrawActions::BrickWall: {
        QGraphicsLineItem* line_item = (QGraphicsLineItem*)m_drawing_item;
        QLine line = line_item->line().toLine();
        line = QLine(line.p1(), pos);
        line_item->setLine(line);

        break;
    }
    default:
        break;
    }
}
