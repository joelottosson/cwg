/******************************************************************************
*
* Copyright Consoden AB, 2014
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

#include "gamemodel.h"
#include "sprite.h"

class GameWorld
{
public:
    GameWorld(int updateInterval);

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
    const int GetTotalNumberOfGames() const {return m_matchState.totalNumberOfGames;}
    const int GetCurrentGameNumber() const {return m_matchState.currentGameNumber;}
    const int GetPlayerOneTotalPoints() const {return m_matchState.playerOnePoints;}
    const int GetPlayerTwoTotalPoints() const {return m_matchState.playerTwoPoints;}

    const MatchState& GetMatchState() const {return m_matchState;}
    const GameState& GetGameState() const {return m_matchState.gameState;}
    const std::vector<Sprite>& Sprites() const {return m_sprites;}

    void SetTextBig(const QStringList& lines) {m_textBig=lines;}
    const QStringList& GetTextBig() const {return m_textBig;}

    void SetTextSmall(const QStringList& lines) {m_textSmall=lines;}
    const QStringList& GetTextSmall() const {return m_textSmall;}

private:
    MatchState m_matchState;
    PlayerMap m_players;
    const int m_animationUpdateInterval;
    qreal m_moveSpeed; //game speed, 1 square per second
    qreal m_towerSpeed;
    qint64 m_lastAnimationUpdate;
    std::vector<Sprite> m_sprites;
    SpriteData m_explosion;
    SpriteData m_tankFire;
    QStringList m_textBig;
    QStringList m_textSmall;

    typedef std::multimap<qint64, boost::function<void()> > WorldEvents;
    WorldEvents m_eventQueue;
    void HandleEventQueue(qint64 time);
    void InitMediaPlayers();

    QMediaPlayer m_fireMediaPlayer1;
    QMediaPlayer m_explosionMediaPlayer1;
    QMediaPlayer m_fireMediaPlayer2;
    QMediaPlayer m_explosionMediaPlayer2;

    inline void UpdateTowerAngle(qint64 timeToNextUpdate, qreal movement, Tank& tank);

    template <class T>
    inline void UpdatePosition(qint64 timeToNextUpdate, qreal movement, T& item)
    {
        if (item.position==item.paintPosition)
        {
            return;
        }

        if (timeToNextUpdate<=m_animationUpdateInterval)
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
                break;
            }
        }
    }

};

#endif
