#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "walls.h"
#include "emitters.h"
#include "receivers.h"
#include "emitterdialog.h"

#include <QDebug>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QFileDialog>
#include <QKeyEvent>

#define ALIGN_THRESHOLD 16
#define PROXIMITY_SIZE 16
#define ERASER_SIZE 20


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


    //TODO: add a confirmation dialog before to quit...
    // Window File menu actions
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionOpen, SIGNAL(triggered()),this, SLOT(actionOpen()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(actionSave()));

    // Window Edit menu actions
    connect(ui->actionAddBrickWall,     SIGNAL(triggered()),   this, SLOT(addBrickWall()));
    connect(ui->actionAddConcreteWall,  SIGNAL(triggered()),   this, SLOT(addConcreteWall()));
    connect(ui->actionAddPartitionWall, SIGNAL(triggered()),   this, SLOT(addPartitionWall()));
    connect(ui->actionAddEmitter,       SIGNAL(triggered()),   this, SLOT(addEmitter()));
    connect(ui->actionAddReceiver,      SIGNAL(triggered()),   this, SLOT(addReceiver()));
    connect(ui->actionEraseObject,      SIGNAL(triggered(bool)), this, SLOT(toggleEraseMode(bool)));
    connect(ui->actionEraseAll,         SIGNAL(triggered()),   this, SLOT(eraseAll()));

    // Right-panel buttons
    connect(ui->button_addBrickWall,    SIGNAL(clicked()),      this, SLOT(addBrickWall()));
    connect(ui->button_addConcreteWall, SIGNAL(clicked()),      this, SLOT(addConcreteWall()));
    connect(ui->button_addPartition,    SIGNAL(clicked()),      this, SLOT(addPartitionWall()));
    connect(ui->button_addEmitter,      SIGNAL(clicked()),      this, SLOT(addEmitter()));
    connect(ui->button_addReceiver,     SIGNAL(clicked()),      this, SLOT(addReceiver()));
    connect(ui->button_eraseObject,     SIGNAL(clicked(bool)),  this, SLOT(toggleEraseMode(bool)));
    connect(ui->button_eraseAll,        SIGNAL(clicked()),      this, SLOT(eraseAll()));

    // Scene events
    connect(m_scene, SIGNAL(mouseRightReleased(QPoint,Qt::KeyboardModifiers)),
            this, SLOT(graphicsSceneRightReleased(QPoint,Qt::KeyboardModifiers)));
    connect(m_scene, SIGNAL(mouseLeftReleased(QPoint,Qt::KeyboardModifiers)),
            this, SLOT(graphicsSceneLeftReleased(QPoint,Qt::KeyboardModifiers)));
    connect(m_scene, SIGNAL(mouseMoved(QPoint,Qt::KeyboardModifiers)),
            this, SLOT(graphicsSceneMouseMoved(QPoint,Qt::KeyboardModifiers)));
    connect(m_scene, SIGNAL(keyPressed(QKeyEvent*)), this, SLOT(keyPressed(QKeyEvent*)));

    // Initialize the mouse tracker on the scene
    initMouseTracker();
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
}

/**
 * @brief MainWindow::addConcreteWall
 * Slot called when the button "Add concrete wall" is clicked
 */
void MainWindow::addConcreteWall() {
    cancelCurrentDrawing();

    m_draw_action = DrawActions::ConcreteWall;
    m_drawing_item = nullptr;
}

/**
 * @brief MainWindow::addPartitionWall
 * Slot called when the button "Add partition" is clicked
 */
void MainWindow::addPartitionWall() {
    cancelCurrentDrawing();

    m_draw_action = DrawActions::PartitionWall;
    m_drawing_item = nullptr;
}

void MainWindow::toggleEraseMode(bool state) {
    // Set both button and menu's action state
    ui->button_eraseObject->setChecked(state);
    ui->actionEraseObject->setChecked(state);

    if (state) {
        cancelCurrentDrawing();

        // Begin erasing
        m_draw_action = DrawActions::Erase;

        // Draw a dashed rectangle that will follow the mouse cursor (erasing area)
        QPen pen(QBrush(Qt::gray), 1, Qt::DashLine);
        QGraphicsRectItem *rect_item = new QGraphicsRectItem(0, 0, ERASER_SIZE, ERASER_SIZE);
        rect_item->hide();
        rect_item->setPen(pen);

        m_drawing_item = rect_item;
        m_scene->addItem(m_drawing_item);
    }
    else {
        // Stop erasing
        cancelCurrentDrawing();
    }
}

void MainWindow::eraseAll() {
    int answer = QMessageBox::question(
                    this,
                    "Confirmation de la suppression",
                    "Êtes-vous sûr de vouloir tout supprimer ?");

    if (answer == QMessageBox::Yes) {
        clearAllItems();
    }
}

void MainWindow::addEmitter() {
    cancelCurrentDrawing();

    // Dialog to configure the antenna
    EmitterDialog *emitter_dialog = new EmitterDialog(this);
    int ans = emitter_dialog->exec();

    if (ans == QDialog::Rejected)
        return;

    EmitterType::EmitterType type = emitter_dialog->getEmitterType();
    double power      = emitter_dialog->getPower();
    double frequency  = emitter_dialog->getFrequency();
    double resistance = emitter_dialog->getResistance();
    double efficiency = emitter_dialog->getEfficiency();

    // Create an emitter of the selected type to place on the scene
    switch (type) {
    case EmitterType::HalfWaveDipole:
        m_drawing_item = new HalfWaveDipole(frequency, power, efficiency, resistance);
        break;
    }

    // We are placing an emitter
    m_draw_action = DrawActions::Emitter;

    // Hide the item until the mouse come on the scene
    m_drawing_item->setVisible(false);
    m_scene->addItem(m_drawing_item);
}

void MainWindow::addReceiver() {
    cancelCurrentDrawing();

    // Create an Receiver to place on the scene
    m_draw_action = DrawActions::Receiver;
    m_drawing_item = new Receiver();

    // Hide the item until the mouse come on the scene
    m_drawing_item->setVisible(false);
    m_scene->addItem(m_drawing_item);
}

/**
 * @brief MainWindow::clearAllItems
 *
 * This function resets all the scene, lists and actions
 */
void MainWindow::clearAllItems() {
    // Cancel the current drawing (if one)
    cancelCurrentDrawing();

    // Clear the lists and the graphics scene
    m_wall_list.clear();
    m_emitter_list.clear();
    m_receiver_list.clear();
    m_scene->clear();

    // Re-init mouse trackers on the scene
    initMouseTracker();
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
        ui->actionEraseObject->setChecked(false);
    }

    // Remove the current placing object from the scene and delete it
    if (m_drawing_item) {
        m_scene->removeItem(m_drawing_item);
        delete m_drawing_item;
    }

    m_draw_action = DrawActions::None;
    m_drawing_item = nullptr;

    // Hide the mouse tracker
    setMouseTrackerVisible(false);
}

/**
 * @brief MainWindow::keyPressed
 * @param e
 *
 * Slot called when the used presses any key on the keyboard
 */
void MainWindow::keyPressed(QKeyEvent *e) {
    // Cancel current drawing on Escape pressed
    if (e->key() == Qt::Key_Escape) {
        cancelCurrentDrawing();
    }
}

/**
 * @brief MainWindow::graphicsSceneRightReleased
 * @param pos
 *
 * Slot called when the user releases the right button on the graphics scene
 */
void MainWindow::graphicsSceneRightReleased(QPoint pos, Qt::KeyboardModifiers mod_keys) {
    Q_UNUSED(pos); // To avoid the compiler's warning (unused variable 'pos')
    Q_UNUSED(mod_keys);

    // Right click = cancel the current action
    cancelCurrentDrawing();
}

/**
 * @brief MainWindow::graphicsSceneLeftReleased
 * @param pos
 *
 * Slot called when the user releases the left button on the graphics scene
 */
void MainWindow::graphicsSceneLeftReleased(QPoint pos, Qt::KeyboardModifiers mod_keys) {

    // If we aren't placing something yet
    if (m_drawing_item == nullptr) {

        // Actions to do on the first click
        switch (m_draw_action) {
        //////////////////////////////// WALLS ACTIONS (1st click) /////////////////////////////////
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
        //////////////////////////////// WALLS ACTIONS (2nd click) /////////////////////////////////
        case DrawActions::BrickWall:
        case DrawActions::ConcreteWall:
        case DrawActions::PartitionWall: {
            // Placing of the wall done (second click)
            Wall *wall = (Wall*) m_drawing_item;

            // Add the new Wall to the walls list
            m_wall_list.append(wall);

            // Detach the drawn wall from the mouse
            m_drawing_item = nullptr;

            // Repeat the last action if the control or shift key was pressed
            if (mod_keys & (Qt::ShiftModifier | Qt::ControlModifier)) {
                // Simulate a click on the same place, so we start a new wall of the
                // same type at the end of the previous one
                graphicsSceneLeftReleased(pos, mod_keys);
            }
            else {
                m_draw_action = DrawActions::None;
            }
            break;
        }
        //////////////////////////////// EMITTER ACTION /////////////////////////////////
        case DrawActions::Emitter: {
            Emitter *emitter = (Emitter*) m_drawing_item;
            m_emitter_list.append(emitter);

            // Repeat the last action if the control or shift key was pressed
            if (mod_keys & (Qt::ShiftModifier | Qt::ControlModifier)) {
                // Clone the last placed receiver and place it
                m_drawing_item = emitter->clone();
                m_drawing_item->setVisible(false);
                m_scene->addItem(m_drawing_item);
            }
            else {
                // Detach the placed emitter from the mouse
                m_drawing_item = nullptr;
                m_draw_action = DrawActions::None;
            }
            break;
        }
        //////////////////////////////// RECEIVER ACTION /////////////////////////////////
        case DrawActions::Receiver: {
            Receiver *receiver = (Receiver*) m_drawing_item;
            m_receiver_list.append(receiver);

            // Repeat the last action if the control or shift key was pressed
            if (mod_keys & (Qt::ShiftModifier | Qt::ControlModifier)) {
                // Re-create a copy of the last placed receiver
                m_drawing_item = new Receiver();
                m_drawing_item->setVisible(false);
                m_scene->addItem(m_drawing_item);
            }
            else {
                // Detach the placed received from the mouse
                m_drawing_item = nullptr;
                m_draw_action = DrawActions::None;
            }
            break;
        }
        //////////////////////////////// ERASE ACTION /////////////////////////////////
        case DrawActions::Erase: {
            QGraphicsRectItem *rect_item = (QGraphicsRectItem*) m_drawing_item;

            // Retreive all items under the eraser rectangle
            QRectF rect (rect_item->pos(), rect_item->rect().size());
            QList<QGraphicsItem*> trash = m_scene->items(rect);

            // Remove each items from the graphics scene and delete it
            foreach (QGraphicsItem *item, trash) {

                // Remove only the known types
                if (dynamic_cast<Wall*>(item)) {
                    m_scene->removeItem(item);

                    // Remove it from the walls list
                    m_wall_list.removeAll((Wall*) item);
                    delete item;
                }
                else if (dynamic_cast<Emitter*>(item)){
                    m_scene->removeItem(item);
                    m_emitter_list.removeAll((Emitter*) item);
                    delete item;
                }
                else if (dynamic_cast<Receiver*>(item)){
                    m_scene->removeItem(item);
                    m_receiver_list.removeAll((Receiver*) item);
                    delete item;
                }

                // Don't remove other items (ie: mouse tracker lines or
                // eraser rectancgle)
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
void MainWindow::graphicsSceneMouseMoved(QPoint pos, Qt::KeyboardModifiers mod_keys) {
    Q_UNUSED(mod_keys);

    // Show mouse tracker only if we are placing something
    setMouseTrackerVisible(m_draw_action != DrawActions::None);

    // Hide the mouse cursor on drawing (use the mouse tracker lines)
    if (m_draw_action != DrawActions::None) {
        ui->graphicsView->setCursor(Qt::BlankCursor);
    }
    else {
        ui->graphicsView->setCursor(Qt::ArrowCursor);
    }

    // Mouse tracker follows the mouse if visible
    if (m_mouse_tracker_visible) {
        setMouseTrackerPosition(pos);
    }

    // No more thing to do if we are not placing an item
    if (m_drawing_item == nullptr) {
        return;
    }

    switch (m_draw_action) {
    //////////////////////////////// WALLS ACTIONS /////////////////////////////////
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
    ////////////////////////// EMITTER/RECEIVER ACTION ////////////////////////////
    case DrawActions::Emitter:
    case DrawActions::Receiver:{
        m_drawing_item->setPos(pos);

        if (!m_drawing_item->isVisible()) {
            m_drawing_item->setVisible(true);
        }
        break;
    }
    //////////////////////////////// ERASE ACTION /////////////////////////////////
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
    double min_dist = PROXIMITY_SIZE + 1;
    QPoint attractive_point = actual;

    // Loop over each Wall of the scene
    foreach (QGraphicsItem *item, m_scene->items()) {
        // Skip this item if it's the current drawing item
        if (item == m_drawing_item)
            continue;

        // Use the dynamic cast to be shure the item is a Wall
        Wall* wall_item = dynamic_cast<Wall*>(item);

        // If this item is a wall
        if (wall_item) {
            // Save the two points of the line in a list to loop over them
            QList<QPointF> line_points;
            line_points << wall_item->line().p1() << wall_item->line().p2();

            // For the two points of the line (start and end points)
            foreach (QPointF pt, line_points) {
                QLineF bounding_line(actual, pt);

                double true_lenght       = bounding_line.length();
                double horizontal_length = abs(actual.x() - pt.x());
                double vertical_length   = abs(actual.y() - pt.y());

                // Keep the one with the closest distance to the mouse position
                if (true_lenght < min_dist) {
                    // Point over point alignment
                    min_dist = true_lenght;
                    attractive_point = pt.toPoint();
                }
                else {
                    if (horizontal_length < PROXIMITY_SIZE) {
                        // Horizontal alignment
                        attractive_point = QPoint(pt.x(), attractive_point.y());
                    }
                    if (vertical_length < PROXIMITY_SIZE) {
                        // Vertical alignment
                        attractive_point = QPoint(attractive_point.x(), pt.y());
                    }
                }
            }
        }
    }

    return attractive_point;
}


////////////////////////////////// MOUSE TRACKER SECTION /////////////////////////////////////

void MainWindow::initMouseTracker() {
    // Add two lines to the scene that will track the mouse cursor when visible
    QPen tracker_pen(QBrush(QColor(0, 0, 255, 100)), 1, Qt::DotLine);

    m_mouse_tracker_x = new QGraphicsLineItem();
    m_mouse_tracker_y = new QGraphicsLineItem();

    m_mouse_tracker_x->setPen(tracker_pen);
    m_mouse_tracker_y->setPen(tracker_pen);

    setMouseTrackerVisible(false);

    m_scene->addItem(m_mouse_tracker_x);
    m_scene->addItem(m_mouse_tracker_y);
}

void MainWindow::setMouseTrackerVisible(bool visible) {
    m_mouse_tracker_visible = visible;

    m_mouse_tracker_x->setVisible(visible);
    m_mouse_tracker_y->setVisible(visible);
}

void MainWindow::setMouseTrackerPosition(QPoint pos) {
    // Get the viewport dimensions
    QGraphicsView *view = ui->graphicsView;

    //TODO: get properly the viewport dimensions
    QLine x_line(pos.x(), -view->height(), pos.x(), view->height());
    QLine y_line(-view->width(), pos.y(), view->width(), pos.y());

    m_mouse_tracker_x->setLine(x_line);
    m_mouse_tracker_y->setLine(y_line);
}


//////////////////////////////// FILE SAVE/RESTORE HANDLING SECTION ///////////////////////////////////

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
   in >> m_emitter_list;
   in >> m_receiver_list;

   // Update the graphics scene with read data
   foreach (Wall* w, m_wall_list) {
       m_scene->addItem(w);
   }
   foreach (Emitter* e, m_emitter_list) {
       m_scene->addItem(e);
   }
   foreach (Receiver* r, m_receiver_list) {
       m_scene->addItem(r);
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
    QDataStream out (&file);
    out << m_wall_list;
    out << m_emitter_list;
    out << m_receiver_list;

    // Close the file
    file.close();
}
