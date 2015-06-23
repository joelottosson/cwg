/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include <boost/make_shared.hpp>
#include <QtGui>
#include <qstyleoption.h>
#include "sprite.h"
#include "tankgamewidget.h"

namespace SDob = Safir::Dob::Typesystem;
namespace CWG= Consoden::TankGame;


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
	,m_dudes(":/images/tiny-tux.png")
	,m_dead_dude(":/images/dead-tux.png")
    ,m_poison(":/images/poison.png")
{


    this->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    Reset();



}

void TankGameWidget::Reset()
{
    UpdatePaintConstants();

    m_backgroundPainter.reset();
    m_backgroundPixmap.reset(new QPixmap(m_const.boardPixelSizeFloat.x()+2, m_const.boardPixelSizeFloat.y()+2));
    m_backgroundPixmap->fill(Qt::transparent);
    m_backgroundPainter.reset(new QPainter(m_backgroundPixmap.get()));
    m_backgroundPainter->setRenderHint(QPainter::Antialiasing);
    PaintGrid(*m_backgroundPainter);
    PaintWalls(*m_backgroundPainter);
}

void TankGameWidget::UpdatePaintConstants()
{
    m_const.boardPixelSizeFloat.setX(m_const.squarePixelSize*m_world.GetGameState().size.x());
    m_const.boardPixelSizeFloat.setY(m_const.squarePixelSize*m_world.GetGameState().size.y());
    m_const.boardPixelSizeInt.setX(static_cast<int>(m_const.boardPixelSizeFloat.x()));
    m_const.boardPixelSizeInt.setY(static_cast<int>(m_const.boardPixelSizeFloat.y()));
    if (m_const.boardPixelSizeFloat.y()<height())
    {
        m_scale=1.0;
    }
    else
    {
        //perform auto scale
        m_scale=(height()-10.0)/m_const.boardPixelSizeFloat.y();
    }

    m_const.upperLeft.setX(width()/2-(m_const.boardPixelSizeFloat.x()*m_scale)/2);
    m_const.upperLeft.setY(height()/2-(m_const.boardPixelSizeFloat.y()*m_scale)/2);
}

void TankGameWidget::paintEvent(QPaintEvent*)
{
    UpdatePaintConstants();

    //Paint mines
    PaintMines(*m_backgroundPainter);

    QPixmap tmp(*m_backgroundPixmap);    
    QPainter painter(&tmp);
    painter.setRenderHint(QPainter::Antialiasing);

    PaintPoison(painter);

    if(m_world.GetGameState().dudes.size() == 0 ){
    	std::wcout << "AINT GOT NO DUDES!" << std::endl;
    }else{
    	PaintDudes(m_world.GetGameState().dudes.front(), painter);
    }

    //Paint tanks
    int blueTank=true;
    for (auto& tank : m_world.GetGameState().tanks)
    {
        PaintTank(tank, blueTank, painter);
        blueTank=!blueTank;
    }

    //Paint sprites
    for (auto& s : m_world.Sprites())
    {

        PaintSprite(s, painter);
        //s.killToggle();
    }

    //Paint missiles
    for (auto& vt : m_world.GetGameState().missiles)
    {
        PaintMissile(vt.second, painter);
    }

    if (m_world.GetGameState().paintWinner)
    {
        PaintWinner(painter);
    }

    for (auto& t : m_world.ScreenTexts())
    {
        PaintText(t, painter);
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
        painter.drawLine(0, i*m_const.squarePixelSize, m_const.boardPixelSizeFloat.x(), i*m_const.squarePixelSize);
    }
    //vertical
    for (int i=0; i<=m_world.GetGameState().size.x(); ++i)
    {
        painter.drawLine(i*m_const.squarePixelSize, 0, i*m_const.squarePixelSize, m_const.boardPixelSizeFloat.y());
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

void TankGameWidget::PaintPoison(QPainter& painter)
{
    for (const auto& pos : m_world.GetGameState().poison)
    {
        painter.drawPixmap(ToScreen(pos, 0, 0), m_poison);
    }
}

//TODO: Stuff added by me
/*
 * Draws and moves the dude.
 *
 * does the drawing without doing any animation or funny stuff :/
 *
 * gets called at every screen refresh
 *
 */
void TankGameWidget::PaintDudes(const Dude& dude, QPainter& painter)
{

	if(!dude.dying){

		painter.save();
        dude.updateFramecounter(dude.walking_sprite);
        QPainter::PixmapFragment pf=QPainter::PixmapFragment::create(ToScreen(dude.paintPosition, m_const.squarePixelSize/2, m_const.squarePixelSize/2),
                                                                     dude.walking_sprite.fragments[dude.current_frame], 1, 1, 0, 1);
        painter.drawPixmapFragments(&pf, 1, dude.walking_sprite.image);
		painter.restore();

	}else{

		painter.save();
		dude.updateFramecounter(dude.dead_sprite);
        QPainter::PixmapFragment pf=QPainter::PixmapFragment::create(ToScreen(dude.paintPosition, m_const.squarePixelSize/2, m_const.squarePixelSize/2),
                                                                     dude.dead_sprite.fragments[dude.current_frame], 1, 1, 0, 1);
        std::wcout << "The dude frame counter for dead dude is " << dude.current_frame << " and the size of the thing is "<< dude.dead_sprite.fragments.size()  << std::endl;
        painter.drawPixmapFragments(&pf, 1, dude.dead_sprite.image);
		painter.restore();
	}

}

void TankGameWidget::PaintMines(QPainter& painter)
{    
    for (const auto& mine : m_world.GetGameState().mines)
    {
        painter.drawPixmap(ToScreen(mine, 0, 0), m_mine);
    }
}

int TankGameWidget::CalculateWrappingCoordinate(int val, int maxVal, int boardSize)
{
    if (val<0)
        return val+boardSize;
    else if (val>maxVal)
        return val-boardSize;
    else
        return val;
}

void TankGameWidget::PaintTank(const Tank& tank, bool blueTank, QPainter& painter)
{

    if (tank.explosion==Destroyed)
    {

        const int xoffset=(m_const.squarePixelSize-m_tankWreck.width())/2;
        const int yoffset=(m_const.squarePixelSize-m_tankWreck.height())/2;
        const int x=xoffset+(tank.paintPosition.x())*m_const.squarePixelSize;
        const int y=yoffset+(tank.paintPosition.y())*m_const.squarePixelSize;
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

        if (tank.isWrapping)
        {
            const int wrapX=CalculateWrappingCoordinate(x, m_const.boardPixelSizeInt.x()-tankImage.width(), m_const.boardPixelSizeInt.x());
            const int wrapY=CalculateWrappingCoordinate(y, m_const.boardPixelSizeInt.y()-tankImage.height(), m_const.boardPixelSizeInt.y());
            painter.save();
            painter.translate(wrapX+tankImage.width()/2, wrapY+tankImage.height()/2);
            painter.rotate(DirectionToAngle(tank.moveDirection));
            painter.translate(tankImage.width()/-2, tankImage.height()/-2);
            painter.drawPixmap(0, 0, tankImage);
            painter.restore();
        }
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

        if (tank.isWrapping)
        {
            const int wrapX=CalculateWrappingCoordinate(x, m_const.boardPixelSizeInt.x()-m_tankTower.width(), m_const.boardPixelSizeInt.x());
            const int wrapY=CalculateWrappingCoordinate(y, m_const.boardPixelSizeInt.y()-m_tankTower.height(), m_const.boardPixelSizeInt.y());
            painter.save();
            painter.translate(wrapX+m_tankTower.width()/2, wrapY+m_tankTower.height()/2);
            painter.rotate(tank.paintTowerAngle);
            painter.translate(m_tankTower.width()/-2, tankImage.height()/-2);
            painter.drawPixmap(0, 0, m_tankTower);
            painter.restore();
        }
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

void TankGameWidget::PaintText(const ScreenText& txt, QPainter& painter)
{
    if (txt.Finished())
    {
        return; //shall not be rendered now
    }

    painter.save();
    painter.setPen(txt.Pen());
    QFont font=painter.font();
    font.setPixelSize(txt.FontSize());
    font.setBold(true);
    painter.setFont(font);

    int rowStep=3*txt.FontSize();

    if (txt.Position().x()<0 || txt.Position().y()<0)
    {
        int xPos=txt.Position().x();
        int yPos=txt.Position().y();
        if (xPos<0) xPos=0;
        if (yPos<0) yPos=0;
        QRect rect(xPos, yPos, m_const.boardPixelSizeFloat.x(), txt.FontSize());
        for (const QString& s : txt.Text())
        {
            painter.drawText(rect, Qt::AlignCenter, s);
            rect.setY(rect.y()+rowStep);
        }
    }
    else
    {
        auto pos=ToScreen(txt.Position(), 0, 0);
        for (const QString& s : txt.Text())
        {
            painter.drawText(pos, s);
            pos.setY(pos.y()+rowStep);
        }
    }

    painter.restore();
}

void TankGameWidget::PaintWinner(QPainter& painter)
{
//    QStringList sl;

//    if (m_world.GetGameState().winnerPlayerId==0)
//    {
//        sl.append("Game was a draw");
//    }
//    else
//    {
//        const Player* p1=m_world.GetPlayerOne();
//        const Player* p2=m_world.GetPlayerTwo();
//        if (!p1 || !p2)
//        {
//            return;
//        }

//        const Player* winner=NULL;
//        const Player* loser=NULL;
//        const Tank* loserTank=NULL;

//        if (m_world.GetGameState().winnerPlayerId==p1->id)
//        {
//            //player 1 won
//            winner=p1;
//            loser=p2;
//            loserTank=m_world.GetTankTwo();
//        }
//        else if (m_world.GetGameState().winnerPlayerId==p2->id)
//        {
//            //player 2 won
//            winner=p2;
//            loser=p1;
//            loserTank=m_world.GetTankOne();
//        }

//        if (winner)
//        {
//            sl.append("Current game winner: "+winner->name);

//            if (loser && loserTank)
//            {
//                QString loserTxt=loser->name;
//                switch (loserTank->deathCause)
//                {
//                case Tank::HitMissile:
//                    loserTxt+=" was hit by a missile!";
//                    break;
//                case Tank::HitMine:
//                    loserTxt+=" hit a mine!";
//                    break;
//                case Tank::HitTank:
//                    loserTxt+=" drove into another tank!";
//                    break;
//                case Tank::HitWall:
//                    loserTxt+=" drove into a wall!";
//                    break;
//                case Tank::None:
//                    loserTxt="Time limit expired.";
//                    break;
//                }
//                sl.append(loserTxt);
//            }
//        }
//        else
//        {
//            sl.append("Winner has an unknown player id!");
//        }
//    }

//    PaintText(sl, 18, 3, m_const.boardPixelSize.y()/2, painter);
}
