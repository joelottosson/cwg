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
    std::string m_imageName;
	int m_layer;


	/**
	 * Creates a new passive group with no sound information.
	 *
	 * this is the worst constructor the world has ever laid eyes upon.
	 *
	 * it all should be fairly straight forward but the weird board function thing is a the function from the Board
	 * class used to retrive the positions of the silly things.
	 */
	PassiveGroup(MatchState match_state, char const* image_name, int frames, int width, int height, int life_time, int repetitions, int layer,double delay, boost::function<const Board::PointVec& ( const Board&)> board_function);

	virtual ~PassiveGroup();



	void clear();

	/**
	 * Initializes the media player. Has no effect if sound_enabled is false.
	 */
	void setSoundPlayer(char const* sound_file, bool sound_enabled,int volume);


    /*
     * This function removes sprites on the board only when the amount of objects in this class differs from the amount on the board.
     * Will also play sound if sound is enabled and the sound is not already playing.
     */
	void updateGroupOnChange(Board board,GameState game_state,std::multimap<qint64, boost::function<void()>>&  events_queue,double delay);
	void updateGroupOnChange(Board board,GameState game_state,std::multimap<qint64, boost::function<void()>>&  events_queue);


	bool operator<(const PassiveGroup& other) const;
	void updateSprites();
private:

	int m_repetitions; //0 means forever. (could be a really crappy song title or the name for a Bond movie.)



	boost::function<const Board::PointVec& ( const Board&)> m_lol_function;

	SpriteData m_sprite_data;
	bool m_sound_enabled;
	QMediaPlayer m_media_player;

	double m_update_delay;



};

#endif /* SRC_TANK_ENGINE_SRC_TANK_ENGINE_PASSIVEGROUP_H_ */
