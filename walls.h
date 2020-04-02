#ifndef WALL_H
#define WALL_H

#include <QGraphicsLineItem>

// Abstract wall class
class Wall : public QGraphicsLineItem
{
public:
    Wall(QLine line, int thickness);

    int getThickness();

    // The 'virtual' keyword makes these functions abstracts
    virtual double getRelPermitivity() = 0;
    virtual double getConductivity() = 0;

private:
    int  m_thickness;
};


class BrickWall : public Wall
{
public:
    BrickWall(QLine line, int thickness);

    double getRelPermitivity();
    double getConductivity();
};


class ConcreteWall : public Wall
{
public:
    ConcreteWall(QLine line, int thickness);

    double getRelPermitivity();
    double getConductivity();
};


class PartitionWall : public Wall
{
public:
    PartitionWall(QLine line, int thickness);

    double getRelPermitivity();
    double getConductivity();
};

#endif // WALL_H
