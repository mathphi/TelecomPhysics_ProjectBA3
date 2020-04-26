#ifndef RAYPATH_H
#define RAYPATH_H

#include "simulationitem.h"
#include "constants.h"

class RayPath : public SimulationItem
{
public:
    RayPath(QList<QLineF> rays, complex<double> elec_field);

    complex<double> getElecField();
    QList<QLineF> getRays();

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

private:
    QLineF getScaledLine(QLineF r) const;

    QList<QLineF> m_rays;
    complex<double> m_elec_field;

    qreal m_sim_scale;
};

#endif // RAYPATH_H
