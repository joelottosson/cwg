/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include "TankLogic.h"
#include "GameMap.h"
#include "BfsHelper.h"

const std::wstring TankLogic::PlayerName = L"tank_player_powerup"; //TODO: change to your team name

void TankLogic::MakeMove(Consoden::TankGame::GameStatePtr gameState)
{
    GameMap gm(m_ownTankId, gameState);
    auto currentPosition=gm.OwnPosition();
    auto enemyPosition=gm.EnemyPosition();
    BfsHelper bfs(gameState, currentPosition);
    Consoden::TankGame::Direction::Enumeration moveDirection;

    // Look for powerups and enemy
    int enemySteps = 20000;
    if (bfs.CanReachSquare(enemyPosition)) {
        enemySteps = bfs.StepsToSquare(enemyPosition);
    }

    int powerUpSteps = 20000;
    std::pair<int,int> powerUpPos;
    for (int x = 0; x < gm.SizeX(); x++) {
        for (int y = 0; y < gm.SizeY(); y++) {
            std::pair<int,int> pos = std::make_pair(x, y);
            if (!bfs.CanReachSquare(pos)) {
                continue;
            }

            if ((gm.IsLaserAmmo(pos) || gm.IsRedeemerAmmo(pos) || gm.IsSmokeGrenade(pos)) && powerUpSteps > bfs.StepsToSquare(pos)) {
                powerUpSteps = bfs.StepsToSquare(pos);
                powerUpPos = pos;
            }
        }
    }

    if (powerUpSteps < enemySteps) {
        // It is possible to get a power up, do it
        moveDirection=bfs.FindDirection(currentPosition, bfs.BacktrackFromSquare(powerUpPos));

    } else if (enemySteps < 20000) {
        // It is possible to move all the way to the enemy, do it
        moveDirection=bfs.FindDirection(currentPosition, bfs.BacktrackFromSquare(enemyPosition));

    } else {
        //Find an empty sqaure we can move to, otherwise stand still
        moveDirection=Consoden::TankGame::Direction::Neutral;
        if (!gm.IsWall(gm.Move(currentPosition, Consoden::TankGame::Direction::Left)) &&
            !gm.IsMine(gm.Move(currentPosition, Consoden::TankGame::Direction::Left)))
        {
            moveDirection=Consoden::TankGame::Direction::Left;
        }
        else if (!gm.IsWall(gm.Move(currentPosition, Consoden::TankGame::Direction::Right)) &&
                 !gm.IsMine(gm.Move(currentPosition, Consoden::TankGame::Direction::Right)))
        {
            moveDirection=Consoden::TankGame::Direction::Right;
        }
        else if (!gm.IsWall(gm.Move(currentPosition, Consoden::TankGame::Direction::Up)) &&
                 !gm.IsMine(gm.Move(currentPosition, Consoden::TankGame::Direction::Up)))
        {
            moveDirection=Consoden::TankGame::Direction::Up;
        }
        else if (!gm.IsWall(gm.Move(currentPosition, Consoden::TankGame::Direction::Down)) &&
                 !gm.IsMine(gm.Move(currentPosition, Consoden::TankGame::Direction::Down)))
        {
            moveDirection=Consoden::TankGame::Direction::Down;
        }
    }

    // Evaluate firing direction
    int opponent_x = gm.EnemyPosition().first;
    int opponent_y = gm.EnemyPosition().second;
    int player_x = gm.OwnPosition().first;
    int player_y = gm.OwnPosition().second;
    int x_distance;
    int x_wrap_distance;
    Consoden::TankGame::Direction::Enumeration x_direction;
    int y_distance;
    int y_wrap_distance;
    Consoden::TankGame::Direction::Enumeration y_direction;
    Consoden::TankGame::Direction::Enumeration tower_direction;

    if (opponent_x > player_x) {
        // opponent to the right
        x_distance = opponent_x - player_x;
        x_wrap_distance = player_x - opponent_x + gm.SizeX();
        if (x_distance < x_wrap_distance || !bfs.CanWrapX()) {
            x_direction = Consoden::TankGame::Direction::Right; //Shortest linear path to enemy is right
        } else {
            x_direction = Consoden::TankGame::Direction::Left; //Shortest linear path to enemy is left
            x_distance = x_wrap_distance;
        }
    } else {
        // opponent to the left
        x_distance = player_x - opponent_x;
        x_wrap_distance = opponent_x - player_x + gm.SizeX();
        if (x_distance < x_wrap_distance || !bfs.CanWrapX()) {
            x_direction = Consoden::TankGame::Direction::Left; //Shortest linear path to enemy is left
        } else {
            x_direction = Consoden::TankGame::Direction::Right; //Shortest linear path to enemy is right
            x_distance = x_wrap_distance;
        }            
    }

    if (opponent_y > player_y) {
        // opponent is down
        y_distance = opponent_y - player_y;
        y_wrap_distance = player_y - opponent_y + gm.SizeY();
        if (y_distance < y_wrap_distance || !bfs.CanWrapY()) {
            y_direction = Consoden::TankGame::Direction::Down; //Shortest linear path to enemy is down
        } else {
            y_direction = Consoden::TankGame::Direction::Up; //Shortest linear path to enemy is up
            y_distance = y_wrap_distance;
        }
    } else {
        // opponent is up
        y_distance = player_y - opponent_y;
        y_wrap_distance = opponent_y - player_y + gm.SizeY();
        if (y_distance < y_wrap_distance || !bfs.CanWrapY()) {
            y_direction = Consoden::TankGame::Direction::Up; //Shortest linear path to enemy is up
        } else {
            y_direction = Consoden::TankGame::Direction::Down; //Shortest linear path to enemy is down
            y_distance = y_wrap_distance;
        }            
    }

    if (x_distance > y_distance) {
        tower_direction = x_direction;
    } else {
        tower_direction = y_direction;
    }

    // Firing logic
    bool fire = false;
    bool fire_redeemer = false;
    int redemer_timer = 3;
    bool fire_laser = false;
    bool deploy_smoke = false;
    bool drop_mine = false;

    if (gm.HasSmoke()) {
        deploy_smoke = true;
    }

    if (gm.HasRedeemer()) {
        fire_redeemer = true;
        fire = true;
        redemer_timer = 4;
    } else if (gm.LaserAmmoCount() > 0) {
        fire_laser = true;
        fire = true;
        // Stand still to fire laser
        moveDirection = Consoden::TankGame::Direction::Neutral;
    } else {
        // Go for regular missile
        fire = true;
    }

    //Sometimes we also drop a mine
    drop_mine=(static_cast<int>(gameState->ElapsedTime().GetVal()) % 3)==0;

    //Move our joystick.
    SetJoystick(moveDirection, tower_direction, fire, drop_mine, fire_laser,deploy_smoke,fire_redeemer, redemer_timer);
}

