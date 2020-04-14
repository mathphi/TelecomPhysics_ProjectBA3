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

void SimulationHandler::recursiveCompute(int level, QPoint emitter_pos, QPoint receiver_pos, Wall *wall){
    QPointF image = mirror(emitter_pos, wall);
    QLineF virtual_ray (image, receiver_pos);
    QPointF th_reflection_pt;
    QLineF::IntersectionType i_t = virtual_ray.intersects(wall->getLine(),&th_reflection_pt);
    if(i_t != QLineF::BoundedIntersection){
        return;
    }
    double dn = virtual_ray.length();
}

void SimulationHandler::recursiveReflection(Emitter *emitter,
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
   if(level < 2){
       foreach (Wall *w, simulationData()->getWallsList()){
           if(w == reflect_wall){
               continue;
           }
           recursiveReflection(emitter, receiver, w, images, walls, level+1);
       }
   }
}

void SimulationHandler::computeRayPath(Emitter *emitter, Receiver *receiver, QList<QPointF> images, QList<Wall*> walls){
    QPointF target_point = receiver->getRealPos();

    double scale = m_simulation_scene->simulationScale(); //TODO get rid of this

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
        //TODO check if transmission
        target_point = reflection_pt;
        m_simulation_scene->addLine(QLineF(ray.p1()* scale,ray.p2()*scale));

    }
    QLineF ray(emitter->getRealPos(), target_point);
    //TODO check if transmission
    m_simulation_scene->addLine(QLineF(ray.p1()* scale,ray.p2()*scale));
}

void SimulationHandler::computeAllRays(){
    QElapsedTimer tm;
    tm.start();

    foreach(Emitter *e, simulationData()->getEmittersList()){
        foreach(Receiver *r , simulationData()->getReceiverList()){
            computeRayPath(e, r);
            foreach(Wall *w, simulationData()->getWallsList()){
                recursiveReflection(e,r,w);
            }
        }
    }
    qDebug()<<tm.nsecsElapsed();
}
