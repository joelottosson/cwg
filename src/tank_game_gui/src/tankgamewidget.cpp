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
#include "gameworld.h"

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
    //PaintMines(*m_backgroundPainter);

    QPixmap tmp(*m_backgroundPixmap);    
    QPainter painter(&tmp);
    painter.setRenderHint(QPainter::Antialiasing);

    //PaintPoison(painter);


    //Paint sprites
    for (auto& s : m_world.Sprites())
    {

        PaintSprite(s, painter);
        //s.killToggle();
    }

    std::priority_queue<PassiveGroup*>  passives = m_world.getPassiveGroups();
    std::priority_queue<PassiveGroup*> creepy_copy = passives;
    while(!creepy_copy.empty()){
    	PassiveGroup* a = creepy_copy.top();
    	creepy_copy.pop();
	    for (auto& s : a->m_sprites)
	    {

	        PaintSprite(s, painter);
	        //s.killToggle();
	    }
	}


    //Paint tanks
    int blueTank=true;
    for (auto& tank : m_world.GetGameState().tanks)
    {
        PaintTank(tank, blueTank, painter);
        blueTank=!blueTank;
    }



    if(m_world.GetGameState().dudes.size() != 0 ){
    	PaintDudes(m_world.GetGameState().dudes.front(), painter);
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

	qreal rotation = 0;
    if(tank.moveDirection == Direction::None){
    	rotation = DirectionToAngle(tank.oldMoveDirection);
    }else{
    	rotation = DirectionToAngle(tank.moveDirection);
    	tank.oldMoveDirection = tank.moveDirection;
    }



    if (tank.explosion==Destroyed)
    {
    	drawWithTranslationAndRotation(painter,m_tankWreck,tank.paintPosition,rotation);

    }

    const QPixmap& tankImage=blueTank ? m_tankBlue : m_tankRed;
    //tank body
    {
    	drawWithTranslationAndRotation(painter,tankImage,tank.paintPosition,rotation);

        if (tank.isWrapping)
        {
            const int xoffset=(m_const.squarePixelSize-tankImage.width())/2;
            const int yoffset=(m_const.squarePixelSize-tankImage.height())/2;
            const int x=xoffset+tank.paintPosition.x()*m_const.squarePixelSize;
            const int y=yoffset+tank.paintPosition.y()*m_const.squarePixelSize;
            const int wrapX=CalculateWrappingCoordinate(x, m_const.boardPixelSizeInt.x()-tankImage.width(), m_const.boardPixelSizeInt.x());
            const int wrapY=CalculateWrappingCoordinate(y, m_const.boardPixelSizeInt.y()-tankImage.height(), m_const.boardPixelSizeInt.y());
            painter.save();
            painter.translate(wrapX+tankImage.width()/2, wrapY+tankImage.height()/2);
            painter.rotate(rotation);
            painter.translate(tankImage.width()/-2, tankImage.height()/-2);
            painter.drawPixmap(0, 0, tankImage);
            painter.restore();
        }
    }

    //tank tower
    {        

    	drawWithTranslationAndRotation(painter,m_tankTower,tank.paintPosition,tank.paintTowerAngle);

        if (tank.isWrapping)
        {
        	const int xoffset=(m_const.squarePixelSize-m_tankTower.width())/2;
        	const int yoffset=(m_const.squarePixelSize-m_tankTower.height())/2;
        	const int x=xoffset+tank.paintPosition.x()*m_const.squarePixelSize;
        	const int y=yoffset+tank.paintPosition.y()*m_const.squarePixelSize;
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
    if (!missile.visible)
    {
        return;
    }
    drawWithTranslationAndRotation(painter,m_missile,missile.paintPosition,DirectionToAngle(missile.moveDirection));
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

/*
 *
 */
void TankGameWidget::drawWithTranslationAndRotation(QPainter& painter,QPixmap image,QPointF position, qreal rotation){
	const int xoffset=(m_const.squarePixelSize-image.width())/2;
	const int yoffset=(m_const.squarePixelSize-image.height())/2;
	const int x=xoffset+position.x()*m_const.squarePixelSize;
	const int y=yoffset+position.y()*m_const.squarePixelSize;
	painter.save();
	painter.translate(x+image.width()/2, y+image.height()/2);
	painter.rotate(rotation);
	painter.translate(image.width()/-2, image.height()/-2);
	painter.drawPixmap(0, 0, image);
	painter.restore();


}



void TankGameWidget::PaintWinner(QPainter& painter)
{
	//old and islly function
	return;
}
