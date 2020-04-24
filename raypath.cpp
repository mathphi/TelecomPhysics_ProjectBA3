#include "raypath.h"

RayPath::RayPath(QList<QLineF> rays, complex<double> elec_field)
{
    m_rays = rays;
    m_elec_field = elec_field;

}
complex<double>RayPath::getElecField(){
    return m_elec_field;
}
