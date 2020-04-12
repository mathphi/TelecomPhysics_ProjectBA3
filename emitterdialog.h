#ifndef EMITTERDIALOG_H
#define EMITTERDIALOG_H

#include <QDialog>

#include "emitters.h"

namespace Ui {
class EmitterDialog;
}

class EmitterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EmitterDialog(QWidget *parent = nullptr);
    ~EmitterDialog();

    EmitterType::EmitterType getEmitterType();
    double getPower();
    double getFrequency();
    double getResistance();
    double getEfficiency();

protected:
    virtual void keyPressEvent(QKeyEvent *event);

private slots:
    void powerSpinboxChanged(double value);

private:
    Ui::EmitterDialog *ui;
};

#endif // EMITTERDIALOG_H