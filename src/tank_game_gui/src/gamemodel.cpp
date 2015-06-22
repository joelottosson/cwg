/*
 * gamemodel.cpp
 *
 *  Created on: Jun 22, 2015
 *      Author: grandmother
 */

#include "gamemodel.h"

Dude::Dude(QPointF position, Direction direction):
	position(position),
	moveDirection(direction),
	visible(true),
	dying(false),
	paintPosition(position)
{
	walking_sprite.image=QPixmap(":/images/tux-anim.png");
	walking_sprite.lifeTime=500;
	for (int i=0; i < 4; ++i)
	{
		walking_sprite.fragments.push_back(QRectF(i*72, 0, 72, 72));
	}

	walking_frames = 4;

	dead_sprite.image=QPixmap(":/images/animated-dead-tux.png");
	dead_sprite.lifeTime=500;
	for (int i=0; i < 4; ++i)
	{
		dead_sprite.fragments.push_back(QRectF(i*72, 0, 72, 72));
	}

	dead_frames = 4;


	current_frame = 0;
	last_update = 0;

}

void Dude::updateFramecounter(SpriteData sprite ) const {
	qint64 elapsed_time = QDateTime::currentMSecsSinceEpoch() - last_update;
	if(elapsed_time >= (qint64)(sprite.lifeTime/sprite.fragments.size())){
		current_frame = (current_frame + 1) % sprite.fragments.size();
		last_update = QDateTime::currentMSecsSinceEpoch();
	}
}



