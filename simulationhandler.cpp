#include "simulationhandler.h"

#include <QDebug>
#include <QElapsedTimer>

SimulationHandler::SimulationHandler(SimulationScene *scene)
{
    m_simulation_data = new SimulationData();
    m_simulation_scene = scene;
}

SimulationData *SimulationHandler::simulationData() {
    return m_simulation_data;
}

/**
 * @brief SimulationHandler::mirror
 * @param source
 * @param wall
 * @return
 *
 * This function returns the coordinates of the "source" point
 * after an axial symmetry through the wall.
 *
 * WARNING: the y axis is upside down in the graphics scene !
 */
QPointF SimulationHandler::mirror(QPointF source, Wall *wall) {
    // Get the angle of the wall to the horizontal axis
    double theta = wall->getRealLine().angle() * M_PI / 180.0 - M_PI / 2.0;

    // Translate the origin of the coordinates system on the base of the wall
    double x = source.x() - wall->getRealLine().p1().x();
    double y = source.y() - wall->getRealLine().p1().y();

    // We use a rotation matrix :
    //   x' =  x*cos(θ) - y*sin(θ)
    //   y' = -x*sin(θ) - y*cos(θ)
    // to set the y' axis along the wall, so the image is at the opposite of the x' coordinate
    // in the new coordinates system.
    double x_p =  x*cos(theta) - y*sin(theta);
    double y_p = -x*sin(theta) - y*cos(theta);

    // Rotate and translate back to the original coordinates system
    return QPointF(-x_p*cos(theta) - y_p*sin(theta), x_p*sin(theta) - y_p*cos(theta)) + wall->getRealLine().p1();
}

void SimulationHandler::recursiveReflection(
        Emitter *emitter,
        Receiver *receiver,
        Wall *reflect_wall,
        QList<QPointF> images,
        QList<Wall *> walls,
        int level)
{
   QPointF src_image;

   if(images.size() == 0){
       src_image = mirror(emitter->getRealPos(),reflect_wall);
   }
   else{
       src_image = mirror(images.last(),reflect_wall);
   }

   images.append(src_image);
   walls.append(reflect_wall);

   computeRayPath(emitter, receiver, images, walls);

   if(level < 3) {
       foreach (Wall *w, simulationData()->getWallsList()){

           if(w == reflect_wall){
               continue;
           }

           recursiveReflection(emitter, receiver, w, images, walls, level+1);
       }
   }
}

void SimulationHandler::computeRayPath(
        Emitter *emitter,
        Receiver *receiver,
        QList<QPointF> images,
        QList<Wall*> walls)
{
    QPointF target_point = receiver->getRealPos();

    double scale = m_simulation_scene->simulationScale(); //TODO get rid of this

    QList<QLineF> rays;

    for(int i = images.size()-1; i >= 0 ; i--){
        Wall *reflect_wall = walls[i];
        QPointF src_image = images[i];

        QLineF virtual_ray (src_image, target_point);

        QPointF reflection_pt;
        QLineF::IntersectionType i_t = virtual_ray.intersects(reflect_wall->getRealLine(),&reflection_pt);

        if(i_t != QLineF::BoundedIntersection){
            return;
        }

        QLineF ray(reflection_pt, target_point);
        rays.append(ray);

        //TODO check if transmission
        target_point = reflection_pt;

    }
    QLineF ray(emitter->getRealPos(), target_point);
    rays.append(ray);
    //TODO check if transmission

    foreach(QLineF r, rays) {
        m_simulation_scene->addLine(QLineF(r.p1()* scale, r.p2()*scale));
    }
}

void SimulationHandler::computeAllRays() {
    QElapsedTimer tm;
    tm.start();

    foreach(Emitter *e, simulationData()->getEmittersList()) {
        foreach(Receiver *r , simulationData()->getReceiverList()) {

            computeRayPath(e, r);

            foreach(Wall *w, simulationData()->getWallsList()) {
                recursiveReflection(e,r,w);
            }
        }
    }

    qDebug() << tm.nsecsElapsed();
}

complex<double> SimulationHandler::complexPermittivity(double e_r, double sigma, double omega) {
    return e_r*EPSILON_0 - 1i*sigma/omega;
}
complex<double> SimulationHandler::characteristicImpedance(complex<double> e) {
    return sqrt(MU_0/e);
}
complex<double> SimulationHandler::propagationConstant(double omega, complex<double> e) {
    return 1i*omega*sqrt(MU_0*e);
}
double SimulationHandler::air_nb_wave(double omega) {
    return omega*sqrt(MU_0*EPSILON_0);
}
