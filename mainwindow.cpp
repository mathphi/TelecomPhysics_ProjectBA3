#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>
#include <QGraphicsScene>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_scene = new QGraphicsScene();
    ui->graphicsView->setScene(m_scene);

    connect(ui->button_addBrickWall,SIGNAL(clicked()),this,SLOT(addBrickWall()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addBrickWall(){
    m_scene->addEllipse(0,0,100,150);

}
