#include "simulationhandler.h"

#include <QDebug>

SimulationHandler::SimulationHandler()
{
    m_simulation_data = new SimulationData();
    m_sim_started = false;
    m_sim_cancelling = false;
    m_init_cu_count = 0;
}

SimulationData *SimulationHandler::simulationData() {
    return m_simulation_data;
}

/**
 * @brief SimulationHandler::getRayPathsList
 * @return
 *
 * This function returns all the computed ray paths in the scene
 */
QList<RayPath*> SimulationHandler::getRayPathsList() {
    QList<RayPath*> ray_paths;

    // Append the ray paths of each receivers to the ray paths list to return
    foreach(Receiver *re, m_receivers_list) {
        ray_paths.append(re->getRayPaths());
    }

    return ray_paths;
}

/**
 * @brief SimulationHandler::isRunning
 * @return
 *
 * This function returns true if a simulation computation is running
 */
bool SimulationHandler::isRunning() {
    return m_sim_started;
}


/**************************************************************************************************/
// --------------------------------- COMPUTATION FUNCTIONS -------------------------------------- //
/**************************************************************************************************/

/**
 * @brief SimulationHandler::mirror
 *
 * This function returns the coordinates of the image of the 'source' point
 * after an axial symmetry through the wall.
 *
 * WARNING: the y axis is upside down in the graphics scene !
 *
 * @param source : The position of the source whose image is calculated
 * @param wall   : The wall over which compute the image
 * @return       : The coordinates of the image
 */
QPointF SimulationHandler::mirror(QPointF source, Wall *wall) {
    // Get the angle of the wall to the horizontal axis
    double theta = wall->getRealLine().angle() * M_PI / 180.0 - M_PI / 2.0;

    // Translate the origin of the coordinates system on the base of the wall
    double x = source.x() - wall->getRealLine().p1().x();
    double y = source.y() - wall->getRealLine().p1().y();

    // We use a rotation matrix :
    //   x' =  x*cos(??) - y*sin(??)
    //   y' = -x*sin(??) - y*cos(??)
    // to set the y' axis along the wall, so the image is at the opposite of the x' coordinate
    // in the new coordinates system.
    double x_p =  x*cos(theta) - y*sin(theta);
    double y_p = -x*sin(theta) - y*cos(theta);

    // Rotate back to the original coordinates system
    QPointF rel_pos = QPointF(-x_p*cos(theta) - y_p*sin(theta), x_p*sin(theta) - y_p*cos(theta));

    // Translate back to the original coordinates system
    return rel_pos + wall->getRealLine().p1();
}


/**
 * @brief SimulationHandler::computeReflection
 *
 * This function computes the reflection coefficient for the reflection of
 * an incident ray on a wall.
 * The coefficient returned is 3-dimensionnal:
 *  - the two first components are the same and are the coefficient
 *    for a parallel polarization
 *  - the last component is the coefficient for the othogonal polarization
 *
 * @param em     : The emitter (source of this ray)
 * @param w      : The reflection wall
 * @param ray_in : The incident ray
 * @return       : The reflection coefficient for this reflection
 */
vector<complex> SimulationHandler::computeReflection(Emitter *em, Wall *w, QLineF in_ray) {
    // Get the pulsation of the emitter
    double omega = em->getFrequency()*2*M_PI;

    // Get the properties of the reflection wall
    double e_r = w->getRelPermitivity();
    double sigma = w->getConductivity();

    // Compute the properties of the mediums (air and wall)
    complex epsilon_tilde = e_r*EPSILON_0 - 1i*sigma/omega;
    complex Z1 = Z_0;
    complex Z2 = sqrt(MU_0/epsilon_tilde);

    // Compute the incident and transmission angles
    double theta_i = w->getNormalAngleTo(in_ray);
    double theta_t = asin(real(Z2/Z1) * sin(theta_i));

    // Length of the travel of the ray in the wall
    double s = w->getThickness() / cos(theta_t);

    // Compute the reflection coefficient for an orthogonal
    // polarization (equation 8.39)
    complex Gamma_orth = (Z2*cos(theta_i) - Z1*cos(theta_t)) / (Z2*cos(theta_i) + Z1*cos(theta_t));
    // Compute the reflection coefficient for a parallel
    // polarization (equation 8.32)
    complex Gamma_para = (Z2*cos(theta_t) - Z1*cos(theta_i)) / (Z2*cos(theta_t) + Z1*cos(theta_i));

    // Propagation constants (m -> in wall, 0 -> in air)
    complex gamma_m = 1i*omega*sqrt(MU_0*epsilon_tilde);
    complex gamma_0 = 1i*omega*sqrt(MU_0*EPSILON_0);

    // Compute the reflection coefficient for the orthogonal polarization (equation 8.43)
    complex reflection_orth =
            Gamma_orth +
            (1.0 - pow(Gamma_orth, 2.0)) *
            Gamma_orth * exp(-2.0*gamma_m*s + 2.0*gamma_0*s * sin(theta_t) * sin(theta_i)) /
            (1.0 - pow(Gamma_orth, 2.0) *
             exp(-2.0*gamma_m*s + 2.0*gamma_0*s * sin(theta_t) * sin(theta_i)));

    // Compute the reflection coefficient for the parallel polarization (equation 8.43)
    complex reflection_para =
            Gamma_para +
            (1.0 - pow(Gamma_para, 2.0)) *
            Gamma_para * exp(-2.0*gamma_m*s + 2.0*gamma_0*s * sin(theta_t) * sin(theta_i)) /
            (1.0 - pow(Gamma_para, 2.0) *
             exp(-2.0*gamma_m*s + 2.0*gamma_0*s * sin(theta_t) * sin(theta_i)));

    // Return as a 3-D vector
    return {
        reflection_para,
        reflection_para,
        reflection_orth
    };
}

/**
 * @brief SimulationHandler::computeTransmissons
 *
 * This function computes all the transmissions undergone by the 'ray', and returns
 * the total transmission coefficient.
 * The coefficient returned is 3-dimensionnal:
 *  - the two first components are the same and are the coefficient
 *    for a parallel polarization
 *  - the last component is the coefficient for the othogonal polarization
 *
 * @param em          : The emitter (source of this ray)
 * @param ray         : The ray for which to compute the transmissions
 * @param origin_wall : The wall from which this ray come from (reflection), or nullptr
 * @param target_wall : The wall to which this ray go to (reflection), or nullptr
 * @return            : The total transmission coefficient for all undergone transmissions
 */
vector<complex> SimulationHandler::computeTransmissons(
        Emitter *em,
        QLineF ray,
        Wall *origin_wall,
        Wall *target_wall)
{
    // Get pulsation from the emitter
    double omega = em->getFrequency()*2*M_PI;

    // Propagation constant (air)
    complex gamma_0 = 1i*omega*sqrt(MU_0*EPSILON_0);

    // Total transmission coefficient (for this ray)
    vector<complex> total_coeff = {1,1,1};

    // Loop over all walls of the scene and look for transmissions (intersection with ray)
    foreach(Wall *w, simulationData()->getWallsList()) {
        // No transmission through the origin or target wall (where this ray is reflected)
        if (w == origin_wall || w == target_wall) {
            continue;
        }

        // Get the transmission point
        QPointF pt;
        QLineF::IntersectionType i_t = ray.intersects(w->getRealLine(), &pt);

        // There is transmission if the intersection with the ray is
        // on the wall (not on its extension)
        if (i_t != QLineF::BoundedIntersection) {
            continue;
        }

        // Get properties from the transmission wall
        double e_r = w->getRelPermitivity();
        double sigma = w->getConductivity();

        // Compute the properties of the mediums (air and wall)
        complex epsilon_tilde = e_r*EPSILON_0 - 1i*sigma/omega;
        complex Z1 = Z_0;
        complex Z2 = sqrt(MU_0/epsilon_tilde);

        // Propagation constant (in this wall)
        complex gamma_m = 1i*omega*sqrt(MU_0*epsilon_tilde);

        // Compute the incident and transmission angles
        double theta_i = w->getNormalAngleTo(ray);
        double theta_t = asin(real(Z2/Z1)*sin(theta_i));

        // Length of the travel of the ray in the wall
        double s = w->getThickness() / cos(theta_t);

        // Compute the reflection coefficient for an orthogonal polarization (equation 8.39).
        // The transmission coefficient is deduced from the reflection coefficient (equation 8.37).
        complex Gamma_orth = (Z2*cos(theta_i)-Z1*cos(theta_t))/(Z2*cos(theta_i)+Z1*cos(theta_t));

        // Compute the reflection coefficient for a parallel polarization (equation 8.32)
        complex Gamma_para = (Z2*cos(theta_t)-Z1*cos(theta_i))/(Z2*cos(theta_t)+Z1*cos(theta_i));

        // Compute the transmission coefficient for the orthogonal polarization (equation 8.44)
        complex transmission_orth =
                (1.0 - pow(Gamma_orth, 2.0)) * exp(-gamma_m*s) /
                (1.0 - pow(Gamma_orth, 2.0) *
                 exp(-2.0*gamma_m*s + 2.0*gamma_0*s * sin(theta_t) * sin(theta_i)));

        // Compute the transmission coefficient for the orthogonal polarization (equation 8.44)
        complex transmission_para =
                (1.0 - pow(Gamma_para, 2.0)) * exp(-gamma_m*s) /
                (1.0 - pow(Gamma_para, 2.0) *
                 exp(-2.0*gamma_m*s + 2.0*gamma_0*s * sin(theta_t) * sin(theta_i)));

        // Return as a 3-D vector
        vector<complex> coeff = {
            transmission_para,
            transmission_para,
            transmission_orth
        };

        // Multiply the total transmission coefficient with this one
        // The multiplication is made component by component (not a cross product).
        total_coeff *= coeff;
    }

    return total_coeff;
}

/**
 * @brief SimulationHandler::computeNominalElecField
 *
 * This function computes the "Nominal" electric field (equation 8.77).
 * So this is the electric field as if there were no reflection or transmission.
 * The electric field is returned in a 3-dimentionnal vector.
 *
 * @param em    : The emitter (source of this ray)
 * @param e_ray : The ray coming out from the emitter
 * @param r_ray : The ray coming to the receiver
 * @param dn    : The total length of the ray path
 * @return      : The "Nominal" electric field
 */
vector<complex> SimulationHandler::computeNominalElecField(
        Emitter *em,
        QLineF e_ray,
        QLineF r_ray,
        double dn)
{
    // Incidence angle of the ray from the emitter
    double phi = em->getIncidentRayAngle(e_ray);

    // Get the polarization vector of the emitter
    vector<complex> polarization = em->getPolarization();

    // Get properties from the emitter
    double GTX = em->getGain(phi);
    double PTX = em->getPower();
    double omega = em->getFrequency()*2*M_PI;

    // Compute the direction of the parallel component of the electric field at the receiver.
    // This direction is a unit vector normal to the propagation vector in the incidence plane.
    QLineF E_unit = r_ray.normalVector().unitVector();

    // Propagation constant (air)
    complex gamma_0 = 1i*omega*sqrt(MU_0*EPSILON_0);

    // Direct (nominal) electric field (equation 8.77)
    complex E = sqrt(60.0*GTX*PTX) * exp(-gamma_0*dn) / dn;

    // The first component of the polarization vector is the parallel component, the second
    // is the orthogonal one.
    // E_unit is the direction vector of the electric field in the incidence plane.
    return {
        E * polarization[0] * E_unit.dx(),
        E * polarization[0] * E_unit.dy(),
        E * polarization[1]
    };
}

/**
 * @brief SimulationHandler::computeRayPower
 *
 * This function computes the power of a ray path from an emitter to a receiver
 * (equation 8.83, applyed to one ray)
 *
 * @param em  : The emitter (source of the ray path)
 * @param re  : The receiver (destination of the ray path)
 * @param ray : The ray coming to the receiver
 * @param En  : The electric field of the ray
 * @return    : The power of the ray path to the receiver
 */
double SimulationHandler::computeRayPower(
        Emitter *em,
        Receiver *re,
        QLineF ray,
        vector<complex> En)
{
    // Incidence angle of the ray to the receiver
    double phi = re->getIncidentRayAngle(ray);

    // Get the frequency from the emitter
    double frequency = em->getFrequency();

    // Get the antenna's resistance and effective height
    double Ra = re->getResistance();
    vector<complex> he = re->getEffectiveHeight(phi, frequency);

    // norm() = square of modulus
    return norm(dotProduct(he, En)) / (8.0 * Ra);
}

/**
 * @brief SimulationHandler::computeRayPath
 *
 * This function computes the ray path for a combination reflections.
 *
 * @param emitter  : The emitter for this ray path
 * @param receiver : The receiver for this ray path
 * @param images   : The list of reflection images computed for this ray path
 * @param walls    : The list of walls that form a combination of reflections
 * @return         : A pointer to the new RayPath object computed (or nullptr if invalid)
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
    vector<complex> coeff = {1,1,1};

    // Total length of the ray path
    double dn;

    // Wall of the next reflection (towards the receiver)
    Wall *target_wall = nullptr;

    // Loop over the images (backward)
    for (int i = images.size()-1; i >= 0 ; i--) {
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
        if (i_t != QLineF::BoundedIntersection) {
            return nullptr; // Return an invalid ray path
        }

        // If the target point is the same as the reflection point
        //  -> not a physics situation -> invalid raypath
        if (reflection_pt == target_point) {
            return nullptr;
        }

        // Add this ray line to the list of lines forming the ray path
        QLineF ray(reflection_pt, target_point);
        rays.append(ray);

        // Compute the reflection coefficient for this reflection
        // The multiplication is made component by component (not a cross product).
        coeff *= computeReflection(emitter, reflect_wall, ray);

        // Compute the transmission coefficient for all transmissions undergone by the ray line.
        coeff *= computeTransmissons(emitter, ray, reflect_wall, target_wall);

        // The next target point is the current reflection point
        target_point = reflection_pt;
        target_wall = reflect_wall;
    }

    // If the target point is the same as the emitter point
    //  -> not a physics situation -> invalid raypath
    if (emitter->getRealPos() == target_point) {
        return nullptr;
    }

    // The last ray line is from the emitter to the target point
    QLineF ray(emitter->getRealPos(), target_point);
    rays.append(ray);

    // Compute all the transmissions undergone by the ray line.
    coeff *= computeTransmissons(emitter, ray, nullptr, target_wall);

    // If there were no images in the list, we are computing the direct ray, so the length
    // of the ray path (dn) is the length of the ray line from emitter to receiver.
    if (images.size() == 0) {
        dn = ray.length();
    }

    // Compute the electric field for this ray path (equation 8.78)
    // rays.last() is the ray coming out from the emitter
    // rays.first() is the ray coming to the receiver
    // The multiplication is made component by component (not a cross product).
    vector<complex> En = coeff * computeNominalElecField(emitter, rays.last(), rays.first(), dn);

    // Compute the power of the ray coming to the receiver (first ray in the list)
    double power = computeRayPower(emitter, receiver, rays.first(), En);

    // Return a new RayPath object
    RayPath *rp = new RayPath(emitter, rays, power);
    return rp;
}

/**
 * @brief SimulationHandler::recursiveReflection
 *
 * This function gets the image of the source (emitter of previous image) over the 'reflect_wall'
 * and compute the ray path recursively.
 * The computed ray paths are added to the RayPaths list of the receiver
 *
 * @param emitter      : The emitter for this ray path
 * @param receiver     : The receiver for this ray path
 * @param reflect_wall : The wall on which we will compute the reflection
 * @param images       : The list of images for the previous reflections
 * @param walls        : The list of walls for the previous reflections
 * @param level        : The recursion level
 */
void SimulationHandler::recursiveReflection(
        Emitter *emitter,
        Receiver *receiver,
        Wall *reflect_wall,
        QList<QPointF> images,
        QList<Wall *> walls,
        int level)
{
    // Position of the image of the source
    QPointF src_image;

    if (images.size() == 0) {
        // If there is no previous reflection, the source is the emitter
        src_image = mirror(emitter->getRealPos(), reflect_wall);
    }
    else {
        // If there are previous reflections, the source is the last image
        src_image = mirror(images.last(), reflect_wall);
    }

    // Keep the list of walls and images for the next recursions
    images.append(src_image);
    walls.append(reflect_wall);

    // Compute the complete ray path for this set of reflections
    RayPath *rp = computeRayPath(emitter, receiver, images, walls);

    // Add this ray path to his receiver
    receiver->addRayPath(rp);

    // If the level of recursion is under the max number of reflections
    if (level < simulationData()->maxReflectionsCount())
    {
        // Compute the reflection from the 'reflect_wall' to all other walls of the scene
        foreach (Wall *w, simulationData()->getWallsList())
        {
            // We don't have to compute any reflection from the 'reflect_wall' to itself
            if (w == reflect_wall){
                continue;
            }

            // Recursive call for each wall of the scene (and increase the recusion level)
            recursiveReflection(emitter, receiver, w, images, walls, level+1);
        }
    }
}

/**************************************************************************************************/
// ---------------------------- SIMULATION MANAGEMENT FUNCTIONS --------------------------------- //
/**************************************************************************************************/

/**
 * @brief SimulationHandler::computeAllRays
 *
 * This function computes the rays from every emitters to every receivers.
 * This is an asynchronous function that adds computation units to the thread pool.
 */
void SimulationHandler::computeAllRays() {
    // Start the time counter
    m_computation_timer.start();

    // Loop over the receivers
    foreach(Receiver *r, m_receivers_list)
    {
        // Loop over the emitters
        foreach(Emitter *e, simulationData()->getEmittersList())
        {
            // Compute the direct ray path and add it to his receiver
            r->addRayPath(computeRayPath(e, r));

            // For each wall in the scene, compute the reflections recursively
            foreach(Wall *w, simulationData()->getWallsList())
            {
                // Don't compute any reflection if not needed
                if (simulationData()->maxReflectionsCount() > 0) {
                    // Compute the ray paths recursively (in a thread)
                    recursiveReflectionThreaded(e, r, w);
                }
            }
        }
    }

    // Call it once (in the case we don't have any computation unit created)
    computationUnitFinished();
}

/**
 * @brief SimulationHandler::recursiveReflectionThreaded
 * @param e
 * @param r
 * @param w
 *
 * This function creates a computation unit to compute the reflections
 * recursively in a thread.
 */
void SimulationHandler::recursiveReflectionThreaded(Emitter *e, Receiver *r, Wall *w) {
    // Create a computation unit for the recursive computation of the reflections
    ComputationUnit *cu = new ComputationUnit(this, e, r, w);

    // One thread can write in this list at a time (mutex)
    m_mutex.lock();
    m_computation_units.append(cu);
    m_mutex.unlock();

    // Connect the computation unit to the simulation handler
    connect(cu, SIGNAL(computationFinished()), this, SLOT(computationUnitFinished()));

    // Add this computation unit to the queue of the thread pool
    m_threadpool.start(cu);

    // Increase the computation units counter
    m_init_cu_count++;
}

/**
 * @brief SimulationHandler::computationUnitFinished
 *
 * This slot is called when a computation unit finished to compute
 */
void SimulationHandler::computationUnitFinished() {
    // Get the computation unit (origin of the signal)
    ComputationUnit *cu = qobject_cast<ComputationUnit*> (sender());

    // If a computation unit is the origin of the call to this function
    if (cu != nullptr) {
        // One thread can write in this list at a time (mutex)
        m_mutex.lock();
        m_computation_units.removeOne(cu);
        m_mutex.unlock();

        // Delete this computation unit
        cu->deleteLater();
    }

    // Compute the progression and send the progression signal
    double progress = 1.0 - (double) m_computation_units.size() / (double) m_init_cu_count;
    emit simulationProgress(progress);

    // All computations done
    if (m_computation_units.size() == 0){
        qDebug() << "Time (ms):" << m_computation_timer.nsecsElapsed() / 1e6;
        qDebug() << "Count:" << getRayPathsList().size();
        qDebug() << "Receivers:" << m_receivers_list.size();
        qDebug() << "Walls:" << m_simulation_data->getWallsList().size();

        // Mark the simulation as stopped
        m_sim_started = false;

        if (!m_sim_cancelling) {
            // Emit the simulation finished signal
            emit simulationFinished();
        }
        else {
            // Reset the cancelling flag
            m_sim_cancelling = false;

            // Emit the simulation cancelled signal
            emit simulationCancelled();
        }
    }
}

/**
 * @brief SimulationHandler::startSimulationComputation
 * @param rcv_list
 *
 * This function starts a computation of all rays to a list of receivers
 */
void SimulationHandler::startSimulationComputation(QList<Receiver*> rcv_list) {
    // Don't start a new computation if already running
    if (isRunning())
        return;

    // Reset the previously computed data (if one)
    resetComputedData();

    // Setup the receivers list
    m_receivers_list = rcv_list;

    // Mark the simulation as running
    m_sim_started = true;

    // Reset the counter of computation units
    m_init_cu_count = 0;

    // Emit the simulation started signal
    emit simulationStarted();
    emit simulationProgress(0);

    // Compute all rays
    computeAllRays();
}

/**
 * @brief SimulationHandler::stopSimulationComputation
 *
 * This function cancel the current simulation.
 * The cancel operation must wait for all threads to finish.
 */
void SimulationHandler::stopSimulationComputation() {
    // Clear the queue of the thread pool
    m_threadpool.clear();

    // Mark the simulation as cancelling
    m_sim_cancelling = true;

    // Delete all CU that are not (yet) running
    foreach(ComputationUnit *cu, m_computation_units) {
        if (!cu->isRunning()) {
            // One thread can write in this list at a time (mutex)
            m_mutex.lock();
            m_computation_units.removeOne(cu);
            m_mutex.unlock();

            // Delete this CU
            cu->deleteLater();
        }
    }
}

/**
 * @brief SimulationHandler::resetComputedData
 *
 * This function erases the computation results and computed RayPaths
 */
void SimulationHandler::resetComputedData() {
    // Reset each receiver
    foreach(Receiver *r, m_receivers_list) {
        r->reset();
    }

    // Clear the receivers list
    m_receivers_list.clear();
}

/**
 * @brief SimulationHandler::getPowerDataBoundaries
 * @param min
 * @param max
 *
 * This function gives the boundary values of power in the scene
 * (max and min power value around all receivers in the scene).
 */
void SimulationHandler::powerDataBoundaries(double *min, double *max) {
    *min = 0;
    *max = 0;

    // Loop over every receiver and get its power
    foreach(Receiver *re , m_receivers_list)
    {
        // Get the receiver's received power
        double pwr = re->receivedPower();

        // Keep this value if min/max
        if (*min > pwr || *min == 0) {
            *min = pwr;
        }
        if (*max < pwr) {
            *max = pwr;
        }
    }
}

/**
 * @brief SimulationHandler::showReceiversResults
 *
 * This function send to all receivers to show their results
 */
void SimulationHandler::showReceiversResults(ResultType::ResultType r_type) {
    // Data range for Bitrate
    double min = 54;
    double max = 433;

    // If we want to show the power
    if (r_type == ResultType::Power) {
        // Get the max and min from the receivers
        powerDataBoundaries(&min, &max);

        // We show the values in dBm
        min = SimulationData::convertPowerTodBm(min);
        max = SimulationData::convertPowerTodBm(max);
    }

    // Loop over every receiver and show its results
    foreach(Receiver *re , m_receivers_list)
    {
        // Show the results of each receiver
        re->showResults(r_type, min, max);
    }
}
