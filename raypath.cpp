#include "raypath.h"
#include "simulationscene.h"

#include <QPainter>

#define PEN_WIDTH 1

RayPath::RayPath(QList<QLineF> rays, complex<double> elec_field) : SimulationItem()
{
    m_rays = rays;
    m_elec_field = elec_field;

    m_sim_scale = simulationScene()->simulationScale();
}

complex<double> RayPath::getElecField() {
    return m_elec_field;
}

QList<QLineF> RayPath::getRays() {
    return m_rays;
}

QLineF RayPath::getScaledLine(QLineF r) const {
    return QLineF(r.p1() * m_sim_scale, r.p2() * m_sim_scale);
}

QRectF RayPath::boundingRect() const {
    // Return the rectangle containing all the ray lines
    return shape().controlPointRect();
}

QPainterPath RayPath::shape() const {
    QPainterPath path;

    // Each ray is a line
    foreach (QLineF ray, m_rays) {
        QLineF l = getScaledLine(ray);
        path.moveTo(l.p1());
        path.lineTo(l.p2());
    }

    // Take care of the width of the pen
    QPainterPathStroker ps;
    ps.setWidth(PEN_WIDTH);

    QPainterPath p = ps.createStroke(path);
    p.addPath(path);
    return p;
}

void RayPath::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    // Draw each ray as a line
    for (int i = 0 ; i < m_rays.size() ; i++) {
        QLineF ray = m_rays[i];

        //painter->setPen(m_pen);
        painter->drawLine(getScaledLine(ray));
    }
}
