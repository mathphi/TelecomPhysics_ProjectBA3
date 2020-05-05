#ifndef ANTENNA_H
#define ANTENNA_H

#include "constants.h"
#include <QString>

namespace AntennaType {
enum AntennaType {
    HalfWaveDipoleVert  = 1,
    End = -1
};
}


class Antenna
{
public:
    Antenna(double efficiency = 1.0);

    double getEfficiency() const;

    static Antenna *createAntenna(AntennaType::AntennaType type, double efficiency);

    virtual AntennaType::AntennaType getAntennaType() const = 0;
    virtual QString getAntennaName() const = 0;
    virtual QString getAntennaLabel() const = 0;

    virtual double getResistance() const = 0;
    virtual complex<double> getEffectiveHeight(double theta, double phi, double frequency) const = 0;
    virtual double getGain(double theta, double phi) const = 0;

private:
    double m_efficiency;
};

class HalfWaveDipoleVert : public Antenna
{
public:
    HalfWaveDipoleVert(double efficiency = 1.0);

    AntennaType::AntennaType getAntennaType() const override;
    QString getAntennaName() const override;
    QString getAntennaLabel() const override;

    double getResistance() const override;
    complex<double> getEffectiveHeight(double theta, double phi, double frequency) const override;
    double getGain(double theta, double phi) const override;
};


// Operator overload to write objects from the Antenna class into a files
QDataStream &operator>>(QDataStream &in, Antenna *&a);
QDataStream &operator<<(QDataStream &out, Antenna *a);

#endif // ANTENNA_H
