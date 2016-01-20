/*
 * PassiveGroup.cpp
 *
 *  Created on: Jun 29, 2015
 *      Author: grandmother
 */

#include "PassiveGroup.h"
#include "gameworld.h"


/**
 * Creates a new passive group with no sound information
 */
PassiveGroup::PassiveGroup(MatchState match_state, char const* image_name, int frames, int width, int height, int life_time, int repetitions, int layer,double delay, boost::function<const Board::PointVec& ( const Board&)> board_function){
	m_match_state = match_state;

	m_layer = layer;

	m_silly_image_name = image_name;
    m_image = QPixmap(image_name);
	m_sprite_data.lifeTime  = life_time;
	m_update_delay = delay;


	if(m_image.isNull()){
		std::wcout << "Could not load image " << image_name << "!!" << std::endl;
		abort();
	}
	if(m_image.width() == 0 || m_image.height() == 0){
		std::wcout << "image is of silly size :/" << std::endl;
		abort();
	}


	for (int i=0; i < frames; ++i)
	{
		m_sprite_data.fragments.push_back(QRectF(i*width, 0, width, height));
	}

	m_repetitions = repetitions;
	m_sprite_data.image = m_image;


	m_sound_enabled = false;

	m_lol_function = board_function;

}

PassiveGroup::PassiveGroup(MatchState match_state, char const* image_name, int frames, int width, int height, int life_time, int repetitions, int layer,double delay,char const* sound_file, bool sound_enabled,  boost::function<const Board::PointVec& ( const Board&)> board_function){
	PassiveGroup(match_state, image_name, frames, width, height ,life_time, repetitions, layer,delay, board_function);
	setSoundPlayer(sound_file,  sound_enabled,100);


}

/**
 * Initializes the media player. Has no effect if sound_enabled is false.
 */
void PassiveGroup::setSoundPlayer(char const* sound_file, bool sound_enabled,int volume){
	if(!sound_enabled){return;}

	const char* runtime=getenv("SAFIR_RUNTIME");
	QString path=QDir::cleanPath(QString(runtime)+QDir::separator()+"data"+QDir::separator()+"tank_game"+QDir::separator()+"sounds");
    m_media_player.setVolume(100);
    QString file_path=QDir::cleanPath(path+QDir::separator()+sound_file);
    m_media_player.setMedia(QUrl::fromLocalFile(file_path));
    m_sound_enabled = true;

}

void PassiveGroup::clear(){
	m_sprites.clear();
	m_positions.clear();
}

void  PassiveGroup::updateGroupOnChange(Board board,GameState game_state,std::multimap<qint64, boost::function<void()>>&  events_queue){
	updateGroupOnChange(board, game_state,events_queue,m_update_delay);
}


bool PassiveGroup::operator<(const PassiveGroup& other) const{
	return m_layer < other.m_layer;
}


void  PassiveGroup::updateGroupOnChange(Board board,GameState game_state,std::multimap<qint64, boost::function<void()>>&  events_queue,double delay){
	PointVec board_positions = m_lol_function(board);
    if (board_positions.size() != m_positions.size())
    {
        if (m_positions.empty())
        {

            m_positions.insert(m_positions.begin(), board_positions.begin(), board_positions.end());
            for (auto pos : m_positions)
            {
            	m_sprites.push_back(Sprite(m_sprite_data, pos, QDateTime::currentMSecsSinceEpoch(), m_repetitions));
            }

        }
        else
        {


            m_positions.clear();
            m_positions.insert(m_positions.begin(), board_positions.begin(), board_positions.end());

            events_queue.insert(std::multimap<qint64, boost::function<void()>>::value_type(game_state.lastUpdate+game_state.pace*delay, [=]
            {



                for (auto spriteIt=m_sprites.begin(); spriteIt!=m_sprites.end();)
                {

                    bool remove=true;
                    if (spriteIt->Data()==&m_sprite_data)
                    {
                        for (auto& pos : m_positions)
                        {
                            if (pos==spriteIt->Position())
                            {
                                remove=false;
                                break;
                            }
                        }
                    }

                    if (remove)
                    {
                        spriteIt=m_sprites.erase(spriteIt);
                    }
                    else
                    {
                        ++spriteIt;
                    }
                }

                //play sound
                if (m_sound_enabled)
                {
                	if(true || m_media_player.state() != m_media_player.PlayingState){
						m_media_player.stop();
						m_media_player.play();

                	}
                }
            }));
        }
    }
}


void PassiveGroup::updateSprites(){
    for (auto it=m_sprites.begin(); it!=m_sprites.end();)
    {
        if (it->Finished())
        {
            it=m_sprites.erase(it);
        }
        else
        {
            it->Update();

            ++it;
        }
    }
}



PassiveGroup::~PassiveGroup() {
	// TODO Auto-generated destructor stub
}

