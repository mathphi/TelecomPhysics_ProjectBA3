#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <complex>

// Needed to avoid a confusion with 1i implementations
using namespace std::literals::complex_literals;

// To avoid to type std:: before each complex
using namespace std;

const double LIGHT_SPEED = 299792458.0;     // [m/s]
const double EPSILON_0   = 8.85418782e-12;  // [F/m]
const double MU_0        = 12.566370614e-7; // [T m/A]

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

#endif // CONSTANTS_H
