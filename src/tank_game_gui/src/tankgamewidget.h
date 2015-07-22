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
#include "sprite.h"

namespace CWG= Consoden::TankGame;

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
    QPixmap m_dudes;
    QPixmap m_dead_dude;
    QPixmap m_poison;
    QPixmap m_redeemer;
    SpriteData m_dude_anim;


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
    void PaintDudes(const Dude& dude, QPainter& painter);
    void PaintMines(QPainter& painter);
    void PaintTank(const Tank& tank, bool blue, QPainter& painter);
    void PaintMissile(const Missile& missile, QPainter& painter);
    void PaintRedeemer(const Redeemer& redeemer, QPainter& painter);
    void PaintSprite(const Sprite& sprite, QPainter& painter);
    //void PaintSpriteDude(const Sprite& sprite, QPainter& painter);
    void PaintText(const ScreenText& txt, QPainter& painter);
    void PaintWinner(QPainter& painter);
    inline void ManualDraw(QPainter& painter,QPixmap pixmap, QPointF pos,qreal rotation, bool wrap,Direction direction);
    inline void ManualDraw(QPainter& painter,QPixmap pixmap, QPointF pos,qreal rotation, bool wrap);
};

#endif // TANKGAMEWIDGET_H
