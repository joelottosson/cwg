/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#ifndef GAMEWORLD_H
#define GAMEWORLD_H

#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QMediaPlaylist>
#include <QDir>
#include <iostream>
#include <boost/function.hpp>
#include <Consoden/TankGame/Player.h>
#include <Consoden/TankGame/Match.h>
#include <Consoden/TankGame/GameState.h>
#include <Consoden/TankGame/Joystick.h>
//#include <memory>
#include <queue>
#include <boost/make_shared.hpp>


#include "gamemodel.h"
#include "sprite.h"
#include "boardparser.h"
#include "screentext.h"
#include "PassiveGroup.h"
#include "../../Config.h"

class GameWorld
{
public:
	GameWorld(int updateInterval, bool soundEnabled,ConfigSystem::Config conf);



    std::vector<Sprite> getPassiveSprites() const;
    std::vector<boost::shared_ptr<PassiveGroup>>  getPassiveGroups() const;

    ConfigSystem::Config m_c;

    void Clear();
    void ClearGameState();

    void Reset(const Consoden::TankGame::MatchPtr& match, boost::int64_t id);
    void Reset(const Consoden::TankGame::GameStatePtr& game, boost::int64_t id);

    void Update();
    void Update(const Consoden::TankGame::MatchPtr& match);
    void Update(const Consoden::TankGame::GameStatePtr& game);
    void Update(const Consoden::TankGame::JoystickConstPtr& joystick);



    qint64 MatchId() const {return m_matchState.machId;}
    qint64 GameId() const {return m_matchState.gameState.gameId;}
    bool MatchFinished() const;

    void AddPlayer(const Consoden::TankGame::PlayerConstPtr player, qint64 id);
    void DeletePlayer(qint64 id);
    const PlayerMap& GetPlayers() const {return m_players;}
    const Player* GetPlayerByName(const QString& name) const;
    const Player* GetPlayerById(qint64 id) const;
    const Player* GetPlayerOne() const;
    const Player* GetPlayerTwo() const;
    const Joystick* GetJoystickOne() const;
    const Joystick* GetJoystickTwo() const;
    const Tank* GetTankOne() const;
    const Tank* GetTankTwo() const;
    const int GetPlayerOneTotalPoints() const {return m_matchState.playerOnePoints;}
    const int GetPlayerTwoTotalPoints() const {return m_matchState.playerTwoPoints;}

    const MatchState& GetMatchState() const {return m_matchState;}
    const GameState& GetGameState() const {return m_matchState.gameState;}

    const std::vector<Sprite>& Sprites() const {return m_sprites;}

    void SetTextBig(const QStringList& lines);
    const std::vector<ScreenText>& ScreenTexts() const {return m_screenText;}




private:
    MatchState m_matchState;
    PlayerMap m_players;
    const int m_animationUpdateInterval;
    const bool m_soundEnabled;
    qreal m_moveSpeed; //game speed, 1 square per second
    qreal m_towerSpeed;

    //Essentially the number of pixels per ingame unit of distance. Does not take the scaling of the screen into account.
    const qreal m_pixels_per_square;

    qint64 m_lastAnimationUpdate;
    std::vector<Sprite> m_sprites;
    SpriteData m_explosion;
    SpriteData m_tankFire;
    SpriteData m_laser_middle;
    SpriteData m_laser_start;
    SpriteData m_dude;
    SpriteData m_smoke;
    std::vector<ScreenText> m_screenText;

    std::vector<boost::shared_ptr<PassiveGroup>> m_passive_objects;




    typedef std::multimap<qint64, boost::function<void()> > WorldEvents;
    WorldEvents m_eventQueue;
    void HandleEventQueue(qint64 time);
    void InitMediaPlayers();

    int wrap(int pos, int size);

    QMediaPlayer m_fireMediaPlayer1;
    QMediaPlayer m_explosionMediaPlayer1;
    QMediaPlayer m_smokeMediaPlayer;
    QMediaPlayer m_fireMediaPlayer2;
    QMediaPlayer m_explosionMediaPlayer2;
    QMediaPlayer m_dude_dies_MediaPlayer;
	QMediaPlayer m_laser_fire_MediaPlayer;


    void SetTextSmall(const QStringList& lines);
    void SetTextPlayer(int playerNumber, const QStringList& lines);
    void clearPassiveObjects();

    //TODO: Having all of these silly things inlined makes absolutely no sense.
    inline void UpdateMissiles(const Consoden::TankGame::GameStatePtr &game);
    inline void UpdateRedeemers(const Consoden::TankGame::GameStatePtr &game);
    inline void UpdateTowerAngle(qint64 timeToNextUpdate, qreal movement, Tank& tank);
    inline void UpdateCoins(const Board& board);
    inline void UpdatePoison(const Board& board);
    inline void UpdateLaserAmmo(const Board& boardParser);
    inline void UpdatePoints(const Consoden::TankGame::MatchPtr& match);
    inline void UpdateTank(const Consoden::TankGame::TankPtr& tank,const Board& board);
    inline void UpdateDude(const Consoden::TankGame::DudePtr& dude);
    inline void DrawLaser(const Consoden::TankGame::TankPtr& tank,const Board& board);
    void BadassExplosion(Redeemer& redeemer, int radius);


    inline void UpdateDudes(const Board& board);


    //QPointF calculateColisionPosition(Tank& own, Tank& enemy);
    //void calculateColisionPosition(Tank& own, Tank& enemy);
    QPointF calculateColisionPosition(Tank& own, Tank& enemy);
    QPointF directionToVector(Direction dir);
    const char* directionToString(Direction dir);
    void collisionOverride(Tank& own, Tank& enemy);
    qreal timeToEvent(QPointF a, QPointF b,qreal speed);
    qreal manhattanDistanceOnTorus(QPointF a, QPointF b);
    qreal simpleDistance(QPointF a, QPointF b);

    inline void UpdateTankWrapping(const Consoden::TankGame::TankPtr& tank, Tank& lastVal);

    /**
     * This function updates the paint position of the item according to its movement and speed.
     * paint position will be set to the actual position of the item even
     * if the movement is not finished.
     *
     * timeToNextUpdate: The time to the next game update update.
     * movement: The distance (in board squares) that the item will move within this state update.
     * item: The thing to be moved
     *
     *
     */
    template <class T>
        inline void UpdatePosition(qint64 timeToNextUpdate, qreal movement, T& item){
    	UpdatePosition(timeToNextUpdate, movement, item,true);
    }

    /**
     * This function updates the paint position of the item according to its movement and speed.
     *
     * timeToNextUpdate: The time to the next game update update (irrelevant if force_set is true)
     * movement: The distance (in board squares) that the item will move within this state udpdate.
     * item: The thing to be moved
     * force_set: if true the paint position will be set to the actual position of the item even
     * if the movement is not finished
     *
     */
    template <class T>
    inline void UpdatePosition(qint64 timeToNextUpdate, qreal movement, T& item,bool force_set)
    {
        if (item.position==item.paintPosition)
        {
            return;
        }

        if (timeToNextUpdate<=m_animationUpdateInterval && force_set)
        {
            item.paintPosition=item.position;
        }
        else
        {
            switch (item.moveDirection)
            {
            case LeftHeading:
            {
                qreal prev=item.paintPosition.x();
                item.paintPosition.setX(prev-movement);
            }
                break;
            case RightHeading:
            {
                qreal prev=item.paintPosition.x();
                item.paintPosition.setX(prev+movement);
            }
                break;
            case UpHeading:
            {
                qreal prev=item.paintPosition.y();
                item.paintPosition.setY(prev-movement);
            }
                break;
            case DownHeading:
            {
                qreal prev=item.paintPosition.y();
                item.paintPosition.setY(prev+movement);
            }
                break;
            case None:
            	item.paintPosition=item.position;
                break;
            }
        }
    }

    /**
        * This function updates the paint position of the item according to its movement and speed.
        *
        * timeToNextUpdate: The time to the next game update update (irrelevant if force_set is true)
        * movement: The distance (in board squares) that the item will move within this state udpdate.
        * item: The thing to be moved
        * force_set: if true the paint position will be set to the actual position of the item even
        * if the movement is not finished
        *
        */
       template <class T>
       inline void UpdatePositionNoOvershoot(qint64 timeToNextUpdate, qreal movement, T& item,bool force_set){
    	    {
    	        if (item.position==item.paintPosition)
    	        {
    	            return;
    	        }

    	        if (timeToNextUpdate<=m_animationUpdateInterval && force_set)
    	        {
    	            item.paintPosition=item.position;
    	        }
    	        else
    	        {
    	            switch (item.moveDirection)
    	            {
    	            case LeftHeading:
    	            {
    	                qreal prev=item.paintPosition.x();
    	                item.paintPosition.setX(prev-movement);
    	                if (item.paintPosition.x()<item.position.x())
    	                {
    	                    item.paintPosition=item.position;
    	                }
    	            }
    	                break;
    	            case RightHeading:
    	            {
    	                qreal prev=item.paintPosition.x();
    	                item.paintPosition.setX(prev+movement);
    	                if (item.paintPosition.x()>item.position.x())
    	                {
    	                    item.paintPosition=item.position;
    	                }
    	            }
    	                break;
    	            case UpHeading:
    	            {
    	                qreal prev=item.paintPosition.y();
    	                item.paintPosition.setY(prev-movement);
    	                if (item.paintPosition.y()<item.position.y())
    	                {
    	                    item.paintPosition=item.position;
    	                }
    	            }
    	                break;
    	            case DownHeading:
    	            {
    	                qreal prev=item.paintPosition.y();
    	                item.paintPosition.setY(prev+movement);
    	                if (item.paintPosition.y()>item.position.y())
    	                {
    	                    item.paintPosition=item.position;
    	                }
    	            }
    	                break;
    	            case None:
    	            	item.paintPosition=item.position;
    	                break;
    	            }
    	        }
    	    }

       }



};



#endif
