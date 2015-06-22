/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <QString>
#include <QPoint>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <boost/cstdint.hpp>
#include "sprite.h"

typedef std::vector<QPointF> PointVec;
enum Direction {LeftHeading, RightHeading, UpHeading, DownHeading, None};
enum ExplosionStatus {NotInFlames, SetInFlames, Burning, Destroyed};

inline qreal DirectionToAngle(Direction d)
{
    switch (d)
    {
    case RightHeading:
        return 0;
    case LeftHeading:
        return 180;
    case UpHeading:
        return 270;
    case DownHeading:
        return 90;
    default:
        return 0;
    }
}

//************************************
// Joystick
//************************************
struct Joystick
{

    qint64 playerId;
    int tankId;
    bool fire;
    Direction moveDirection;
    Direction towerDirection;

    Joystick()
        :fire(false)
        ,moveDirection(None)
        ,towerDirection(None)
    {
    }
};
typedef std::map<int, Joystick> JoystickMap;

//************************************
// Tank
//************************************
struct Tank
{
    enum Death {None, HitMissile, HitMine, HitTank, HitWall};

    qint64 playerId;
    QPointF position;
    Direction moveDirection;
    Direction towerDirection;
    bool fires;
    ExplosionStatus explosion;
    Death deathCause;


    //paintInfo
    QPointF paintPosition;
    qreal paintTankAngle;
    qreal paintTowerAngle;
    bool isWrapping;

    Tank(QPointF pos, Direction direction)
        :playerId(0)
        ,position(pos)        
        ,moveDirection(direction)
        ,towerDirection(direction)
        ,fires(false)
        ,explosion(NotInFlames)
        ,deathCause(None)
        ,paintPosition(pos)
        ,paintTankAngle(DirectionToAngle(direction))
        ,paintTowerAngle(DirectionToAngle(direction))
        ,isWrapping(false)
    {
    }
};
typedef std::vector<Tank> TankVec;

//************************************
// Missile
//************************************
struct Missile
{
    int tankId;
    QPointF position; //front position, might be outside board bounds
    Direction moveDirection;
    bool visible;
    ExplosionStatus explosion;

    bool paintFire;
    QPointF paintPosition;
};
typedef std::map<int, Missile> MissileMap;

//************************************
// Player
//************************************
struct Player
{
    QString name;
    qint64 id;
};
typedef std::map<qint64 /*playerId*/, Player> PlayerMap;

//************
// Dude
//************
struct Dude
{

    QPointF position; //position of the dude
    Direction moveDirection;
    bool visible;
    bool dying;
    QPointF paintPosition;
    SpriteData walking_sprite;
    SpriteData dead_sprite;
    int walking_frames;
    int dead_frames;
    mutable int current_frame = 0;
    mutable qint64 last_update;

    Dude(QPointF position, Direction direction);

    void updateFramecounter(SpriteData sprite ) const;



};
typedef std::vector<Dude> DudeMap;


//************************************
// GameState
//************************************
struct GameState
{
    qint64 gameId{0};
    QPoint size{10, 10};
    int pace{1000}; //milliseconds per square
    int elapsedTime{0};
    PointVec walls{};
    PointVec mines{};
    PointVec coins{};
    PointVec poison{};
    DudeMap dudes{};
    MissileMap missiles{};
    TankVec tanks{};
    JoystickMap joystics{};

    bool finished{false};
    bool paintWinner{false};
    qint64 winnerPlayerId; //0=draw

    qint64 lastUpdate; //timestamp in millisec
};

struct MatchState
{
    qint64 machId{0};
    qint64 players[2];
    bool finished{false};
    bool paintWinner{false};
    qint64 winnerPlayerId; //0=draw
    GameState gameState{};
    int totalNumberOfGames{0};
    int currentGameNumber{0};
    int playerOnePoints{0};
    int playerTwoPoints{0};
};

#endif // GAMEMODEL_H
