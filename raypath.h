#ifndef RAYPATH_H
#define RAYPATH_H

#include <QGraphicsItem>

#include "constants.h"

class RayPath
{
public:
    RayPath(QList<QLineF> rays, complex<double> elec_field);
    complex<double> getElecField();

private:
    QList<QLineF> m_rays;
    complex<double> m_elec_field;

};

#endif // RAYPATH_H
