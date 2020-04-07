#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "walls.h"

#include <QDebug>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QFileDialog>

#define ALIGN_THRESHOLD 16
#define ERASER_SIZE 20
#define PROXIMITY_SIZE 16


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
    connect(ui->button_eraseAll,        SIGNAL(clicked()), this, SLOT(eraseAll()));

    connect(m_scene, SIGNAL(mouseRightReleased(QPoint)),  this, SLOT(graphicsSceneRightReleased(QPoint)));
    connect(m_scene, SIGNAL(mouseLeftReleased(QPoint)), this, SLOT(graphicsSceneLeftReleased(QPoint)));
    connect(m_scene, SIGNAL(mouseMoved(QPoint)),    this, SLOT(graphicsSceneMouseMoved(QPoint)));

    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionOpen, SIGNAL(triggered()),this, SLOT(actionOpen()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(actionSave()));
}

MainWindow::~MainWindow() {
    delete ui;
}

/**
 * @brief MainWindow::addBrickWall
 * Slot called when the button "Add brick wall" is clicked
 */
void MainWindow::addBrickWall() {
    cancelCurrentDrawing();

    m_draw_action = DrawActions::BrickWall;
    m_drawing_item = nullptr;

    // Mouse design
    ui->graphicsView->setCursor(Qt::CrossCursor);
}

/**
 * @brief MainWindow::addConcreteWall
 * Slot called when the button "Add concrete wall" is clicked
 */
void MainWindow::addConcreteWall() {
    cancelCurrentDrawing();

    m_draw_action = DrawActions::ConcreteWall;
    m_drawing_item = nullptr;

    // Mouse design
    ui->graphicsView->setCursor(Qt::CrossCursor);
}

/**
 * @brief MainWindow::addPartitionWall
 * Slot called when the button "Add partition" is clicked
 */
void MainWindow::addPartitionWall() {
    cancelCurrentDrawing();

    m_draw_action = DrawActions::PartitionWall;
    m_drawing_item = nullptr;

    // Mouse design
    ui->graphicsView->setCursor(Qt::CrossCursor);
}

void MainWindow::toggleEraseMode(bool state) {
    if (state) {
        cancelCurrentDrawing();

        // Begin erasing
        m_draw_action = DrawActions::Erase;

        // Draw a dashed rectangle that will follow the mouse cursor (erasing area)
        QPen pen(QBrush(Qt::gray),1,Qt::DashLine);
        QGraphicsRectItem *rect_item = new QGraphicsRectItem(0,0,ERASER_SIZE,ERASER_SIZE);
        rect_item->hide();
        rect_item->setPen(pen);

        m_drawing_item = rect_item;
        m_scene->addItem(m_drawing_item);

        // Hide the cursor's pointer
        ui->graphicsView->setCursor(Qt::BlankCursor);
    }

    else {
        // Stop erasing

        if(m_drawing_item) {
            m_scene->removeItem(m_drawing_item);
            delete m_drawing_item;
        }

        m_draw_action = DrawActions::None;
        m_drawing_item = nullptr;
        ui->graphicsView->setCursor(Qt::ArrowCursor);

    }
}

void MainWindow::eraseAll() {
    int answer = QMessageBox::question(
                    this,
                    "Confirmation de la suppression",
                    "Êtes-vous sûr de vouloir tout supprimer ?");

    if (answer == QMessageBox::No) {
        return;
    }

    clearAllItems();
}

/**
 * @brief MainWindow::clearAllItems
 *
 * This function resets all the scene, lists and actions
 */
void MainWindow::clearAllItems() {
    // If we were erasing, uncheck the "Erase object" button
    if (m_draw_action == DrawActions::Erase) {
        ui->button_eraseObject->setChecked(false);
    }

    m_draw_action = DrawActions::None;
    m_drawing_item = nullptr;
    ui->graphicsView->setCursor(Qt::ArrowCursor);

    // Clear the lists and the graphics scene
    m_wall_list.clear();
    m_scene->clear();
}

/**
 * @brief MainWindow::cancelCurrentDrawing
 *
 * This function cancels the current drawing action
 */
void MainWindow::cancelCurrentDrawing() {
    // If we were erasing, uncheck the "Erase object" button
    if (m_draw_action == DrawActions::Erase) {
        ui->button_eraseObject->setChecked(false);
    }

    // Remove the current placing object from the scene and delete it
    if (m_drawing_item) {
        m_scene->removeItem(m_drawing_item);
        delete m_drawing_item;
    }

    m_draw_action = DrawActions::None;
    m_drawing_item = nullptr;

    // Mouse design
    ui->graphicsView->setCursor(Qt::ArrowCursor);
}

/**
 * @brief MainWindow::graphicsSceneRightReleased
 * @param pos
 *
 * Slot called when the user releases the right button on the graphics scene
 */
void MainWindow::graphicsSceneRightReleased(QPoint pos) {
    Q_UNUSED(pos); // To avoid the compiler's warning (unused variable 'pos')

    // Right click = cancel the current action
    cancelCurrentDrawing();
}

/**
 * @brief MainWindow::graphicsSceneLeftReleased
 * @param pos
 *
 * Slot called when the user releases the left button on the graphics scene
 */
void MainWindow::graphicsSceneLeftReleased(QPoint pos) {

    if (m_drawing_item == nullptr) {

        // Actions to do on the first click
        switch (m_draw_action) {
        case DrawActions::BrickWall: {
            // Add a brick wall to the scene
            pos = attractivePoint(pos);
            QLine line(pos, pos);
            m_drawing_item = new BrickWall(line);
            m_scene->addItem(m_drawing_item);
            break;
        }
        case DrawActions::ConcreteWall: {
            // Add a concrete wall to the scene
            pos = attractivePoint(pos);
            QLine line(pos, pos);
            m_drawing_item = new ConcreteWall(line);
            m_scene->addItem(m_drawing_item);
            break;
        }
        case DrawActions::PartitionWall: {
            // Add a partition wall to the scene
            pos = attractivePoint(pos);
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
        // Action to do when we are placing an item
        switch (m_draw_action) {
        case DrawActions::BrickWall:
        case DrawActions::ConcreteWall:
        case DrawActions::PartitionWall: {
            // Placing of the wall done (second click)
            Wall *wall = (Wall*) m_drawing_item;

            // Add the new Wall to the walls list
            m_wall_list.append(wall);

            m_drawing_item = nullptr;
            m_draw_action = DrawActions::None;

            // Mouse design
            ui->graphicsView->setCursor(Qt::ArrowCursor);
            break;
        }
        case DrawActions::Erase: {
            QGraphicsRectItem *rect_item = (QGraphicsRectItem*) m_drawing_item;

            // Retreive all items under the eraser rectangle
            QRectF rect (rect_item->pos(), rect_item->rect().size());
            QList<QGraphicsItem*> trash = m_scene->items(rect);

            // Remove the eraser's rectangle from the trash selection list
            trash.removeAll(rect_item);

            // Remove each items from the graphics scene and delete it
            foreach (QGraphicsItem *item, trash) {
                m_scene->removeItem(item);

                // If it's a Wall -> remove it from the walls list
                if ((Wall*) item) {
                    m_wall_list.removeAll((Wall*) item);
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
void MainWindow::graphicsSceneMouseMoved(QPoint pos) {
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

        // Apply the moveAligned algorithm
        pos = moveAligned(line.p1(), pos);
        // Apply the attractivePoint algorithm
        pos = attractivePoint(pos);

        // Compute the new line
        QLine new_line = QLine(line.p1(), pos);

        // Replace the target point of the line by the position of the mouse
        wall_item->setLine(new_line);
        break;
    }
    case DrawActions::Erase: {
        // The rectangle of the eraser is centered on the mouse
        m_drawing_item->setPos(pos - QPoint(ERASER_SIZE/2,ERASER_SIZE/2));

        // The rectangle of the eraser starts hidden
        m_drawing_item->show();
        break;
    }
    default:
        break;
    }
}


QPoint MainWindow::moveAligned(QPoint start, QPoint actual) {
    QPoint delta = actual - start;
    QPoint end = actual;

    // Align by X if we are close to the starting X position
    if (abs(delta.x()) < ALIGN_THRESHOLD) {
        end.setX(start.x());
    }

    // Align by Y if we are close to the starting Y position
    if (abs(delta.y()) < ALIGN_THRESHOLD) {
        end.setY(start.y());
    }

    return end;
}

QPoint MainWindow::attractivePoint(QPoint actual) {
    // Observation rectangle
    QRect rect(actual - QPoint(PROXIMITY_SIZE/2, PROXIMITY_SIZE/2), QSize(PROXIMITY_SIZE, PROXIMITY_SIZE) );

    double min_dist = PROXIMITY_SIZE + 1;
    QPoint closest_point = actual;

    // Loop over each Wall inside the rectangle and keep the closest extremity
    foreach (QGraphicsItem *item,  m_scene->items(rect)) {
        // Skip this item if it's the current drawing item
        if (item == m_drawing_item)
            continue;

        Wall* wall_item = (Wall*) item;

        // If this item is a wall
        if (wall_item) {
            QLineF bounding_line1(actual, wall_item->line().p1());
            QLineF bounding_line2(actual, wall_item->line().p2());

            double lenght1 = bounding_line1.length();
            double lenght2 = bounding_line2.length();

            // Keep the one with the closest distance to the mouse position
            if (lenght1 < min_dist) {
                min_dist = lenght1;
                closest_point = wall_item->line().p1().toPoint();
            }
            if (lenght2 < min_dist) {
                min_dist = lenght2;
                closest_point = wall_item->line().p2().toPoint();
            }
        }
    }

    return closest_point;
}

void MainWindow::actionOpen() {
    int answer = QMessageBox::question(
                    this,
                    "Confirmation",
                    "L'état actuel de la simulation sera perdu.\n"
                    "Voulez-vous continuer ?");

    if (answer == QMessageBox::No) {
        return;
    }

   QString file_path = QFileDialog::getOpenFileName(this,"Ouvrir un fichier", QString(), "*.rtmap");

   // If the user cancelled the dialog
   if (file_path.isEmpty()) {
       return;
   }

   // Open the file (reading)
   QFile file(file_path);
   if (!file.open(QIODevice::ReadOnly)) {
       QMessageBox::critical(this, "Erreur", "Impossible d'ouvrir le fichier ");
       return;
   }

   // Clear all the current data
   clearAllItems();

   // Read data from the file
   QDataStream in(&file);
   in >> m_wall_list;

   // Update the graphics scene with read data
   foreach (Wall* w, m_wall_list ) {
       m_scene->addItem(w);
   }

   // Close the file
   file.close();
}

void MainWindow::actionSave() {
    QString file_path = QFileDialog::getSaveFileName(this,"Enregistrer dans un fichier", QString(), "*.rtmap");

    // If the used cancelled the dialog
    if (file_path.isEmpty()) {
        return;
    }

    // If the file hasn't the .rtmap extention -> add it
    if (file_path.split('.').last() != "rtmap") {
        file_path.append(".rtmap");
    }

    QFile file(file_path);
    // Open the file (writing)
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Erreur", "Impossible d'ouvrir le fichier ");
        return;
    }

    // Write current data into the file
    QDataStream out (& file);
    out << m_wall_list;

    // Close the file
    file.close();
}
