/*
 * gamemodel.cpp
 *
 *  Created on: Jun 22, 2015
 *      Author: grandmother
 */

#include "gamemodel.h"



Tank::Tank(QPointF pos, Direction direction)
	:playerId(0)
	,position(pos)
	,moveDirection(direction)
	,oldMoveDirection(direction)
	,towerDirection(direction)
	,oldTowerDirection(direction)
	,fires(false)
	,explosion(NotInFlames)
	,deathCause(None)
	,paintPosition(pos)
	,paintTankAngle(DirectionToAngle(direction))
	,paintTowerAngle(DirectionToAngle(direction))
	,isWrapping(false)
	//,old_position(position)

	//,accepts_updates(true)

{
}

Dude::Dude(QPointF position, Direction direction):
	position(position),
	moveDirection(direction),
	visible(true),
	is_dead(false),
	paintPosition(position)
{
	walking_sprite.image=QPixmap(":/images/tux-anim.png");
	walking_sprite.lifeTime=500;
	for (int i=0; i < 4; i++)
	{
		walking_sprite.fragments.push_back(QRectF(i*72, 0, 72, 72));
	}

	walking_frames = 4;

	dead_sprite.image=QPixmap(":/images/animated-dead-tux.png");
	dead_sprite.lifeTime=500;
	for (int i=0; i < 2; i++)
	{
		dead_sprite.fragments.push_back(QRectF(i*72, 0, 72, 72));
	}

	dead_frames = 2;

	current_frame = 0;
	last_update = 0;

}

void Dude::updateFramecounter(SpriteData sprite ) const {
	qint64 elapsed_time = QDateTime::currentMSecsSinceEpoch() - last_update;
	if(current_frame >= (qint64)(sprite.fragments.size())){
		current_frame = 0;

	}else if(elapsed_time >= (qint64)(sprite.lifeTime/sprite.fragments.size())){
		current_frame = (current_frame + 1) % sprite.fragments.size();
		last_update = QDateTime::currentMSecsSinceEpoch();
	}
}



