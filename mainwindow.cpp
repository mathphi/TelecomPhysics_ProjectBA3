#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "walls.h"
#include "emitter.h"
#include "receiver.h"
#include "emitterdialog.h"

#include <QDebug>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QFileDialog>
#include <QLabel>


#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
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

    // This attribute is true when we are dragging the scene view with the mouse
    m_dragging_view = false;

    // This item will store the area of simulation (if type == area)
    m_sim_area_item = nullptr;

    // The default mode for UI is the EditorMode
    m_ui_mode = UIMode::EditorMode;

    // Create the graphics scene
    m_scene = new SimulationScene();
    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->setMouseTracking(true);

    // Dimensions of the scene
    QRect scene_rect(QPoint(0,0), ui->graphicsView->size());
    ui->graphicsView->setSceneRect(scene_rect);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Enable antialiasing for the graphics view
    ui->graphicsView->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // The simulation handler manages the simulation's data
    m_simulation_handler = new SimulationHandler();

    // Hide the simulation group by default
    ui->group_simulation->hide();

    // Window File menu actions
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(actionOpen()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(actionSave()));

    // Window Edit menu actions
    connect(ui->actionAddBrickWall,     SIGNAL(triggered()),     this, SLOT(addBrickWall()));
    connect(ui->actionAddConcreteWall,  SIGNAL(triggered()),     this, SLOT(addConcreteWall()));
    connect(ui->actionAddPartitionWall, SIGNAL(triggered()),     this, SLOT(addPartitionWall()));
    connect(ui->actionAddEmitter,       SIGNAL(triggered()),     this, SLOT(addEmitter()));
    connect(ui->actionAddReceiver,      SIGNAL(triggered()),     this, SLOT(addReceiver()));
    connect(ui->actionEraseObject,      SIGNAL(triggered(bool)), this, SLOT(toggleEraseMode(bool)));
    connect(ui->actionEraseAll,         SIGNAL(triggered()),     this, SLOT(eraseAll()));

    // Window View menu actions
    connect(ui->actionZoomIn,       SIGNAL(triggered()), this, SLOT(actionZoomIn()));
    connect(ui->actionZoomOut,      SIGNAL(triggered()), this, SLOT(actionZoomOut()));
    connect(ui->actionZoomReset,    SIGNAL(triggered()), this, SLOT(actionZoomReset()));
    connect(ui->actionZoomBest,     SIGNAL(triggered()), this, SLOT(actionZoomBest()));

    // Right-panel buttons
    // Scene edition buttons group
    connect(ui->button_addBrickWall,    SIGNAL(clicked()),      this, SLOT(addBrickWall()));
    connect(ui->button_addConcreteWall, SIGNAL(clicked()),      this, SLOT(addConcreteWall()));
    connect(ui->button_addPartition,    SIGNAL(clicked()),      this, SLOT(addPartitionWall()));
    connect(ui->button_addEmitter,      SIGNAL(clicked()),      this, SLOT(addEmitter()));
    connect(ui->button_addReceiver,     SIGNAL(clicked()),      this, SLOT(addReceiver()));
    connect(ui->button_eraseObject,     SIGNAL(clicked(bool)),  this, SLOT(toggleEraseMode(bool)));
    connect(ui->button_eraseAll,        SIGNAL(clicked()),      this, SLOT(eraseAll()));
    connect(ui->button_simulation,      SIGNAL(clicked()),      this, SLOT(switchSimulationMode()));

    // Simulation buttons group
    connect(ui->button_simControl, SIGNAL(clicked()),         this, SLOT(simulationControlAction()));
    connect(ui->button_simReset,   SIGNAL(clicked()),         this, SLOT(simulationResetAction()));
    connect(ui->button_editScene,  SIGNAL(clicked()),         this, SLOT(switchEditSceneMode()));
    connect(ui->button_simExport,  SIGNAL(clicked()),         this, SLOT(exportSimulationAction()));
    connect(ui->checkbox_rays,     SIGNAL(toggled(bool)),     this, SLOT(raysCheckboxToggled(bool)));
    connect(ui->slider_threshold,  SIGNAL(valueChanged(int)), this, SLOT(raysThresholdChanged(int)));

    connect(ui->combobox_simType,  SIGNAL(currentIndexChanged(int)),
            this, SLOT(simulationTypeChanged()));
    connect(ui->spinbox_reflections, SIGNAL(valueChanged(int)),
            m_simulation_handler->simulationData(), SLOT(setReflectionsCount(int)));

    // Simulation handler signals
    connect(m_simulation_handler, SIGNAL(simulationStarted()), this, SLOT(simulationStarted()));
    connect(m_simulation_handler, SIGNAL(simulationFinished()), this, SLOT(simulationFinished()));
    connect(m_simulation_handler, SIGNAL(simulationCancelled()), this, SLOT(simulationCancelled()));
    connect(m_simulation_handler, SIGNAL(simulationProgress(double)), this, SLOT(simulationProgress(double)));

    // Scene events handling
    connect(m_scene, SIGNAL(mouseRightReleased(QGraphicsSceneMouseEvent*)),
            this, SLOT(graphicsSceneRightReleased(QGraphicsSceneMouseEvent*)));
    connect(m_scene, SIGNAL(mouseLeftPressed(QGraphicsSceneMouseEvent*)),
            this, SLOT(graphicsSceneLeftPressed(QGraphicsSceneMouseEvent*)));
    connect(m_scene, SIGNAL(mouseLeftReleased(QGraphicsSceneMouseEvent*)),
            this, SLOT(graphicsSceneLeftReleased(QGraphicsSceneMouseEvent*)));
    connect(m_scene, SIGNAL(mouseMoved(QGraphicsSceneMouseEvent*)),
            this, SLOT(graphicsSceneMouseMoved(QGraphicsSceneMouseEvent*)));
    connect(m_scene, SIGNAL(mouseDoubleClicked(QGraphicsSceneMouseEvent*)),
            this, SLOT(graphicsSceneDoubleClicked(QGraphicsSceneMouseEvent*)));
    connect(m_scene, SIGNAL(mouseWheelEvent(QGraphicsSceneWheelEvent*)),
            this, SLOT(graphicsSceneWheelEvent(QGraphicsSceneWheelEvent*)));
    connect(m_scene, SIGNAL(keyPressed(QKeyEvent*)), this, SLOT(keyPressed(QKeyEvent*)));

    // Initialize the mouse tracker on the scene
    initMouseTracker();

    // Update the simulation UI to match the simulation data
    updateSimulationUI();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    int ans = QMessageBox::question(
                this,
                "Quitter",
                "Les modifications non enregistrées seront perdues.\n"
                "Voulez-vous vraiment quitter la simulation ?");

    // Close the window only if the user clicked the Yes button
    if (ans == QMessageBox::Yes) {
        event->accept();
    }
    else {
        event->ignore();
    }
}

void MainWindow::showEvent(QShowEvent *event) {
    event->accept();
    resetView();
    updateSceneRect();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    event->accept();
    updateSceneRect();
}

/**
 * @brief MainWindow::updateSceneRect
 *
 * This function updates the scene rect (when the window is resized or shown)
 */
void MainWindow::updateSceneRect() {
    // The scale factor is the diagonal components of the transformation matrix
    qreal scale_factor = ui->graphicsView->transform().m11();

    // Offset to avoid the scrollbars
    int offset = ceil(2/scale_factor);

    // Get the previous scene rect defined and extract his position from the center of the graphics view
    QRectF prev_rect = ui->graphicsView->sceneRect();
    QPointF prev_pos =
            prev_rect.topLeft() + QPointF(
                prev_rect.width() + offset, prev_rect.height() + offset) / 2.0;

    // Apply the previous position to the new graphics view size
    // Remove 10px to the new size to avoid the scrolls bars
    QPointF new_pos = prev_pos - QPointF(ui->graphicsView->width(), ui->graphicsView->height()) / scale_factor / 2.0;
    QRectF new_rect(new_pos, ui->graphicsView->size() / scale_factor - QSize(offset, offset));

    // Apply the new computed scene rect
    ui->graphicsView->setSceneRect(new_rect);

    // Send the changed of the scene rect to the scene
    m_scene->viewRectChanged(ui->graphicsView->sceneRect(), ui->graphicsView->transform().m11());
}

void MainWindow::moveSceneView(QPointF delta) {
    ui->graphicsView->setSceneRect(
                ui->graphicsView->sceneRect().x() + delta.x(),
                ui->graphicsView->sceneRect().y() + delta.y(),
                ui->graphicsView->sceneRect().width(),
                ui->graphicsView->sceneRect().height());

    // Send the changed of the scene rect to the scene
    m_scene->viewRectChanged(ui->graphicsView->sceneRect(), ui->graphicsView->transform().m11());
}

void MainWindow::scaleView(double scale, QPointF pos) {
    // Don't scale too high or too low
    if (ui->graphicsView->transform().m11() * scale > 10.0 ||
            ui->graphicsView->transform().m11() * scale < 0.1) {
        return;
    }

    QRectF scene_rect = ui->graphicsView->sceneRect();

    // Compute the position of the mouse from the center of the scene
    QPointF centered_pos = pos - scene_rect.topLeft() - (scene_rect.bottomRight() - scene_rect.topLeft()) / 2.0;

    // Compute a delta position proportionnal to the scale factor and
    // the centered mouse position
    QPointF delta_pos = (scale - 1.0) * centered_pos;

    // Apply the scaling and the delta position
    ui->graphicsView->scale(scale, scale);
    moveSceneView(delta_pos);

    // The scene dimensions changed
    updateSceneRect();
}

void MainWindow::resetView() {
    ui->graphicsView->resetTransform();
    ui->graphicsView->resetMatrix();
    updateSceneRect();

    QPointF view_delta(
                ui->graphicsView->sceneRect().x() + ui->graphicsView->sceneRect().width() / 2.0,
                ui->graphicsView->sceneRect().y() + ui->graphicsView->sceneRect().height() / 2.0);

    moveSceneView(-view_delta);
}

void MainWindow::bestView() {
    // Get the bounding rectangle of all items of the scene
    QRectF bounding_rect = m_scene->simulationBoundingRect();

    // If there is nothing on the scene, reset the view
    if (bounding_rect.isNull()) {
        resetView();
        return;
    }

    // Add a margin to this rectangle
    bounding_rect.adjust(-50.0, -50.0, 50.0, 50.0);

    // The view scale is the diagonal components of the transformation matrix
    qreal view_scale = ui->graphicsView->transform().m11();

    // Get the most limiting scale factor
    qreal scale_factor = qMin(
                ui->graphicsView->width() / bounding_rect.width(),
                ui->graphicsView->height() / bounding_rect.height());

    scale_factor /= view_scale;

    // Offset to avoid the scrollbars
    int offset = ceil(2/view_scale);

    // Get the new rect
    QRectF view_rect(
                bounding_rect.x() + bounding_rect.width() / 2.0 - ui->graphicsView->width() / view_scale / 2.0,
                bounding_rect.y() + bounding_rect.height() / 2.0 - ui->graphicsView->height() / view_scale / 2.0,
                ui->graphicsView->width() / view_scale - offset,
                ui->graphicsView->height() / view_scale - offset);

    // Scale the view to fit the bounding rect in the view
    scaleView(scale_factor);

    // Apply the new rect scaled by the scale_factor
    ui->graphicsView->setSceneRect(QRectF(view_rect.topLeft(), view_rect.size()));

    // Clean the scene rect dimensions
    updateSceneRect();
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

void MainWindow::configureEmitter(Emitter *em) {
    // Dialog to configure the emitter
    EmitterDialog emitter_dialog(em, this);
    int ans = emitter_dialog.exec();

    if (ans == QDialog::Rejected)
        return;

    AntennaType::AntennaType type = emitter_dialog.getAntennaType();
    double power      = emitter_dialog.getPower();
    double frequency  = emitter_dialog.getFrequency();
    double efficiency = emitter_dialog.getEfficiency();

    em->setPower(power);
    em->setFrequency(frequency);
    em->setAntenna(type, efficiency);
}

void MainWindow::addEmitter() {
    cancelCurrentDrawing();

    // Dialog to configure the emitter
    EmitterDialog emitter_dialog(this);
    int ans = emitter_dialog.exec();

    if (ans == QDialog::Rejected)
        return;

    AntennaType::AntennaType type = emitter_dialog.getAntennaType();
    double power      = emitter_dialog.getPower();
    double frequency  = emitter_dialog.getFrequency();
    double efficiency = emitter_dialog.getEfficiency();

    // Create an emitter of the selected type to place on the scene
    m_drawing_item = new Emitter(frequency, power, efficiency, type);

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

    // Clear the lists
    m_simulation_handler->simulationData()->reset();

    // Remove all SimulationItem from the scene
    foreach (QGraphicsItem *item, m_scene->items()) {
        // Don't remove other items (ie: mouse tracker lines or
        // eraser rectancgle) than the type SimulationItem
        if (!(dynamic_cast<SimulationItem*>(item))) {
            continue;
        }

        m_scene->removeItem(item);
        delete item;
    }
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

    //////////////////// Keyboard controls of the scene view ////////////////////
    else if (e->key() == Qt::Key_Left) {
        moveSceneView(QPointF(-10, 0));
    }
    else if (e->key() == Qt::Key_Right) {
        moveSceneView(QPointF(10, 0));
    }
    else if (e->key() == Qt::Key_Up) {
        moveSceneView(QPointF(0, -10));
    }
    else if (e->key() == Qt::Key_Down) {
        moveSceneView(QPointF(0, 10));
    }
    /////////////////////////////////////////////////////////////////////////////
}

/**
 * @brief MainWindow::graphicsSceneWheelEvent
 * @param pos
 * @param delta
 * @param mod_keys
 *
 * Slot called when the user use the mouse wheel.
 * It is used to zoom in/out the scene.
 */
void MainWindow::graphicsSceneWheelEvent(QGraphicsSceneWheelEvent *event) {
    qreal scale_factor = 1.0 - event->delta() / 5000.0;
    scaleView(scale_factor, event->scenePos());
}

/**
 * @brief MainWindow::graphicsSceneDoubleClicked
 * @param event
 *
 * Slot called when the user double click on the graphics scene
 */
void MainWindow::graphicsSceneDoubleClicked(QGraphicsSceneMouseEvent *event) {
    // If we are placing something -> nothing to do
    if (m_drawing_item != nullptr) {
        return;
    }

    // Loop over the items under the mouse position
    foreach(QGraphicsItem *item, m_scene->items(event->scenePos())) {
        // Try to cast this item
        Emitter *em = dynamic_cast<Emitter*>(item);

        // If one of them is an Emitter -> configure it
        if (em != nullptr) {
            configureEmitter(em);
        }
    }
}

/**
 * @brief MainWindow::graphicsSceneRightReleased
 *
 * Slot called when the user releases the right button on the graphics scene
 */
void MainWindow::graphicsSceneRightReleased(QGraphicsSceneMouseEvent *) {
    // Right click = cancel the current action
    cancelCurrentDrawing();
}

/**
 * @brief MainWindow::graphicsSceneLeftReleased
 * @param pos
 *
 * Slot called when the user presses the left button on the graphics scene
 */
void MainWindow::graphicsSceneLeftPressed(QGraphicsSceneMouseEvent *event) {
    Q_UNUSED(event);

    // If no draw action pending -> start view dragging
    if (m_draw_action == DrawActions::None) {
        m_dragging_view = true;
        ui->graphicsView->setCursor(Qt::ClosedHandCursor);
    }
}

/**
 * @brief MainWindow::graphicsSceneLeftReleased
 * @param pos
 *
 * Slot called when the user releases the left button on the graphics scene
 */
void MainWindow::graphicsSceneLeftReleased(QGraphicsSceneMouseEvent *event) {
    QPoint pos = event->scenePos().toPoint();

    // End the dragging action when the mouse is released
    if (m_dragging_view) {
        m_dragging_view = false;
        ui->graphicsView->setCursor(Qt::ArrowCursor);
    }

    // If we aren't placing something yet
    if (m_drawing_item == nullptr) {

        // Actions to do on the first click
        switch (m_draw_action) {
        //////////////////////////////// WALLS ACTIONS (1st click) /////////////////////////////////
        case DrawActions::BrickWall: {
            // Add a brick wall to the scene
            pos = attractivePoint(pos);
            QLine line(pos, pos);
            BrickWall *wall = new BrickWall(line);
            wall->setPlacingMode(true);
            m_drawing_item = wall;
            m_scene->addItem(m_drawing_item);
            break;
        }
        case DrawActions::ConcreteWall: {
            // Add a concrete wall to the scene
            pos = attractivePoint(pos);
            QLine line(pos, pos);
            ConcreteWall *wall = new ConcreteWall(line);
            wall->setPlacingMode(true);
            m_drawing_item = wall;
            m_scene->addItem(m_drawing_item);
            break;
        }
        case DrawActions::PartitionWall: {
            // Add a partition wall to the scene
            pos = attractivePoint(pos);
            QLine line(pos, pos);
            PartitionWall *wall = new PartitionWall(line);
            wall->setPlacingMode(true);
            m_drawing_item = wall;
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

            // If the wall has a null length, don't place it
            if (wall->getLine().length() == 0.0) {
                break;
            }

            // Disable the placing mode
            wall->setPlacingMode(false);

            // Add the new Wall to the walls list in the simulation data
            m_simulation_handler->simulationData()->attachWall(wall);

            // Detach the drawn wall from the mouse
            m_drawing_item = nullptr;

            // Repeat the last action if the control or shift key was pressed
            if (event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier)) {
                // Simulate a click on the same place, so we start a new wall of the
                // same type at the end of the previous one
                graphicsSceneLeftReleased(event);
            }
            else {
                m_draw_action = DrawActions::None;
            }
            break;
        }
        //////////////////////////////// EMITTER ACTION /////////////////////////////////
        case DrawActions::Emitter: {
            Emitter *emitter = (Emitter*) m_drawing_item;

            // Add this emitter to the simulation data
            m_simulation_handler->simulationData()->attachEmitter(emitter);

            // Repeat the last action if the control or shift key was pressed
            if (event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier)) {
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

            // Add this receiver to the simulation data
            m_simulation_handler->simulationData()->attachReceiver(receiver);

            // Repeat the last action if the control or shift key was pressed
            if (event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier)) {
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
                // Don't remove other items (ie: mouse tracker lines or
                // eraser rectancgle) than the type SimulationItem
                if (!(dynamic_cast<SimulationItem*>(item))) {
                    continue;
                }

                // Remove the item from the scene
                m_scene->removeItem(item);

                // Action for some types of items
                if (dynamic_cast<Wall*>(item)) {
                    // Remove it from the walls list
                    m_simulation_handler->simulationData()->detachWall((Wall*) item);
                }
                else if (dynamic_cast<Emitter*>(item)){
                    m_simulation_handler->simulationData()->detachEmitter((Emitter*) item);
                }
                else if (dynamic_cast<Receiver*>(item)){
                    m_simulation_handler->simulationData()->detachReceiver((Receiver*) item);
                }

                delete item;
            }
            break;
        }
        default:
            break;
        }
    }

    // Show mouse tracker only if we are placing something
    setMouseTrackerVisible(m_draw_action != DrawActions::None);
}

/**
 * @brief MainWindow::graphicsSceneMouseMoved
 * @param pos
 *
 * Slot called when the mouse move over the graphics scene
 */
void MainWindow::graphicsSceneMouseMoved(QGraphicsSceneMouseEvent *event) {
    QPoint pos = event->scenePos().toPoint();

    // Move the scene to follow the drag movement of the mouse.
    // Use the screenPos that is invariant of the sceneRect.
    if (m_dragging_view) {
        qreal view_scale = ui->graphicsView->transform().m11();
        QPointF delta_mouse = event->lastScreenPos() - event->screenPos();
        moveSceneView(delta_mouse / view_scale);
    }

    // Show mouse tracker only if we are placing something
    setMouseTrackerVisible(m_draw_action != DrawActions::None);

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
        QLine line = wall_item->getLine().toLine();

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
    if (abs(delta.x()) < ALIGN_THRESHOLD && abs(delta.x()) < abs(delta.y())) {
        end.setX(start.x());
    }

    // Align by Y if we are close to the starting Y position
    else if (abs(delta.y()) < ALIGN_THRESHOLD) {
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
            line_points << wall_item->getLine().p1() << wall_item->getLine().p2();

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

///////////////////////////////////// MOUSE TRACKER SECTION ////////////////////////////////////////

void MainWindow::initMouseTracker() {
    // Add two lines to the scene that will track the mouse cursor when visible
    QPen tracker_pen(QBrush(QColor(0, 0, 255, 100)), 1.0 * devicePixelRatioF(), Qt::DotLine);
    tracker_pen.setCosmetic(true);  // Keep the same pen width even if the view is scaled

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

    // Hide the mouse cursor when we use the mouse tracker lines
    if (visible) {
        ui->graphicsView->setCursor(Qt::BlankCursor);
    }
    else if (ui->graphicsView->cursor() == Qt::BlankCursor) {
        ui->graphicsView->setCursor(Qt::ArrowCursor);
    }
}

void MainWindow::setMouseTrackerPosition(QPoint pos) {
    // Get the viewport dimensions
    QGraphicsView *view = ui->graphicsView;

    QLine x_line(pos.x(), view->sceneRect().y(),
                 pos.x(), view->sceneRect().y() + view->sceneRect().height()-1);

    QLine y_line(view->sceneRect().x(), pos.y(),
                 view->sceneRect().x() + view->sceneRect().width()-1, pos.y());

    m_mouse_tracker_x->setLine(x_line);
    m_mouse_tracker_y->setLine(y_line);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////// FILE SAVE/RESTORE HANDLING SECTION /////////////////////////////////

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
       QMessageBox::critical(this, "Erreur", "Impossible d'ouvrir le fichier en lecture");
       return;
   }

   // Clear all the current data
   m_simulation_handler->resetComputedData();
   clearAllItems();

   // Read data from the file
   QDataStream in(&file);
   in >> m_simulation_handler->simulationData();

   // Update the graphics scene with read data
   foreach (Wall* w, m_simulation_handler->simulationData()->getWallsList()) {
       m_scene->addItem(w);
   }
   foreach (Emitter* e, m_simulation_handler->simulationData()->getEmittersList()) {
       m_scene->addItem(e);
   }
   foreach (Receiver* r, m_simulation_handler->simulationData()->getReceiverList()) {
       m_scene->addItem(r);
   }

   updateSimulationUI();
   updateSimulationScene();

   // Go to the edit mode if there is no emitter in the scene
   if (m_simulation_handler->simulationData()->getEmittersList().size() < 1) {
       switchEditSceneMode();
   }

   // Close the file
   file.close();

   // Reset the view after opening the file
   resetView();
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
        QMessageBox::critical(this, "Erreur", "Impossible d'ouvrir le fichier en écriture");
        return;
    }

    // Write current data into the file
    QDataStream out (&file);
    out << m_simulation_handler->simulationData();

    // Close the file
    file.close();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////// ZOOM ACTIONS FUNCTIONS ///////////////////////////////////////

void MainWindow::actionZoomIn() {
    scaleView(1.1);
}

void MainWindow::actionZoomOut() {
    scaleView(0.9);
}

void MainWindow::actionZoomReset() {
    resetView();
}

void MainWindow::actionZoomBest() {
    bestView();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////// MODE SWITCHING FUNCTIONS //////////////////////////////////////

void MainWindow::switchSimulationMode() {
    if (m_ui_mode == UIMode::SimulationMode)
        return;

    // If there is no emitter in the simulation
    if (m_simulation_handler->simulationData()->getEmittersList().size() < 1) {
        QMessageBox::information(this, "Simulation", "Vous devez inclure au moins un émetteur pour effectuer une simulation");
        switchEditSceneMode();
        return;
    }

    // Set the current mode to SimulationMode
    m_ui_mode = UIMode::SimulationMode;

    // Hide the scene edition buttons group
    ui->group_scene_edition->hide();

    // Show the simulation buttons group
    ui->group_simulation->show();

    // Disable the Edit menu (from menu bar)
    ui->menuEdit->setDisabled(true);

    // Cancel the current drawing (if one)
    cancelCurrentDrawing();

    // Update the simulation scene and UI according to new mode
    updateSimulationScene();
    updateSimulationUI();

    // Update the scene rect (since the view size can have changed)
    updateSceneRect();
}

void MainWindow::switchEditSceneMode() {
    if (m_ui_mode == UIMode::EditorMode)
        return;

    // This will reset the simulation data
    bool ans = askSimulationReset();

    // Don't continue if user refused
    if (!ans)
        return;

    // Set the current mode to EditorMode
    m_ui_mode = UIMode::EditorMode;

    // Hide the simulation buttons group
    ui->group_simulation->hide();

    // Show the scene edition buttons group
    ui->group_scene_edition->show();

    // Enable the Edit menu (from menu bar)
    ui->menuEdit->setDisabled(false);

    // Update the simulation scene and UI according to new mode
    updateSimulationScene();
    updateSimulationUI();

    // Update the scene rect (since the view size can have changed)
    updateSceneRect();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////// SIMULATION ACTIONS FUNCTIONS ////////////////////////////////////

void MainWindow::updateSimulationUI() {
    // Set the initial value of the label according to his slider
    raysThresholdChanged(ui->slider_threshold->value());

    // Set the current simulation type
    ui->combobox_simType->setCurrentIndex(m_simulation_handler->simulationData()->simulationType());

    // Set the current reflections count
    ui->spinbox_reflections->setValue(m_simulation_handler->simulationData()->maxReflectionsCount());

    if (!m_simulation_handler->isRunning()) {
        // Hide the progress bar
        ui->progressbar_simulation->hide();
    }

    // Update the simulation type
    simulationTypeChanged();
}

void MainWindow::updateSimulationScene() {
    // Hide receivers only if simulation mode and AreaReceiver simulation type
    if (m_ui_mode == UIMode::EditorMode) {
        setPointReceiversVisible(true);
        setSimAreaVisible(false);
    }
    else if (m_simulation_handler->simulationData()->simulationType() == SimType::PointReceiver) {
        setPointReceiversVisible(true);
        setSimAreaVisible(false);
    }
    else {
        setPointReceiversVisible(false);
        setSimAreaVisible(true);
    }

    filterRaysThreshold();
}

void MainWindow::simulationTypeChanged() {
    // Retreive the selected simulation type
    SimType::SimType sim_type = (SimType::SimType) ui->combobox_simType->currentIndex();

    // Nothing to do if no difference with current type
    if (sim_type == m_simulation_handler->simulationData()->simulationType()) {
        return;
    }

    // This will reset the data -> prevent user
    bool ans = askSimulationReset();

    if (!ans) {
        // Go back to the current type if user refused
        ui->combobox_simType->setCurrentIndex(m_simulation_handler->simulationData()->simulationType());
        return;
    }

    // Set the current simulation type into simulation data
    m_simulation_handler->simulationData()->setSimulationType(sim_type);

    switch (sim_type) {
    case SimType::PointReceiver: {
        switchPointReceiverMode();
        break;
    }
    case SimType::AreaReceiver: {
        switchAreaReceiverMode();
        break;
    }
    }

    updateSimulationScene();
}

void MainWindow::switchPointReceiverMode() {
    // We can show the rays in point receiver mode
    ui->checkbox_rays->setEnabled(true);
    raysCheckboxToggled(ui->checkbox_rays->isChecked());
}

void MainWindow::switchAreaReceiverMode() {
    // Don't show the rays in area mode
    ui->checkbox_rays->setEnabled(false);
    raysCheckboxToggled(false);
}

void MainWindow::simulationControlAction() {
    // If there is no simulation computation currently running
    if (!m_simulation_handler->isRunning())
    {
        // Start the computation for the current simulation type
        switch (m_simulation_handler->simulationData()->simulationType())
        {
        case SimType::PointReceiver: {
            QList<Receiver*> rcv_list = m_simulation_handler->simulationData()->getReceiverList();
            m_simulation_handler->startSimulationComputation(rcv_list);
            break;
        }
        case SimType::AreaReceiver: {
            // If there is no simulation area
            if (m_sim_area_item == nullptr) {
                // This wouldn't happen
                return;
            }

            m_simulation_handler->startSimulationComputation(m_sim_area_item->getReceiversList());
            break;
        }
        }
    }
    else {
        // Cancel the current simulation
        m_simulation_handler->stopSimulationComputation();

        // Change the UI control button
        ui->button_simControl->setText("Arrêt en cours...");
        ui->button_simControl->setEnabled(false);
    }
}

void MainWindow::simulationStarted() {
    // Disable the UI controls
    ui->combobox_simType->setEnabled(false);
    ui->spinbox_reflections->setEnabled(false);
    ui->button_simReset->setEnabled(false);
    ui->button_editScene->setEnabled(false);
    ui->actionOpen->setEnabled(false);

    // Change the control button text
    ui->button_simControl->setText("Arrêter la simulation");
    ui->button_simControl->setEnabled(true);

    // Show the progress bar
    ui->progressbar_simulation->show();
}

void MainWindow::simulationFinished() {
    // Enable the UI controls
    ui->combobox_simType->setEnabled(true);
    ui->spinbox_reflections->setEnabled(true);
    ui->button_simReset->setEnabled(true);
    ui->button_editScene->setEnabled(true);
    ui->actionOpen->setEnabled(true);

    // Change the control button text
    ui->button_simControl->setText("Démarrer la simulation");
    ui->button_simControl->setEnabled(true);

    // Hide the progress bar
    ui->progressbar_simulation->hide();

    // Don't add the ray paths is this is a AreaReceivers simulations
    if (m_simulation_handler->simulationData()->simulationType() == SimType::PointReceiver) {
        // Add all computed rays to the scene
        foreach (RayPath *rp, m_simulation_handler->getRayPathsList()) {
            m_scene->addItem(rp);
        }
    }

    // Filter the rays to show
    filterRaysThreshold();

    //TODO: what to do with this ?
    m_simulation_handler->generateReceiversTooltip();
}

void MainWindow::simulationCancelled() {
    // Enable the UI controls
    ui->combobox_simType->setEnabled(true);
    ui->spinbox_reflections->setEnabled(true);
    ui->button_simReset->setEnabled(true);
    ui->button_editScene->setEnabled(true);
    ui->actionOpen->setEnabled(true);

    // Change the control button text
    ui->button_simControl->setText("Démarrer la simulation");
    ui->button_simControl->setEnabled(true);

    // Hide the progress bar
    ui->progressbar_simulation->hide();

    // Reset the simulations
    m_simulation_handler->resetComputedData();
}

void MainWindow::simulationProgress(double p) {
    // Update the progress bar's value
    ui->progressbar_simulation->setValue(p * 100);
}

void MainWindow::simulationResetAction() {
    askSimulationReset();
}

bool MainWindow::askSimulationReset() {
    int ans = QMessageBox::question(
                this,
                "Réinitialiser la simulation",
                "Voulez-vous vraiment effacer les données de la simulation ?\n"
                "Le plan ne sera pas affecté.");

    // If the user cancelled the action -> abort
    if (ans == QMessageBox::No)
        return false;

    // Reset the computation data
    m_simulation_handler->resetComputedData();

    return true;
}

void MainWindow::raysCheckboxToggled(bool state) {
    // Update the UI
    ui->label_threshold_msg->setEnabled(state);
    ui->label_threshold_val->setEnabled(state);
    ui->slider_threshold->setEnabled(state);

    // Apply the filter to hide/show the rays
    filterRaysThreshold();
}

void MainWindow::raysThresholdChanged(int val) {
    // Set the width of the label to the size of the larger text (-200 dBm)
    if (ui->label_threshold_val->minimumWidth() == 0) {
        ui->label_threshold_val->setText("-200 dBm");
        ui->label_threshold_val->setFixedWidth(ui->label_threshold_val->sizeHint().width());
    }

    // Set the text of the label according to slider
    ui->label_threshold_val->setText(QString("%1 dBm").arg(val));

    // Filter the rays to show
    filterRaysThreshold();
}

void MainWindow::filterRaysThreshold() {
    // Convert the threshold in Watts
    const double threshold = SimulationData::convertPowerToWatts(ui->slider_threshold->value());

    // Loop over the RayPaths
    foreach (RayPath *rp, m_simulation_handler->getRayPathsList())
    {
        // Hide the RayPaths with a power lower than the threshold, or checkbox not checked, or
        // UI is not in simulation mode, or simulation type is area
        if (rp->getPower() > threshold &&
                ui->checkbox_rays->isChecked() &&
                m_ui_mode == UIMode::SimulationMode &&
                m_simulation_handler->simulationData()->simulationType() == SimType::PointReceiver)
        {
            rp->show();
        }
        else {
            rp->hide();
        }
    }
}

void MainWindow::setPointReceiversVisible(bool visible) {
    foreach(Receiver *r, m_simulation_handler->simulationData()->getReceiverList()) {
        r->setVisible(visible);
    }
}

void MainWindow::setSimAreaVisible(bool visible) {
    if (visible)
    {
        // Get the simulation bounding rect
        QRectF area = m_scene->simulationBoundingRect();

        if (m_sim_area_item == nullptr) {
            // Create the area rectangle
            m_sim_area_item = new ReceiversArea();
            m_scene->addItem((SimulationItem*) m_sim_area_item);
        }

        // Re-draw the simulation area
        m_sim_area_item->setArea(area);
    }
    else if (!visible && m_sim_area_item != nullptr)
    {
        // Be sure the simulation is resetted
        m_simulation_handler->resetComputedData();

        // Remove the simulation area
        delete m_sim_area_item;
        m_sim_area_item = nullptr;
    }
}

void MainWindow::exportSimulationAction() {
    // Cancel the (potential) current drawing
    cancelCurrentDrawing();

    QString file_path = QFileDialog::getSaveFileName(this, "Exporter la simulation", QString(), "*.png");

    // If the used cancelled the dialog
    if (file_path.isEmpty()) {
        return;
    }

    // If the file hasn't the .png extention -> add it
    if (file_path.split('.').last() != "png") {
        file_path.append(".png");
    }

    QFile file(file_path);
    // Open the file (writing)
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Erreur", "Impossible d'ouvrir le fichier en écriture");
        return;
    }

    // Prepare an image with the double resolution of the scene
    QImage image(ui->graphicsView->sceneRect().size().toSize()*2, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    // Paint the scene into the image
    QPainter painter(&image);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    m_scene->render(&painter, QRectF(), ui->graphicsView->sceneRect());

    // Write the exported image into the file
    image.save(&file, "png");

    // Close the file
    file.close();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
