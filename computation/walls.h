#ifndef WALL_H
#define WALL_H

#include <QGraphicsItem>
#include <QPen>

#include "interface/simulationitem.h"

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
    void setupTooltip();

    QLineF getLine();
    void setLine(QLineF line);

    QLineF getRealLine();
    double getNormalAngleTo(QLineF line);

    double getThickness();

    QPen getPen();
    void setPen(QPen pen);

    QBrush getBrush();
    void setBrush(QBrush b);

    virtual QString getTypeName() const = 0;

    // The 'virtual' keyword makes these functions abstracts
    virtual double getRelPermitivity() const = 0;
    virtual double getConductivity() const = 0;
    virtual WallType::WallType getWallType() const = 0;

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

private:
    QRectF getLengthTextRect() const;

    QLineF m_line;
    double  m_thickness;

    QPen m_pen;
    QBrush m_brush;

    qreal m_text_scale;
};


class BrickWall : public Wall
{
public:
    BrickWall(QLineF line);
    BrickWall(QLineF line, double thickness);

    QString getTypeName() const override;

    double getRelPermitivity() const override;
    double getConductivity() const override;
    WallType::WallType getWallType() const override;
};


class ConcreteWall : public Wall
{
public:
    ConcreteWall(QLineF line);
    ConcreteWall(QLineF line, double thickness);

    QString getTypeName() const override;

    double getRelPermitivity() const override;
    double getConductivity() const override;
    WallType::WallType getWallType() const override;
};


class PartitionWall : public Wall
{
public:
    PartitionWall(QLineF line);
    PartitionWall(QLineF line, double thickness);

    QString getTypeName() const override;

    double getRelPermitivity() const override;
    double getConductivity() const override;
    WallType::WallType getWallType() const override;
};

// Operator overload to write objects from the Wall class into a files
QDataStream &operator>>(QDataStream &in, Wall *&w);
QDataStream &operator<<(QDataStream &out, Wall *w);

#endif // WALL_H
