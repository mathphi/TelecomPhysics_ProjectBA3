#include "simulationhandler.h"

#include <QDebug>
#include <QElapsedTimer>

#define MAX_REFLECTIONS_COUNT 3

//TODO: remove this (used to print complex with qDebug())
QDebug operator<<(QDebug debug, const complex<double> &c)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << '(' << real(c) << " + " << imag(c) << "i)";

    return debug;
}

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
 * @param source : The position of the source whose image is calculated
 * @param wall   : The wall over which compute the image
 *
 * @return
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

/**
 * @brief SimulationHandler::recursiveReflection
 * @param emitter      : The emitter for this ray path
 * @param receiver     : The receiver for this ray path
 * @param reflect_wall : The wall on which we will compute the reflection
 * @param images       : The list of images for the previous reflections
 * @param walls        : The list of walls for the previous reflections
 * @param level        : The recursion level
 *
 * This function gets the image of the source (emitter of previous image) over the 'reflect_wall'
 * and compute the ray path recursively.
 */
QList<RayPath *> SimulationHandler::recursiveReflection(
        Emitter *emitter,
        Receiver *receiver,
        Wall *reflect_wall,
        QList<QPointF> images,
        QList<Wall *> walls,
        int level)
{
    // This list will contains all the ray paths computed recursively
    QList<RayPath*> raypath_list;

    // Position of the image of the source
    QPointF src_image;

    if(images.size() == 0) {
        // If there is no previous reflection, the source is the emitter
        src_image = mirror(emitter->getRealPos(), reflect_wall);
    }
    else{
        // If there are previous reflections, the source is the last image
        src_image = mirror(images.last(), reflect_wall);
    }

    // Keep the list of walls and images for the next recursions
    images.append(src_image);
    walls.append(reflect_wall);

    // Compute the complete ray path for this set of reflections
    RayPath *rp = computeRayPath(emitter, receiver, images, walls);

    // Add this ray path to the list
    raypath_list.append(rp);

    // If the level of recursion is under the max number of reflections
    if(level < MAX_REFLECTIONS_COUNT)
    {
        // Compute the reflection from the 'reflect_wall' to all other walls of the scene
        foreach (Wall *w, simulationData()->getWallsList())
        {
            // We don't have to compute any reflection from the 'reflect_wall' to itself
            if(w == reflect_wall){
                continue;
            }

            // Recursive call for each wall of the scene (and increase the recusion level)
            QList<RayPath*> rp_rec = recursiveReflection(emitter, receiver, w, images, walls, level+1);

            // Add the ray paths computed recursively to the list of ray paths to return
            raypath_list.append(rp_rec);
        }
    }

    // Remove all invalid ray paths from the list
    raypath_list.removeAll(nullptr);

    return raypath_list;
}

/**
 * @brief SimulationHandler::computeRayPath
 * @param emitter  : The emitter for this ray path
 * @param receiver : The receiver for this ray path
 * @param images   : The list of reflection images computed for this ray path
 * @param walls    : The list of walls that form a combination of reflections
 *
 * This function computes the ray path for a combination reflections.
 */
RayPath *SimulationHandler::computeRayPath(
        Emitter *emitter,
        Receiver *receiver,
        QList<QPointF> images,
        QList<Wall*> walls)
{
    // We run backward in this function (from receiver to emitter)

    // The first target point is the receiver
    QPointF target_point = receiver->getRealPos();

    // This list will contain the lines forming the ray path
    QList<QLineF> rays;

    // This coefficient will contain the product of all reflection and
    // transmission coefficients for this ray path
    complex<double> coeff = 1;

    // Total length of the ray path
    double dn;

    // Wall of the next reflection (towards the receiver)
    Wall *target_wall = nullptr;

    // Loop over the images (backward)
    for(int i = images.size()-1; i >= 0 ; i--) {
        Wall *reflect_wall = walls[i];
        QPointF src_image = images[i];

        // Compute the virtual ray (line from the image to te target point)
        QLineF virtual_ray (src_image, target_point);

        // If this is the virtual ray from the last image to the receiver,
        // it length is the total ray path length dn.
        if (src_image == images.last()) {
            dn = virtual_ray.length();
        }

        // Get the reflection point (intersection of the virtual ray and the wall)
        QPointF reflection_pt;
        QLineF::IntersectionType i_t =
                virtual_ray.intersects(reflect_wall->getRealLine(), &reflection_pt);

        // The ray path is valid if the reflection is on the wall (not on its extension)
        if(i_t != QLineF::BoundedIntersection) {
            return nullptr; // Return an invalid ray path
        }

        // Add this ray line to the list of lines forming the ray path
        QLineF ray(reflection_pt, target_point);
        rays.append(ray);

        // Compute the reflection coefficient for this reflection
        coeff *= computeReflection(emitter, reflect_wall, ray);

        // Compute the transmission coefficient for all transmissions
        // undergone by the ray line.
        coeff *= computeTransmissons(emitter, ray, reflect_wall, target_wall);

        // The next target point is the current reflection point
        target_point = reflection_pt;
        target_wall = reflect_wall;
    }

    // The last ray line is from the emitter to the target point
    QLineF ray(emitter->getRealPos(), target_point);
    rays.append(ray);

    // Compute all the transmissions undergone by the ray line.
    coeff *= computeTransmissons(emitter, ray, nullptr, target_wall);

    // If there were no images in the list, we are computing the direct ray, so the length
    // of the ray path (dn) is the length of the ray line from emitter to receiver.
    if(images.size() == 0){
        dn = ray.length();
    }

    // Compute the electric field for this ray path
    complex<double> En = coeff * computeNominalElecField(emitter, ray, dn);

    qDebug() << "En" << dn << coeff << En;

    // Return a new RayPath object
    RayPath *rp = new RayPath(rays, En);
    return rp;
}

double SimulationHandler::computePowerToReceiver(Receiver *r, QList<RayPath*> *raypaths_list) {
    // The sum of the power received from each emitter
    double total_power = 0;

    // Loop ever the emitters
    foreach(Emitter *e, simulationData()->getEmittersList()) {
        // List of raypaths from this emitter
        QList<RayPath*> rp_list;

        // Compute the direct ray path and add it to the list
        RayPath *rp = computeRayPath(e, r);
        rp_list.append(rp);

        // For each wall in the scene, compute the reflections recursively
        foreach(Wall *w, simulationData()->getWallsList()) {
            // Append the list of ray paths computed recursively to the main list
            QList<RayPath*> lst = recursiveReflection(e, r, w);
            rp_list.append(lst);
        }

        // Compute the average power of all rays from this emitter to the receiver
        double power = computeAvgPower(e, rp_list);

        // Sum this power with the power from other emitters
        total_power += power;

        // Add these raypaths to the list given in parameters
        raypaths_list->append(rp_list);

        qDebug() << power;
        qDebug() << rp_list.size();
    }

    return total_power;
}

void SimulationHandler::computeAllRays() {
    QElapsedTimer tm;
    tm.start();

    // Loop over the receivers
    foreach(Receiver *r , simulationData()->getReceiverList()) {
        QList<RayPath*> raypath_list;

        double power = computePowerToReceiver(r, &raypath_list);

        qDebug() << power;
        qDebug() << raypath_list.size();

        foreach (RayPath *rp, raypath_list) {
            m_simulation_scene->addItem(rp);
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

complex<double> SimulationHandler::computeReflection(Emitter *e, Wall *w, QLineF ray_in){
    double theta_i = w->getNormalAngleTo(ray_in);
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

complex<double> SimulationHandler::computeNominalElecField(Emitter *e, QLineF ray, double dn){
    double GTX = e->getGain(M_PI_2, 0);
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

        double theta_i = w->getNormalAngleTo(ray);
        double theta_t = asin(real(Z2/Z1)*sin(theta_i));

        complex<double>gamma_orth = (Z2*cos(theta_i) - Z1*cos(theta_t)) / (Z2*cos(theta_i)+ Z1 * cos(theta_t));
        double s = w->getThickness()/cos(theta_t);

        complex<double> gamma_m = propagationConstant(omega, epsilon_tilde);

        qDebug() << "Tr" << Z1 << Z2;

        complex<double> transmission = (1.0-pow(gamma_orth,2.0))*exp(-gamma_m*s)/(1.0-pow(gamma_orth,2.0)*exp(-2.0*gamma_m*s + gamma_0*2.0*s*sin(theta_t)*sin(theta_i)));
        coeff *= transmission;
    }

    return coeff;
}

// Formula 8.83
double SimulationHandler::computeAvgPower(Emitter *e, QList<RayPath *> rp_list){
    double Ra = e->getResistance();
    double theta = M_PI_2;
    double Prx = 0;

    foreach(RayPath* rp, rp_list){
        //TODO phi = incidence angle emiter
        double phi = 0;
        complex<double> he = e->getEffectiveHeight(theta,phi);

        // norm() = square of modulus
        Prx += norm(he * rp->getElecField());

        qDebug() << "Pxr" << Prx << he << rp->getElecField();
    }
    Prx /= 8.0 * Ra;

    return Prx;
}
