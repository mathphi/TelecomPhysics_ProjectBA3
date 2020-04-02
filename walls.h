#ifndef WALL_H
#define WALL_H

#include <QGraphicsLineItem>

class Wall : public QGraphicsLineItem
{
public:
    Wall(QLine line, int thickness);
    int getThickness();

private:
    int  m_thickness;

protected:
    double m_rpermittivity;
    double m_conductivity;
};

class BrickWall : public Wall
{
public:
    BrickWall(QLine line, int thickness);
};


class ConcreteWall : public Wall
{
public:
    ConcreteWall(QLine line, int thickness);
};


class PartitionWall : public Wall
{
public:
    PartitionWall(QLine line, int thickness);
};







#endif // WALL_H
