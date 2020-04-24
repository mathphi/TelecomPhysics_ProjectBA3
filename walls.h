#ifndef WALL_H
#define WALL_H

#include <QGraphicsItem>
#include <QPen>

#include "simulationitem.h"

// Type used to recognize the saved classes into a file
namespace WallType {
enum WallType{
    BrickWall       = 1,
    ConcreteWall    = 2,
    PartitionWall   = 3,
};
}

// Abstract wall class
class Wall : public SimulationItem
{
public:
    Wall(QLineF line, double thickness);

    QLineF getLine();
    void setLine(QLineF line);

    QLineF getRealLine();

    double getThickness();

    QPen getPen();
    void setPen(QPen pen);

    // The 'virtual' keyword makes these functions abstracts
    virtual double getRelPermitivity() = 0;
    virtual double getConductivity() = 0;
    virtual WallType::WallType getWallType() = 0;

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

private:
    QRectF getLengthTextRect() const;

    QLineF m_line;
    double  m_thickness;

    QPen m_pen;

    qreal m_text_scale;
};


class BrickWall : public Wall
{
public:
    BrickWall(QLineF line);
    BrickWall(QLineF line, double thickness);

    double getRelPermitivity();
    double getConductivity();
    WallType::WallType getWallType();
};


class ConcreteWall : public Wall
{
public:
    ConcreteWall(QLineF line);
    ConcreteWall(QLineF line, double thickness);

    double getRelPermitivity();
    double getConductivity();
    WallType::WallType getWallType();
};


class PartitionWall : public Wall
{
public:
    PartitionWall(QLineF line);
    PartitionWall(QLineF line, double thickness);

    double getRelPermitivity();
    double getConductivity();
    WallType::WallType getWallType();
};

// Operator overload to write objects from the Wall class into a files
QDataStream &operator>>(QDataStream &in, Wall *&w);
QDataStream &operator<<(QDataStream &out, Wall *w);

#endif // WALL_H
