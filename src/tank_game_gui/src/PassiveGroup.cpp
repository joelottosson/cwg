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
PassiveGroup::PassiveGroup(MatchState match_state, char* image_name, int frames, int width, int height ,int lifetime, int repetitions, int layer){
	m_match_state = match_state;

	m_layer = layer;

    m_image = QPixmap(":/images/coin_sheet.png");
	m_sprite_data.lifeTime  = lifetime;


	if(m_image.isNull()){
		std::wcout << "DID_NOT_LOAD_IMAGE!!!!" << std::endl;
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
	//  abort();

}

/**
 * Initializes the media player. Has no effect if sound_enabled is false.
 */
void PassiveGroup::setSoundPlayer(char* sound_file, bool sound_enabled){
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

void  PassiveGroup::updateGroupOnChange(const PointVec& board_positions,GameState game_state,std::multimap<qint64, boost::function<void()>>&  events_queue){
	updateGroupOnChange(board_positions, game_state,events_queue,1.0);
}


void  PassiveGroup::updateGroupOnChange(const PointVec& board_positions,GameState game_state,std::multimap<qint64, boost::function<void()>>&  events_queue,double delay){
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
        	std::wcout << "We are inserting something startge here........." << std::endl;
        	std::wcout << "Our size is " << m_positions.size() << " whist the onboard isze is" <<  board_positions.size() << std::endl;
            events_queue.insert(std::multimap<qint64, boost::function<void()>>::value_type(game_state.lastUpdate+game_state.pace*delay, [=]
            {

                m_positions.clear();
                m_positions.insert(m_positions.begin(), board_positions.begin(), board_positions.end());

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
                	if(m_media_player.state() != m_media_player.PlayingState){
						m_media_player.stop();
						m_media_player.play();
						std::wcout << "WE_ARE_JUST_DOING_THIS_ONCE_FFS!!!!!!" << std::endl;
                	}
                }
            }));
        }
    }
}




PassiveGroup::~PassiveGroup() {
	// TODO Auto-generated destructor stub
}

