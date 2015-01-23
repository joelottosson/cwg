/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#ifndef TANKGAMEWIDGET_H
#define TANKGAMEWIDGET_H

#include <QWidget>
#include "gameworld.h"
#include "tankinfowidget.h"

class TankGameWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TankGameWidget(const GameWorld& world, QWidget *parent = 0);

    void Reset();

protected:
    virtual void paintEvent(QPaintEvent *);

private:
    const GameWorld& m_world;
    const int m_margin;
    double m_scale;
    QPixmap m_tankBlue;
    QPixmap m_tankRed;
    QPixmap m_tankTower;
    QPixmap m_obstacle;
    QPixmap m_missile;
    QPixmap m_tankWreck;
    QPixmap m_mine;
    QPixmap m_poison;

    boost::shared_ptr<QPixmap> m_backgroundPixmap;
    boost::shared_ptr<QPainter> m_backgroundPainter;

    struct PaintConstants
    {
        static const int squarePixelSize = 72;
        QPoint boardPixelSizeInt;
        QPointF boardPixelSizeFloat;
        QPointF upperLeft;
    };
    PaintConstants m_const;
    void UpdatePaintConstants();

    inline QPointF ToScreen(const QPointF& p, qreal xOffset, qreal yOffset)
    {
        return QPointF(p.x()*m_const.squarePixelSize+xOffset, p.y()*m_const.squarePixelSize+yOffset);
    }

    void PaintGrid(QPainter& painter);
    void PaintWalls(QPainter& painter);
    void PaintPoison(QPainter& painter);
    void PaintMines(QPainter& painter);
    void PaintTank(const Tank& tank, bool blue, QPainter& painter);
    void PaintMissile(const Missile& missile, QPainter& painter);
    void PaintSprite(const Sprite& sprite, QPainter& painter);
    void PaintText(const ScreenText& txt, QPainter& painter);
    void PaintWinner(QPainter& painter);
};

#endif // TANKGAMEWIDGET_H
