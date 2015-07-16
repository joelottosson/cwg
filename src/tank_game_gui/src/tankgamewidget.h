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

    int CalculateWrappingCoordinate(int val, int maxVal, int boardSize, int padd);

    void drawWithTranslationAndRotation(QPainter& painter, QPixmap image, QPointF position, qreal rotation);
    void drawWithWrapping(QPainter& painter, QPixmap image, QPointF position, qreal rotation);

	/*
	 *  const int xoffset=(m_const.squarePixelSize-m_tankTower.width())/2;
        const int yoffset=(m_const.squarePixelSize-m_tankTower.height())/2;
        const int x=xoffset+tank.paintPosition.x()*m_const.squarePixelSize;
        const int y=yoffset+tank.paintPosition.y()*m_const.squarePixelSize;
        painter.save();
        painter.translate(x+m_tankTower.width()/2, y+m_tankTower.height()/2);
        painter.rotate(tank.paintTowerAngle);
        painter.translate(m_tankTower.width()/-2, tankImage.height()/-2);
        painter.drawPixmap(0, 0, m_tankTower);
        painter.restore();
	 *
	 *
	 * */

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
};

#endif // TANKGAMEWIDGET_H
