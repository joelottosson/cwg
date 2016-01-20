/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include "TankLogic.h"

#include "BfsHelper.h"
#include "GameMap.h"
#include <boost/make_shared.hpp>
#include <utility>

#define SILLY_DEBUG true

const std::wstring TankLogic::PlayerName = L"grandmother"; //TODO: change to your team name
bool fired_last_round = false;

void TankLogic::MakeMove(Consoden::TankGame::GameStatePtr gameState)
{

	m_gm = boost::shared_ptr<GameMap>(new GameMap(m_ownTankId, gameState));

    auto currentPosition=m_gm->OwnPosition();
    auto enemyPosition=m_gm->EnemyPosition();
    BfsHelper bfs(gameState, currentPosition);
    Consoden::TankGame::Direction::Enumeration moveDirection = Consoden::TankGame::Direction::Neutral;
    Consoden::TankGame::Direction::Enumeration towerDirection;
    bool fire = false;
    bool escaping = false;

    auto escape_direction = escape();
    if(escape_direction != TG::Direction::Neutral){
    	std::wcout << "Escaping to the " << direction2String(escape_direction) << std::endl;
    	escaping = true;
    }


    towerDirection = isInSight();
    if(towerDirection != Consoden::TankGame::Direction::Neutral){
		fire = true;
    }



	if (bfs.CanReachSquare(enemyPosition)) {
		// It is possible to move all the way to the enemy, do it
		auto prefered = bfs.FindDirection(currentPosition, bfs.BacktrackFromSquare(enemyPosition));
		moveDirection = pickDirection(prefered,TG::Direction::Neutral, true, true);
	} else {
		moveDirection = pickDirection(TG::Direction::Neutral,TG::Direction::Neutral, true, true);
	}



	if(fire && !aligned(towerDirection,moveDirection)){
		moveDirection = Dir::Neutral;
	}

	if(!fire){towerDirection = moveDirection;}

	if(escaping){moveDirection = escape_direction;}

	SetJoystick(moveDirection, towerDirection, fire, false);

}


Consoden::TankGame::Direction::Enumeration TankLogic::isInSight(){

	auto own = m_gm->OwnPosition();
	auto enemy = m_gm->EnemyPosition();
	int xdiff = own.first - enemy.first;
	int ydiff = own.second - enemy.second;

	if(xdiff == 0){
		if(ydiff > 0){
			return Consoden::TankGame::Direction::Up;
		}else{
			return Consoden::TankGame::Direction::Down;
		}

	}else if(ydiff == 0){
		if(xdiff > 0){
			return Consoden::TankGame::Direction::Left;
		}else{
			return Consoden::TankGame::Direction::Right;
		}
	}else{
		return Consoden::TankGame::Direction::Neutral;
	}

}


TG::Direction::Enumeration TankLogic::pickDirection(TG::Direction::Enumeration preffered_direction,
		TG::Direction::Enumeration ignore_direction,bool conservative,bool choose_coin){

	//TODO: Shuffle list
	TG::Direction::Enumeration lame_array[4] = {TG::Direction::Left,TG::Direction::Right,TG::Direction::Up,TG::Direction::Down};
	TG::Direction::Enumeration candidate = TG::Direction::Neutral;
	std::pair<int,int> current_position = m_gm->OwnPosition();
	std::pair<int,int> new_pos;
	bool found_coin = false;
	bool found_prefered = false;

	for (int i = 0; i < 4; i++) {
		new_pos = m_gm->Move(current_position,lame_array[i]);
		if(m_gm->IsWall(new_pos) ||
				(conservative && m_gm->IsPoisonGas(new_pos)) ||
				m_gm->IsMine(new_pos) ||
				(m_gm->EnemyPosition() == new_pos)){
			continue;
		}

		if(lame_array[i] == ignore_direction){continue;}

		if(m_gm->IsCoin(new_pos)){

			candidate = lame_array[i];
			found_coin = true;
			if(choose_coin){break;}

		}

		if((lame_array[i] == preffered_direction)){

			found_prefered = true;
			if(m_gm->EnemyPosition() == new_pos){continue;}
			if(found_coin && choose_coin){break;}
			candidate = lame_array[i];
			if(!choose_coin){break;}

		}

		if(!found_prefered){candidate = lame_array[i];}
	}

	return candidate;

}

TG::Direction::Enumeration TankLogic::escape(){
	TG::Direction::Enumeration directions[4] = {TG::Direction::Left,TG::Direction::Right,TG::Direction::Up,TG::Direction::Down};
	bool safe[4] = {true,true,true,true};

	std::pair<int,int> position = m_gm->OwnPosition();
	std::pair<int,int> new_pos;


	for (int y = 1; y == 1; y++) {
		for (int x = 1; x == 1; x++) {

				if(x == 0 && y == 0){continue;}

				new_pos = posFromOffs(x,y,position);
				if(m_gm->EnemyPosition() == new_pos){

					if(x < 0){safe[0] = false;}
					if(x > 0){safe[1] = false;}
					if(y < 0){safe[2] = false;}
					if(y > 0){safe[3] = false;}

				}
				if(m_gm->IsMissileInPosition(new_pos)){
					if(x == -1 && y == -1){safe[0] = false; safe[2] = false; continue;}
					if(x == 1 && y == -1){safe[1] = false; safe[2] = false; continue;}
					if(x == -1 && y == 1){safe[0] = false; safe[3] = false; continue;}
					if(x == 1 && y == 1){safe[1] = false; safe[3] = false; continue;}
				}
			}
	}

	for (int direction = 0; direction < 4; direction++) {

			//std::wcout << "Tank is scared of "<< direction << "is" << safe[direction] << std::endl;

	}

	bool scarystuff = false;
	auto candidate = TG::Direction::Neutral;
	for (int direction = 0; direction < 4; direction++) {
		if(!safe[direction] || m_gm->IsWall(new_pos) || m_gm->IsMine(new_pos)){
			//std::wcout << "tank wont go to " << direction << std::endl;
			scarystuff = true;
		}else{
			//std::wcout << "tank thinks  " << direction << " is safe "<< std::endl;
			candidate =	directions[direction];
		}
	}

	if(scarystuff){
		return candidate;
	}else{
		return TG::Direction::Neutral;
	}

}

std::pair<int,int> TankLogic::posFromOffs(int x,int y,std::pair<int,int> pos){
	int new_x = (pos.first + x) % m_gm->SizeX();
	int new_y = (pos.first + y) % m_gm->SizeY();
	return std::make_pair(new_x,new_y);

}

bool TankLogic::aligned(TG::Direction::Enumeration a,TG::Direction::Enumeration b){
    	if(a == b){return true;}
    	if( (a == Dir::Left || a == Dir::Right) && (b == Dir::Left || b == Dir::Right)){return true;}
    	if( (a == Dir::Up || a == Dir::Down) && (b == Dir::Up || b == Dir::Down)){return true;}
    	return false;
    }

const char* TankLogic::direction2String(Dir::Enumeration dir){
	if(dir == Dir::Neutral){return "Neutral";}
	if(dir == Dir::Right){return "Right";}
	if(dir == Dir::Left){return "Left";}
	if(dir == Dir::Up){return "Up";}
	if(dir == Dir::Down){return "Down";}
	return "Dafaq????";
}




