#ifndef WALL_H
#define WALL_H

#include <QGraphicsLineItem>

namespace WallType {
enum WallType{
    BrickWall       = 1,
    ConcreteWall    = 2,
    PartitionWall   = 3,
};
}

// Abstract wall class
class Wall : public QGraphicsLineItem
{
public:
    Wall(QLine line, int thickness);

    int getThickness();

    // The 'virtual' keyword makes these functions abstracts
    virtual double getRelPermitivity() = 0;
    virtual double getConductivity() = 0;
    virtual WallType::WallType getWallType()= 0;

private:
    int  m_thickness;

};


class BrickWall : public Wall
{
public:
    BrickWall(QLine line);
    BrickWall(QLine line, int thickness);

    double getRelPermitivity();
    double getConductivity();
    WallType::WallType getWallType();
};


class ConcreteWall : public Wall
{
public:
    ConcreteWall(QLine line);
    ConcreteWall(QLine line, int thickness);

    double getRelPermitivity();
    double getConductivity();
    WallType::WallType getWallType();

};


class PartitionWall : public Wall
{
public:
    PartitionWall(QLine line);
    PartitionWall(QLine line, int thickness);

    double getRelPermitivity();
    double getConductivity();
    WallType::WallType getWallType();

};

#endif // WALL_H
