/******************************************************************************
*
* Copyright Consoden AB, 2014
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include <boost/make_shared.hpp>
#include <QtGui>
#include <qstyleoption.h>
#include "tankgamewidget.h"

TankGameWidget::TankGameWidget(const GameWorld& world, QWidget *parent)
    :QWidget(parent)
    ,m_world(world)
    ,m_margin(10)
    ,m_scale(1.0)
    ,m_tankBlue(":/images/panzerIV_blue.png")
    ,m_tankRed(":/images/panzerIV_red.png")
    ,m_tankTower(":/images/panzerIV_tower.png")
    ,m_obstacle(":/images/obstacle.jpg")
    ,m_missile(":/images/missile.png")
    ,m_tankWreck(":/images/panzerIV_wreck.png")
    ,m_mine(":/images/mine.png")
    ,m_flag(":/images/flag.png")
{
    this->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    Reset();
}

void TankGameWidget::Reset()
{
    UpdatePaintConstants();

    m_backgroundPainter.reset();
    m_backgroundPixmap.reset(new QPixmap(m_const.boardPixelSize.x()+2, m_const.boardPixelSize.y()+2));
    m_backgroundPixmap->fill(Qt::transparent);
    m_backgroundPainter.reset(new QPainter(m_backgroundPixmap.get()));
    m_backgroundPainter->setRenderHint(QPainter::Antialiasing);
    PaintGrid(*m_backgroundPainter);
    PaintWalls(*m_backgroundPainter);
}

void TankGameWidget::UpdatePaintConstants()
{
    m_const.boardPixelSize.setX(m_const.squarePixelSize*m_world.GetGameState().size.x());
    m_const.boardPixelSize.setY(m_const.squarePixelSize*m_world.GetGameState().size.y());
    if (m_const.boardPixelSize.y()<height())
    {
        m_scale=1.0;
    }
    else
    {
        //perform auto scale
        m_scale=(height()-10.0)/m_const.boardPixelSize.y();
    }

    m_const.upperLeft.setX(width()/2-(m_const.boardPixelSize.x()*m_scale)/2);
    m_const.upperLeft.setY(height()/2-(m_const.boardPixelSize.y()*m_scale)/2);
}

void TankGameWidget::paintEvent(QPaintEvent*)
{
    UpdatePaintConstants();

    //Paint mines
    PaintMines(*m_backgroundPainter);

    QPixmap tmp(*m_backgroundPixmap);    
    QPainter painter(&tmp);
    painter.setRenderHint(QPainter::Antialiasing);

    PaintFlags(painter);

    //Paint tanks
    int blueTank=true;
    for (auto& tank : m_world.GetGameState().tanks)
    {
        PaintTank(tank, blueTank, painter);
        blueTank=!blueTank;
    }

    //Paint missiles
    for (auto& vt : m_world.GetGameState().missiles)
    {
        PaintMissile(vt.second, painter);
    }

    //Paint sprites
    for (auto& s : m_world.Sprites())
    {
        PaintSprite(s, painter);
    }

    if (m_world.GetGameState().paintWinner)
    {
        PaintWinner(painter);
    }

    if (!m_world.GetTextBig().isEmpty())
    {
        PaintText(m_world.GetTextBig(), 30, 8, 0, painter);
    }

    if (!m_world.GetTextSmall().isEmpty())
    {
        PaintText(m_world.GetTextSmall(), 18, 3, m_const.boardPixelSize.y()/2, painter);
    }

    //Paint everything on screen
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    if (m_scale==1.0)
    {
        p.drawPixmap(m_const.upperLeft, tmp);
    }
    else
    {
        p.drawPixmap(m_const.upperLeft, tmp.scaledToWidth(static_cast<int>(tmp.width()*m_scale)));
    }
}

void TankGameWidget::PaintGrid(QPainter &painter)
{
    QPen pen;
    //pen.setStyle(Qt::SolidLine);
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(1);
    pen.setBrush(Qt::black);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);

    //horizontal
    for (int i=0; i<=m_world.GetGameState().size.y(); ++i)
    {
        painter.drawLine(0, i*m_const.squarePixelSize, m_const.boardPixelSize.x(), i*m_const.squarePixelSize);
    }
    //vertical
    for (int i=0; i<=m_world.GetGameState().size.x(); ++i)
    {
        painter.drawLine(i*m_const.squarePixelSize, 0, i*m_const.squarePixelSize, m_const.boardPixelSize.y());
    }
}


void TankGameWidget::PaintWalls(QPainter& painter)
{
    for (auto& p : m_world.GetGameState().walls)
    {
        painter.drawPixmap(p.x()*m_const.squarePixelSize,
                           p.y()*m_const.squarePixelSize,
                           m_const.squarePixelSize,
                           m_const.squarePixelSize,
                           m_obstacle);
    }
}

void TankGameWidget::PaintMines(QPainter& painter)
{    
    for (const auto& mine : m_world.GetGameState().mines)
    {
        painter.drawPixmap(ToScreen(mine, 0, 0), m_mine);
    }
}

void TankGameWidget::PaintFlags(QPainter& painter)
{
    for (const auto& flag : m_world.GetGameState().flags)
    {
        painter.drawPixmap(ToScreen(flag, 0, 0), m_flag);
    }
}

void TankGameWidget::PaintTank(const Tank& tank, bool blueTank, QPainter& painter)
{
    if (tank.explosion==Destroyed)
    {
        const int xoffset=(m_const.squarePixelSize-m_tankWreck.width())/2;
        const int yoffset=(m_const.squarePixelSize-m_tankWreck.height())/2;
        const int x=xoffset+tank.paintPosition.x()*m_const.squarePixelSize;
        const int y=yoffset+tank.paintPosition.y()*m_const.squarePixelSize;
        painter.save();
        painter.translate(x+m_tankWreck.width()/2, y+m_tankWreck.height()/2);
        painter.rotate(DirectionToAngle(tank.moveDirection));
        painter.translate(m_tankWreck.width()/-2, m_tankWreck.height()/-2);
        painter.drawPixmap(0, 0, m_tankWreck);
        painter.restore();
        return;
    }

    const QPixmap& tankImage=blueTank ? m_tankBlue : m_tankRed;
    //tank body
    {
        const int xoffset=(m_const.squarePixelSize-tankImage.width())/2;
        const int yoffset=(m_const.squarePixelSize-tankImage.height())/2;
        const int x=xoffset+tank.paintPosition.x()*m_const.squarePixelSize;
        const int y=yoffset+tank.paintPosition.y()*m_const.squarePixelSize;
        painter.save();
        painter.translate(x+tankImage.width()/2, y+tankImage.height()/2);
        painter.rotate(DirectionToAngle(tank.moveDirection));
        painter.translate(tankImage.width()/-2, tankImage.height()/-2);
        painter.drawPixmap(0, 0, tankImage);
        painter.restore();
    }

    //tank tower
    {        
        const int xoffset=(m_const.squarePixelSize-m_tankTower.width())/2;
        const int yoffset=(m_const.squarePixelSize-m_tankTower.height())/2;
        const int x=xoffset+tank.paintPosition.x()*m_const.squarePixelSize;
        const int y=yoffset+tank.paintPosition.y()*m_const.squarePixelSize;
        painter.save();
        painter.translate(x+m_tankTower.width()/2, y+m_tankTower.height()/2);
        painter.rotate(tank.paintTowerAngle);
        painter.translate(m_tankTower.width()/-2, tankImage.height()/-2);
        painter.drawPixmap(0, 0, m_tankTower);
        painter.restore();
    }
}

void TankGameWidget::PaintMissile(const Missile& missile, QPainter& painter)
{
    const int xoffset=(m_const.squarePixelSize-m_missile.width())/2;
    const int yoffset=(m_const.squarePixelSize-m_missile.height())/2;
    const int x=xoffset+missile.paintPosition.x()*m_const.squarePixelSize;
    const int y=yoffset+missile.paintPosition.y()*m_const.squarePixelSize;

    if (!missile.visible)
    {
        return;
    }

    painter.save();
    painter.translate(x+m_missile.width()/2, y+m_missile.height()/2);
    painter.rotate(DirectionToAngle(missile.moveDirection));
    painter.translate(m_missile.width()/-1, m_missile.height()/-2);
    painter.drawPixmap(0, 0, m_missile);
    painter.restore();
}

void TankGameWidget::PaintSprite(const Sprite& sprite, QPainter& painter)
{
    if (sprite.Started())
    {        
        QPainter::PixmapFragment pf=QPainter::PixmapFragment::create(ToScreen(sprite.Position(), m_const.squarePixelSize/2, m_const.squarePixelSize/2),
                                                                     sprite.Fragment(), 1, 1, sprite.Rotation(), 1);
        painter.drawPixmapFragments(&pf, 1, sprite.Image());
    }
}

void TankGameWidget::PaintText(const QStringList& sl, int pixelSize, int penWidth, int offset, QPainter& painter)
{
    painter.save();
    QPen pen;
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(penWidth);
    pen.setBrush(Qt::yellow);
    painter.setPen(pen);
    QFont font=painter.font();
    font.setPixelSize(pixelSize);
    font.setBold(true);
    painter.setFont(font);

    int rowStep=3*pixelSize;
    QRect rect(0, offset, m_const.boardPixelSize.x(), pixelSize);

    for (const QString& s : sl)
    {
        painter.drawText(rect, Qt::AlignCenter, s);
        rect.setY(rect.y()+rowStep);
    }

    painter.restore();
}

void TankGameWidget::PaintWinner(QPainter& painter)
{
    QStringList sl;

    if (m_world.GetGameState().winnerPlayerId==0)
    {
        sl.append("Game was a draw");
    }
    else
    {
        const Player* p1=m_world.GetPlayerOne();
        const Player* p2=m_world.GetPlayerTwo();
        if (!p1 || !p2)
        {
            return;
        }

        const Player* winner=NULL;
        const Player* loser=NULL;
        const Tank* loserTank=NULL;

        if (m_world.GetGameState().winnerPlayerId==p1->id)
        {
            //player 1 won
            winner=p1;
            loser=p2;
            loserTank=m_world.GetTankTwo();
        }
        else if (m_world.GetGameState().winnerPlayerId==p2->id)
        {
            //player 2 won
            winner=p2;
            loser=p1;
            loserTank=m_world.GetTankOne();
        }

        if (winner)
        {
            sl.append("Winner is: "+winner->name);

            if (loser && loserTank)
            {
                QString loserTxt=loser->name;
                switch (loserTank->deathCause)
                {
                case Tank::None:
                    loserTxt+=" dead by unknown reason!";
                    break;
                case Tank::HitMissile:
                    loserTxt+=" was hit by a missile!";
                    break;
                case Tank::HitMine:
                    loserTxt+=" hit a mine!";
                    break;
                case Tank::HitTank:
                    loserTxt+=" drove into another tank!";
                    break;
                case Tank::HitWall:
                    loserTxt+=" drove into a wall!";
                    break;
                }
                sl.append(loserTxt);
            }
        }
        else
        {
            sl.append("Winner has an unknown player id!");
        }
    }

    PaintText(sl, 18, 3, m_const.boardPixelSize.y()/2, painter);
}
