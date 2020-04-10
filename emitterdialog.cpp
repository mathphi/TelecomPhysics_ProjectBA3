#include "emitterdialog.h"
#include "ui_emitterdialog.h"

#include <QKeyEvent>

EmitterDialog::EmitterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EmitterDialog)
{
    ui->setupUi(this);

    // Disable the help button on title bar
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    // Add items to the antenna type combobox
    ui->combobox_antenna_type->addItem("Dipôle λ/2", EmitterType::HalfWaveDipole);

    connect(ui->spinbox_power, SIGNAL(valueChanged(double)), this, SLOT(powerSpinboxChanged(double)));
}

EmitterDialog::~EmitterDialog()
{
    delete ui;
}

void EmitterDialog::keyPressEvent(QKeyEvent *event) {
    // Prevent dialog closing on enter key press
    if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        return;

    QDialog::keyPressEvent(event);
}

EmitterType::EmitterType EmitterDialog::getEmitterType() {
    return (EmitterType::EmitterType) ui->combobox_antenna_type->currentData().toInt();
}

double EmitterDialog::getPower() {
    return Emitter::convertPowerToWatts(ui->spinbox_power->value());
}

double EmitterDialog::getFrequency() {
    return ui->spinbox_frequency->value();
}

double EmitterDialog::getResistance() {
    return ui->spinbox_resistance->value();
}

double EmitterDialog::getEfficiency() {
    return ui->spinbox_efficiency->value();
}

/**
 * @brief EmitterDialog::powerSpinboxChanged
 * @param value
 *
 * This slots update the label to show the converted power in watts
 * from the given value in dBm
 */
void EmitterDialog::powerSpinboxChanged(double value) {
    QString suffix = "W";

    double power_watts = Emitter::convertPowerToWatts(value);

    // Convert to readable units and values
    if (power_watts < 1e-3) {
        suffix = "nW";
        power_watts *= 1e6;
    }
    else if (power_watts < 1) {
        suffix = "mW";
        power_watts *= 1e3;
    }

    ui->label_power_watts->setText(QString("= %1 %2").arg(power_watts, 0, 'f', 2).arg(suffix));
}
