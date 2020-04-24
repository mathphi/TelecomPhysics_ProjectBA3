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
    complex<double> coeff = 1;
    double dn;
    Wall *target_wall = nullptr;

    for(int i = images.size()-1; i >= 0 ; i--){
        Wall *reflect_wall = walls[i];
        QPointF src_image = images[i];

        QLineF virtual_ray (src_image, target_point);
        if (i == images.size()-1){
            dn = virtual_ray.length();
        }
        QPointF reflection_pt;
        QLineF::IntersectionType i_t = virtual_ray.intersects(reflect_wall->getRealLine(),&reflection_pt);

        if(i_t != QLineF::BoundedIntersection){
            return;
        }

        QLineF ray(reflection_pt, target_point);
        rays.append(ray);

        coeff *= computeReflexion(emitter, reflect_wall, ray);
        coeff*= computeTransmissons(emitter, ray, reflect_wall, target_wall );

        target_point = reflection_pt;
        target_wall = reflect_wall;

    }

    QLineF ray(emitter->getRealPos(), target_point);
    rays.append(ray);
    coeff*= computeTransmissons(emitter, ray);

    if(images.size() == 0){
        dn = ray.length();
    }
    complex<double> En = coeff*computeNominal_elec_field(emitter, ray, dn);
    qDebug()<<dn<<norm(coeff);

    RayPath *rp = new RayPath(rays, En);
    m_raypaths.append(rp);
    foreach(QLineF r, rays) {
        m_simulation_scene->addLine(QLineF(r.p1()* scale, r.p2()*scale));
    }
}

void SimulationHandler::computeAllRays() {
    QElapsedTimer tm;
    tm.start();
    //TODO suppress all objects in m_raypaths
    m_raypaths.clear();

    foreach(Receiver *r , simulationData()->getReceiverList()) {
        foreach(Emitter *e, simulationData()->getEmittersList()) {


            computeRayPath(e, r);

            foreach(Wall *w, simulationData()->getWallsList()) {
                recursiveReflection(e,r,w);
            }

            double power = computePower(e, m_raypaths);
            qDebug()<<power ;
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

complex<double> SimulationHandler::computeReflexion(Emitter *e, Wall *w, QLineF ray_in){
    double theta_i = M_PI/2.0 - w->getRealLine().angleTo(ray_in)/180.0*M_PI;
    if(theta_i > M_PI/2){
       theta_i = abs(theta_i - M_PI);
    }
    double omega = e->getFrequency()*2*M_PI;
    complex<double> epsilon_tilde = complexPermittivity(w->getRelPermitivity(), w->getConductivity(), omega );//rel grave ?
    complex<double> Z1 = Z_AIR;
    complex<double> Z2 = characteristicImpedance(epsilon_tilde);

    double theta_t = asin(real(Z2/Z1)*sin(theta_i));

    complex<double>gamma_orth = (Z2*cos(theta_i) - Z1*cos(theta_t)) / (Z2*cos(theta_i)+ Z1 * cos(theta_t));
    double s = w->getThickness()/cos(theta_t);

    complex<double> gamma_m = propagationConstant(omega, epsilon_tilde);
    complex<double> gamma_0 = propagationConstant(omega, EPSILON_0);
    complex<double> reflection = gamma_orth + (1.0 - pow(gamma_orth,2.0)) * gamma_orth*exp(-2.0* gamma_m*s  + gamma_0*2.0*s*sin(theta_t)*sin(theta_i))/(1.0 - pow(gamma_orth,2.0)*exp(-2.0* gamma_m*s  + gamma_0*2.0*s*sin(theta_t)*sin(theta_i)));
    return reflection;
}
complex<double> SimulationHandler::computeNominal_elec_field(Emitter *e, QLineF ray, double dn){
    double GTX = e->getGain(M_PI, 0);
            //TODO coompute thetaTX, phi
    double PTX = e->getPower();
    double omega = e->getFrequency()*2*M_PI;
    complex<double> gamma_0 = propagationConstant(omega, EPSILON_0);
    return sqrt(60*GTX*PTX)*exp(-gamma_0*dn)/dn;

}

complex<double> SimulationHandler::computeTransmissons(Emitter *e, QLineF ray, Wall *origin_wall, Wall *target_wall){
    double omega = e->getFrequency()*2*M_PI;
    complex<double> gamma_0 = propagationConstant(omega, EPSILON_0);

    complex<double> coeff = 1;
    foreach(Wall *w,simulationData()->getWallsList()){
        if(w == origin_wall || w == target_wall){
            continue;
        }

        QPointF pt;
        QLineF::IntersectionType i_t = ray.intersects(w->getRealLine(),&pt);

        if(i_t != QLineF::BoundedIntersection){
            continue;
        }
        complex<double> epsilon_tilde = complexPermittivity(w->getRelPermitivity(), w->getConductivity(), omega );
        complex<double> Z1 = Z_AIR;
        complex<double> Z2 = characteristicImpedance(epsilon_tilde);

        double theta_i = M_PI/2.0 - w->getRealLine().angleTo(ray)/180.0*M_PI;
        if(theta_i > M_PI/2){
            theta_i = abs(theta_i - M_PI);
        }
        double theta_t = asin(real(Z2/Z1)*sin(theta_i));

        complex<double>gamma_orth = (Z2*cos(theta_i) - Z1*cos(theta_t)) / (Z2*cos(theta_i)+ Z1 * cos(theta_t));
        double s = w->getThickness()/cos(theta_t);

        complex<double> gamma_m = propagationConstant(omega, epsilon_tilde);

        complex<double> transmission = (1.0-pow(gamma_orth,2.0))*exp(-gamma_m*s)/(1.0-pow(gamma_orth,2.0)*exp(-2.0*gamma_m*s + gamma_0*2.0*s*sin(theta_t)*sin(theta_i)));
        coeff *= transmission;
    }
    return coeff;



}

double SimulationHandler::computePower(Emitter *e, QList<RayPath *> rp_list){
    double Ra = e->getResistance();
    double theta = M_PI_2;
    double Prx = 0;

    foreach(RayPath* rp, rp_list){
        double phi = 0;
        //TODO phi = incidence angle emiter
        complex<double> he = e->getEffectiveHeight(theta,phi);
        //norm = square of modulus
        Prx += norm(he * rp->getElecField());

    }
    Prx *= 1.0/(8.0*Ra);
    return Prx;
}
