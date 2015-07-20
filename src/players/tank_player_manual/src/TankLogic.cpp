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
#include <iostream>
#include <stdio.h>
#include <thread>         // std::thread

const std::wstring TankLogic::PlayerName = L"manual"; //TODO: change to your team name

namespace CWG = Consoden::TankGame;
char input = '\n';
bool first = true;

bool alternate = false;


void TankLogic::MakeMove(Consoden::TankGame::GameStatePtr gameState)
{
    //-------------------------------------------------------
    //Example of a stupid tank logic:
    //Remove it and write your own brilliant version!
    //-------------------------------------------------------
    GameMap gm(m_ownTankId, gameState);
    //auto currentPosition=gm.OwnPosition();
    //auto enemyPosition=gm.EnemyPosition();
    //BfsHelper bfs(gameState, currentPosition);
    Consoden::TankGame::Direction::Enumeration moveDirection = CWG::Direction::Neutral;
    Consoden::TankGame::Direction::Enumeration towerDirection = CWG::Direction::Neutral;
    bool fire = false;
    bool drop_mine = false;
    bool fire_laser = false;
    bool deploy_smoke = false;
    bool fire_redeemer = false;
    int redeemer_timer = 0;


    char input = '\n';

    while(gm.TimeToNextMove() > 10 && !first){

		  // Set terminal to raw mode
		  system("stty raw");

		  // Wait for single character
		  input = getchar();
		  // Reset terminal to normal "cooked" mode
		  system("stty cooked");

		  if(!alternate){
			  switch(input){
				  case 'a':
					  moveDirection = CWG::Direction::Left;
					  //towerDirection = CWG::Direction::Left;
					  break;
				  case 'd':
					  moveDirection = CWG::Direction::Right;
					  //towerDirection = CWG::Direction::Right;
					  break;
				  case 'w':
					  moveDirection = CWG::Direction::Up;
					  //towerDirection = CWG::Direction::Up;
					  break;
				  case 's':
					  moveDirection = CWG::Direction::Down;
					  //towerDirection = CWG::Direction::Down;
					  break;
				  case 'e':
					  moveDirection = CWG::Direction::Neutral;
					  //towerDirection = CWG::Direction::Down;
					  break;

				  case 'h':
					  towerDirection = CWG::Direction::Left;
					  break;
				  case 'k':
					  towerDirection = CWG::Direction::Right;
					  break;
				  case 'u':
					  towerDirection = CWG::Direction::Up;
					  break;
				  case 'j':
					  towerDirection = CWG::Direction::Down;
					  break;

				  case 'f':
					  fire = true;
					  break;
				  case 'r':
					  deploy_smoke = true;
					  break;
				  case 'g':
					  fire = true;
					  fire_laser = true;
					  break;

				  case 't':
					  fire = true;
					  fire_redeemer = true;
					  redeemer_timer = 3;
					  break;

				  case 'q':
					  abort();
					  break;
				  case '0':
					  alternate = !alternate;
					  std::wcout << std::endl << "SWAPPED TO ALTERNATE" << std::endl;
					  break;
			  }
		  }else{
			  switch(input){
				  case '4':
					  moveDirection = CWG::Direction::Left;
					  towerDirection = CWG::Direction::Left;
					  break;
				  case '6':
					  moveDirection = CWG::Direction::Right;
					  towerDirection = CWG::Direction::Right;
					  break;
				  case '8':
					  moveDirection = CWG::Direction::Up;
					  towerDirection = CWG::Direction::Up;
					  break;
				  case '5':
					  moveDirection = CWG::Direction::Down;
					  towerDirection = CWG::Direction::Down;
					  break;
				  case '0':
					  alternate = !alternate;
					  std::wcout << std::endl << "SWAPPED TO REGULAR" << std::endl;
					  break;
			  }
		  }


    	  //SetJoystick(moveDirection, towerDirection, fire, drop_mine, fire_laser);
    }

    first = false;



    //Move our joystick.
    SetJoystick(moveDirection, towerDirection, fire, drop_mine, fire_laser,deploy_smoke,fire_redeemer,redeemer_timer);
}
