/*
 * PassiveGroup.h
 *
 * This is a class corresponding to a collections of passive objects such as: coins, ammo, poison etc.
 * This is nice to have due to the similar nature of them.
 *
 *  Created on: Jun 29, 2015
 *      Author: grandmother
 */

#ifndef SRC_TANK_ENGINE_SRC_TANK_ENGINE_PASSIVEGROUP_H_
#define SRC_TANK_ENGINE_SRC_TANK_ENGINE_PASSIVEGROUP_H_


#include "sprite.h"
#include <boost/function.hpp>
#include "boardparser.h"
#include "gamemodel.h"
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QMediaPlaylist>
#include <QDir>

class PassiveGroup {
public:


	MatchState m_match_state;
	PointVec m_positions;
	QPixmap m_image;
	std::vector<Sprite> m_sprites;
	int m_layer;


	/**
	 * Creates a new passive group with no sound information
	 */
	PassiveGroup(MatchState match_state, char* image_name, int frames, int width, int height, int life_time, int repetitions, int layer);
	virtual ~PassiveGroup();

	void clear();

	/**
	 * Initializes the media player. Has no effect if sound_enabled is false.
	 */
	void setSoundPlayer(char* sound_file, bool sound_enabled);

	void updateGroupOnChange(const PointVec& board_positions,GameState game_state, std::multimap<qint64, boost::function<void()>>&  events_queue);
	void updateGroupOnChange(const PointVec& board_positions,GameState game_state, std::multimap<qint64, boost::function<void()>>&  events_queue, double delay);

private:

	int m_repetitions; //0 means forever. (could be a really crappy song title or the name for a Bond movie.)


	SpriteData m_sprite_data;
	bool m_sound_enabled;
	QMediaPlayer m_media_player;



};

#endif /* SRC_TANK_ENGINE_SRC_TANK_ENGINE_PASSIVEGROUP_H_ */
