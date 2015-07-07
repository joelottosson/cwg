/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include "gameworld.h"
#include "PassiveGroup.h"
#include <boost/make_shared.hpp>
#include <memory>
#include <limits>


namespace CWG = Consoden::TankGame;
namespace
{
    Direction ToDirection(const Safir::Dob::Typesystem::ContainerProxy<Consoden::TankGame::Direction::EnumerationContainer>& ec)
    {
        if (ec.IsNull())
        {
            return None;
        }
        switch (ec.GetVal())
        {
        case Consoden::TankGame::Direction::Up: return UpHeading;
        case Consoden::TankGame::Direction::Down: return DownHeading;
        case Consoden::TankGame::Direction::Left: return LeftHeading;
        case Consoden::TankGame::Direction::Right: return RightHeading;
        case Consoden::TankGame::Direction::Neutral: return None;
        }
        return UpHeading;
    }
}

GameWorld::GameWorld(int updateInterval, bool soundEnabled)
    :m_matchState()
    ,m_players()
    ,m_animationUpdateInterval(updateInterval)
    ,m_soundEnabled(soundEnabled)
    ,m_moveSpeed(0)
    ,m_towerSpeed(0)
    ,m_lastAnimationUpdate(0)
    ,m_sprites()
    ,m_explosion()
    ,m_tankFire()
	,m_passive_objects()
    ,m_fireMediaPlayer1()
    ,m_explosionMediaPlayer1()
    ,m_fireMediaPlayer2()
    ,m_explosionMediaPlayer2()

{

    if (m_soundEnabled)
    {
        InitMediaPlayers();
    }

    //load sprite images
    /*m_explosion.image=QPixmap(":/images/explosion_sheet.png");
    m_explosion.lifeTime=1000;
    for (int i=0; i<11; ++i)
    {
        m_explosion.fragments.push_back(QRectF(i*72, 0, 72, 72));
    }*/

    m_explosion.image=QPixmap(":/images/awesome-explosion.png");
	m_explosion.lifeTime=2000;
	for(int j = 0; j < 7 ;j ++){
	for (int i=0; i<7; ++i)
		{
			m_explosion.fragments.push_back(QRectF(i*72, j*72, 72, 72));
		}
	}
/*    m_tankFire.image=QPixmap(":/images/tank_fire_sheet.png");
    m_tankFire.lifeTime=300;
    for (int i=0; i<5; ++i)
    {
        m_tankFire.fragments.push_back(QRectF(i*72, 0, 72, 72));
    }*/
    m_tankFire.image=QPixmap(":/images/cool_flame.png");
    m_tankFire.lifeTime=500;
	for(int j = 0; j <5 ;j ++){
	for (int i=0; i<5; ++i)
		{
			m_tankFire.fragments.push_back(QRectF(i*72, j*72, 72, 72));
		}
	}

	m_laser_middle.image=QPixmap(":/images/laser-middle.png");
	m_laser_middle.lifeTime=1000;
	m_laser_middle.fragments.push_back(QRectF(0, 0, 72, 72));

    m_laser_start.image=QPixmap(":/images/laser-start.png");
	m_laser_start.lifeTime=1000;
	m_laser_start.fragments.push_back(QRectF(0, 0, 72, 72));

	//m_matchState.gameState.


	//m_passive_objects.push(new PassiveGroup(m_matchState,":/images/coin_sheet.png", 8, 72, 72,1000,0,0,"coin.mp3",soundEnabled));
	PassiveGroup* glenn = new PassiveGroup(m_matchState,":/images/laser-ammo.png", 27, 66, 67,1200,0,0,0.75, &Board::LaserAmmo);
	glenn->setSoundPlayer("laser-pickup.mp3",soundEnabled);
	m_passive_objects.push_back(glenn);

	glenn = new PassiveGroup(m_matchState,":/images/coin_sheet.png", 8, 72, 72,1000,0,0,0.75, &Board::Coins);
	glenn->setSoundPlayer("coin.mp3",soundEnabled);
	m_passive_objects.push_back(glenn);

	glenn = new PassiveGroup(m_matchState,":/images/poison.png", 1, 72, 72,1000,0,0,0.75, &Board::Poison);
	glenn->setSoundPlayer("wilhelm_scream.mp3",soundEnabled);
	m_passive_objects.push_back(glenn);

	m_passive_objects.push_back(new PassiveGroup(m_matchState,":/images/mine.png", 1, 72, 72,1000,0,0,0.75, &Board::Mines));


}

std::vector<PassiveGroup*>  GameWorld::getPassiveGroups() const{
	return m_passive_objects;
}

void GameWorld::clearPassiveObjects(){
    for(auto a : m_passive_objects){
    	 a->clear();
    }
}

void GameWorld::Clear()
{
    m_matchState=MatchState();
    m_matchState.finished=true;
    m_sprites.clear();
    m_screenText.clear();
    clearPassiveObjects();



}

void GameWorld::ClearGameState()
{
    m_matchState.gameState=GameState();
    m_matchState.gameState.finished=true;
    m_sprites.clear();
    m_screenText.clear();
    clearPassiveObjects();

}

void GameWorld::Reset(const Consoden::TankGame::MatchPtr& match, boost::int64_t id)
{
    m_matchState=MatchState();
    m_matchState.machId=id;
    m_matchState.players[0]=match->PlayerOneId().GetVal().GetRawValue();
    m_matchState.players[1]=match->PlayerTwoId().GetVal().GetRawValue();
    m_matchState.totalNumberOfGames=match->TotalNumberOfGames();
    m_sprites.clear();
    m_screenText.clear();
    clearPassiveObjects();

    Update(match);


    if (m_matchState.currentGameNumber==1 && !m_matchState.finished)
    {
        QStringList sl;
        sl.append("Start new match");
        SetTextBig(sl);
    }
}

void GameWorld::Reset(const Consoden::TankGame::GameStatePtr &game, boost::int64_t id)
{
    m_matchState.gameState=GameState();
    m_matchState.gameState.gameId=id;
    m_matchState.gameState.lastUpdate=QDateTime::currentMSecsSinceEpoch();
    m_sprites.clear();
    m_screenText.clear();
    clearPassiveObjects();

    if (!game->GamePace().IsNull())
    {
        m_matchState.gameState.pace=static_cast<int>(game->GamePace().GetVal()*1000.0f);
    }

    Board boardParser(&game->Board().GetVal()[0], game->Width().GetVal(), game->Height().GetVal());
    m_matchState.gameState.size.setX(boardParser.GetXSize());
    m_matchState.gameState.size.setY(boardParser.GetYSize());
    m_matchState.gameState.walls.insert(m_matchState.gameState.walls.begin(), boardParser.Walls().begin(), boardParser.Walls().end());



    for (int i=0; i<game->TanksArraySize(); ++i)
    {
        if (!game->Tanks()[i].IsNull())
        {
            const Consoden::TankGame::TankConstPtr& tank=game->Tanks()[i].GetPtr();
            Tank t(QPointF(tank->PosX().GetVal(), tank->PosY().GetVal()), ToDirection(tank->MoveDirection()));
            t.fires=tank->Fire().GetVal();
            t.towerDirection=ToDirection(tank->TowerDirection());

            t.playerId=tank->PlayerId().GetVal().GetRawValue();
            if (tank->InFlames().GetVal())
            {
                t.explosion=SetInFlames;
            }

            m_matchState.gameState.tanks.push_back(t);
        }
    }

    CWG::DudePtr dude =  game->TheDude().GetPtr();
    Dude d(QPointF(dude->PosX().GetVal(), dude->PosY().GetVal()), ToDirection(dude->Direction()));
    m_matchState.gameState.dudes.push_back(d);

    if (game->Counter().GetVal()<=1)
    {
        QStringList sl;
        sl.append("Get ready for game "+QString::number(m_matchState.currentGameNumber)+" of "+QString::number(m_matchState.totalNumberOfGames));
        sl.append("Score: " + QString::number(GetPlayerOneTotalPoints())+" - "+QString::number(GetPlayerTwoTotalPoints()));
        SetTextBig(sl);
    }

    Update(game);
}

void GameWorld::UpdatePoints(const Consoden::TankGame::MatchPtr& match)
{
    int p1Diff=match->PlayerOneTotalPoints()-m_matchState.playerOnePoints;
    int p2Diff=match->PlayerTwoTotalPoints()-m_matchState.playerTwoPoints;
    m_matchState.playerOnePoints=match->PlayerOneTotalPoints();
    m_matchState.playerTwoPoints=match->PlayerTwoTotalPoints();

    if (p1Diff!=0 || p2Diff!=0)
    {
        m_eventQueue.insert(WorldEvents::value_type(m_matchState.gameState.lastUpdate+m_matchState.gameState.pace*0.75, [=]
        {
            //set player point text
            if (p1Diff!=0)
                SetTextPlayer(1, {QString("+")+QString::number(p1Diff)});
            if (p2Diff!=0)
                SetTextPlayer(2, {QString("+")+QString::number(p2Diff)});
        }));
    }
}

void GameWorld::Update(const Consoden::TankGame::MatchPtr& match)
{
    //Update if game is finished and if we have a winner
    m_matchState.finished=match->Winner().GetVal()!=Consoden::TankGame::Winner::Unknown;

    m_matchState.currentGameNumber=match->CurrentGameNumber();

    UpdatePoints(match);

    if (m_matchState.finished)
    {
        if (match->Winner()==Consoden::TankGame::Winner::PlayerOne)
            m_matchState.winnerPlayerId=m_matchState.players[0];
        else if (match->Winner()==Consoden::TankGame::Winner::PlayerTwo)
            m_matchState.winnerPlayerId=m_matchState.players[1];
        else
            m_matchState.winnerPlayerId=0;
    }
}


void GameWorld::UpdateTankWrapping(const Consoden::TankGame::TankPtr& tank, Tank& lastVal)
{
    //is wrapping in x-direction
    int xOld=lastVal.position.x();
    int xNew=tank->PosX();
    int xMax=m_matchState.gameState.size.x()-1;
    if (xOld==0 && xNew==xMax)
    {
        lastVal.isWrapping=true;
        return;
    }
    else if (xOld==xMax && xNew==0)
    {
        lastVal.isWrapping=true;
        return;
    }

    //is wrapping in y-direction
    int yOld=lastVal.position.y();
    int yNew=tank->PosY();
    int yMax=m_matchState.gameState.size.y()-1;
    if (yOld==0 && yNew==yMax)
    {
        lastVal.isWrapping=true;
        return;
    }
    else if (yOld==yMax && yNew==0)
    {
        lastVal.isWrapping=true;
        return;
    }

    lastVal.isWrapping=false;
}


void GameWorld::DrawLaser(const Consoden::TankGame::TankPtr& tank,const Board& board){
    int i=tank->TankId().GetVal();
    Tank& t=m_matchState.gameState.tanks[i];
    UpdateTankWrapping(tank, t);
    t.moveDirection=ToDirection(tank->MoveDirection());
    t.fires=tank->Fire().GetVal();
    t.towerDirection=ToDirection(tank->TowerDirection());


    qreal x_pos = t.position.x();
    qreal y_pos = t.position.y();

    double laser_delay = 0.0;


    qreal rot = 0;
    qreal dx = 0;
    qreal dy = 0;


    if(t.towerDirection != Direction::None){
    	dy = 0;
    	dx = 0;
    }

    Tank& enemy=m_matchState.gameState.tanks[(i + 1) %2];
    QPointF enemy_position = enemy.position;

    switch (t.moveDirection){
    	case Direction::UpHeading:
    		y_pos--;
    		laser_delay = 0.25;
    		//rot = 0;
    		break;
    	case Direction::DownHeading:
    		y_pos++;
    		laser_delay = 0.25;
			//rot = 180;
			break;
    	case Direction::LeftHeading:
    		x_pos--;
    		laser_delay = 0.25;
			//rot = 270;
			break;
    	case Direction::RightHeading:
    		x_pos++;
    		laser_delay = 0.25;
			//rot = 90;
			break;
    	default:

    		break;
    }


    switch (t.towerDirection){
    	case Direction::UpHeading:
    		dy = -1;
    		rot = 0;
    		break;
    	case Direction::DownHeading:
			dy = 1;
			rot = 180;
			break;
    	case Direction::LeftHeading:
			dx = -1;
			rot = 270;
			break;
    	case Direction::RightHeading:
			dx = 1;
			rot = 90;
			break;
    	default:
    		std::wcout << "Tried to fire laser without tower direction" << std::endl;
    		return;
    }


    m_sprites.push_back(Sprite(m_laser_start, QPointF(x_pos,y_pos),QPointF(0,0),rot, m_matchState.gameState.lastUpdate+m_matchState.gameState.pace*laser_delay, 1));

    while(true){
    	x_pos = wrap(x_pos + dx,m_matchState.gameState.size.x());
    	y_pos = wrap(y_pos + dy,m_matchState.gameState.size.y());
    	if(board.isWall(x_pos,y_pos)){
    		break;
    	}else if(enemy_position == QPointF(x_pos,y_pos)){
    		//Hit enemy tank. just chill
    		m_sprites.push_back(Sprite(m_laser_middle, QPointF(x_pos,y_pos),QPointF(0,0),rot, m_matchState.gameState.lastUpdate+m_matchState.gameState.pace*laser_delay, 1));
    		break;
    	}else if(t.position == QPointF(x_pos,y_pos)){
			//Hit our own tank tank. just chill
    		m_sprites.push_back(Sprite(m_laser_middle, QPointF(x_pos,y_pos),QPointF(0,0),rot, m_matchState.gameState.lastUpdate+m_matchState.gameState.pace*laser_delay, 1));
			break;
    	}
    	m_sprites.push_back(Sprite(m_laser_middle, QPointF(x_pos,y_pos),QPointF(0,0),rot, m_matchState.gameState.lastUpdate+m_matchState.gameState.pace*laser_delay, 1));

    }
    m_eventQueue.insert(WorldEvents::value_type(m_matchState.gameState.lastUpdate+m_matchState.gameState.pace*laser_delay, [=]
    {
        if (m_soundEnabled)
        {
            m_laser_fire_MediaPlayer.stop();
            m_laser_fire_MediaPlayer.play();
        }
    }));

}

void GameWorld::UpdateTank(const Consoden::TankGame::TankPtr& tank, const Board& board)
{



    int i=tank->TankId().GetVal();

    Tank& t=m_matchState.gameState.tanks[i];
    //UpdateTankWrapping(tank	, t);
    t.fires=tank->Fire().GetVal();
    t.towerDirection=ToDirection(tank->TowerDirection());

/*    if(tank->HitTank().IsNull()==false && tank->HitTank().GetVal()==true && t.explosion == NotInFlames){
    	calculateColisionPosition(t,m_matchState.gameState.tanks[(tank->TankId() + 1) % 2] );
    	t.explosion=SetInFlames;
    	return;
    }*/

    if(ToDirection(tank->MoveDirection()) == None){
    	t.moveDirection = t.oldMoveDirection;
    }else{
    	t.moveDirection=ToDirection(tank->MoveDirection());
    }
    t.oldMoveDirection = t.moveDirection;






    if(!tank->FireLaser().IsNull() && tank->FireLaser() ){
    	//std::wcout << "is fiering laser" << std::endl;
    	DrawLaser(tank,board);
    }

    //std::wcout << "tank position " << tank->PosX().GetVal() << "," << tank->PosY().GetVal()   << std::endl;
    if (tank->InFlames().GetVal()){
        switch (t.explosion)
        {
			case NotInFlames:
			{
				t.explosion=SetInFlames;

				if (t.deathCause==Tank::HitWall || t.deathCause==Tank::HitTank){
					return;
				}

				t.paintPosition=t.position;
				t.position=QPointF(tank->PosX().GetVal(), tank->PosY().GetVal());

				bool hasSpecialEndPos=false;
				qreal specialEndPos=0;

				//calculateColisionPosition(t,m_matchState.gameState.tanks[(tank->TankId() + 1) % 2]);

				if (tank->HitWall().IsNull()==false && tank->HitWall().GetVal()==true){
					t.deathCause=Tank::HitWall;
					specialEndPos=0.5;
					hasSpecialEndPos=true;

				}

				//if tank hit wall or drove into other tank, we calculate a position half inside the
				//next square before start explosion.
				if (hasSpecialEndPos){
					//didnt know this before, calculate a special end position inside wall
					switch (t.moveDirection)
					{
					case LeftHeading:
						t.position.setX(t.position.x()-specialEndPos);
						break;
					case RightHeading:
						t.position.setX(t.position.x()+specialEndPos);
						break;
					case UpHeading:
						t.position.setY(t.position.y()-specialEndPos);
						break;
					case DownHeading:
						t.position.setY(t.position.y()+specialEndPos);
						break;
					case None:
						break;
					}

					return;
				}
			}
				break;
			case SetInFlames:
				t.explosion=Burning;
				return;
				break;
			default:
				break;
        }



        if (!tank->HitWall().IsNull() && tank->HitWall()){
            t.deathCause=Tank::HitWall;
        }else if (!tank->HitMine().IsNull() && tank->HitMine()){
            t.deathCause=Tank::HitMine;
        }else if (!tank->HitMissile().IsNull() && tank->HitMissile()){
            t.deathCause=Tank::HitMissile;
            //t.position = t.position + directionToVector(t.moveDirection)*.5;
            //t.paintPosition = t.position;
        }else if (!tank->HitTank().IsNull() && tank->HitTank()){
            t.deathCause=Tank::HitTank;
        }else{
            t.deathCause=Tank::None;
        }

    }else if (t.explosion==NotInFlames && t.deathCause == Tank::None){

        t.paintPosition=t.position;
        t.position=QPointF(tank->PosX().GetVal(), tank->PosY().GetVal());


    }else{
    	t.explosion=Destroyed;

    }
}


/*
 * Method updating all of the entities.
 *
 * Gets called when the game state is updated (every second by default)
 *
 * Unfortunately it might appear as if this function needs to match the behavior of the function in Engine.cpp....whose name i have forgotten
 *
 */
void GameWorld::Update(const Consoden::TankGame::GameStatePtr &game)
{

    m_matchState.gameState.lastUpdate=QDateTime::currentMSecsSinceEpoch();
    m_matchState.gameState.elapsedTime=static_cast<int>(game->ElapsedTime().GetVal());

    Board boardParser(&game->Board().GetVal()[0], game->Width().GetVal(), game->Height().GetVal());

    //TODO fix so that no copying is required
    for(auto a : m_passive_objects){
    	a->updateGroupOnChange(boardParser ,m_matchState.gameState, m_eventQueue);
    }

    if(!game->TheDude().IsNull()){
    	auto& dude_game = game->TheDude().GetPtr();
    	Dude& dude = m_matchState.gameState.dudes.front();

    	if(!dude.is_dead && dude_game->Dying().GetVal() && m_soundEnabled){
    		m_dude_dies_MediaPlayer.play();
    	}else{
    		m_dude_dies_MediaPlayer.stop();
    	}

    	if(dude.just_died && dude_game->Dying() && !dude.is_dead){
    		dude.just_died = false;
    		dude.is_dead = true;
    	}

    	if(!dude.is_dead && dude_game->Dying()){
    		dude.just_died = true;
    		dude.position = dude.position-directionToVector(dude.moveDirection)*5;

    	}

    	if(!dude.is_dead){
    		dude.position.setX(dude_game->PosX());
    		dude.position.setY(dude_game->PosY());
    		dude.moveDirection = ToDirection(dude_game->Direction());
    	}


    }

    //Remove missiles that are removed
    for (MissileMap::const_iterator it=m_matchState.gameState.missiles.begin(); it!=m_matchState.gameState.missiles.end(); )
    {
        bool remove=true;
        for (Safir::Dob::Typesystem::ArrayIndex i=0; i < game->MissilesArraySize(); i++)
        {
            if (!game->Missiles()[i].IsNull() && game->Missiles()[i]->MissileId().GetVal()==it->first)
            {
                remove=false;
                break;
            }
        }

        if (remove)
        {
            m_matchState.gameState.missiles.erase(it++);
        }
        else
        {
            ++it;
        }
    }



    //We need to do a special check is the tanks are colliding with eachother.
    //If we don't do this the collision s will be completely wrong since the second tank may
    //change its position and direction after the collision has been evaluated for the first tank
    if(	!game->Tanks()[0].IsNull() && !game->Tanks()[1].IsNull() ){


		const Consoden::TankGame::TankPtr& tank0_ptr=game->Tanks()[0].GetPtr();
		const Consoden::TankGame::TankPtr& tank1_ptr=game->Tanks()[1].GetPtr();
		Tank& tank0 =m_matchState.gameState.tanks[0];
		Tank& tank1 =m_matchState.gameState.tanks[1];

		UpdateTankWrapping(tank0_ptr, tank0);
		UpdateTankWrapping(tank1_ptr, tank1);


		if(tank0.explosion == NotInFlames && tank1.explosion == NotInFlames &&
				tank0_ptr->HitTank().IsNull() == false  && tank0_ptr->HitTank() == true &&
				tank1_ptr->HitTank().IsNull() == false  && tank1_ptr->HitTank() == true
				){
			tank0.moveDirection = ToDirection(tank0_ptr->MoveDirection());
			tank1.moveDirection = ToDirection(tank1_ptr->MoveDirection());
			tank0.explosion = SetInFlames;
			tank1.explosion = SetInFlames;
			QPointF tank0_collison_position = calculateColisionPosition(tank0,tank1);
			QPointF tank1_collison_position = calculateColisionPosition(tank1,tank0);
			tank0.position = tank0_collison_position;
			tank1.position = tank1_collison_position;
			tank0.deathCause=Tank::HitTank;
			tank1.deathCause=Tank::HitTank;

		}else{

			UpdateTank(tank0_ptr, boardParser);
			UpdateTank(tank1_ptr, boardParser);

		}
    }

		//creates and updates missiles
		for (Safir::Dob::Typesystem::ArrayIndex i=0; i < game->MissilesArraySize(); i++)
		{
			if (game->Missiles()[i].IsNull())
			{
				continue;
			}




			const Consoden::TankGame::MissileConstPtr& missile=game->Missiles()[i].GetPtr();

			if(m_matchState.gameState.missiles.find(missile->MissileId().GetVal()) == m_matchState.gameState.missiles.end()){
				if(m_matchState.gameState.tanks[0].explosion != NotInFlames && m_matchState.gameState.tanks[1].explosion != NotInFlames){
					continue;
				}else{
					auto inserted=m_matchState.gameState.missiles.insert(std::make_pair(missile->MissileId().GetVal(), Missile()));
					Missile& m=inserted.first->second;
					m.tankId=missile->TankId();
					m.position=QPointF(missile->HeadPosX().GetVal(), missile->HeadPosY().GetVal());
					m.paintPosition=m.position;
					m.explosion=NotInFlames;
					m.visible=false;
					m.paintFire=true;


					m.moveDirection=ToDirection(missile->Direction());

					if (missile->InFlames().GetVal()){
						m.explosion = (m.explosion == NotInFlames) ? SetInFlames : Burning;

					}else if (m.explosion!=NotInFlames){
						m.explosion=Destroyed;
					}

					//continue;
				}
			}else{

				auto inserted=m_matchState.gameState.missiles.insert(std::make_pair(missile->MissileId().GetVal(), Missile()));
				Missile& m=inserted.first->second;
				//update of existing missile
				m.paintPosition=m.position;
				m.position=QPointF(missile->HeadPosX().GetVal(), missile->HeadPosY().GetVal());
				m.visible=true;



				m.moveDirection=ToDirection(missile->Direction());

				if (missile->InFlames().GetVal()){
					m.explosion = (m.explosion == NotInFlames) ? SetInFlames : Burning;

				}else if (m.explosion!=NotInFlames){
					m.explosion=Destroyed;
				}


	/*			if (inserted.second && m_matchState.gameState.tanks[0].explosion == NotInFlames && m_matchState.gameState.tanks[1].explosion == NotInFlames)
				{
					//new missile
					m.tankId=missile->TankId();
					m.position=QPointF(missile->HeadPosX().GetVal(), missile->HeadPosY().GetVal());
					m.paintPosition=m.position;
					m.explosion=NotInFlames;
					m.visible=false;
					m.paintFire=true;
				}
				else
				{
					//update of existing missile
					m.paintPosition=m.position;
					m.position=QPointF(missile->HeadPosX().GetVal(), missile->HeadPosY().GetVal());
					m.visible=true;
				}*/
			}


		}




    //Update if game is finished and if we have a winner
    if (game->Winner().IsNull() || game->Winner().GetVal()==Consoden::TankGame::Winner::Enumeration::Unknown)
    {
        m_matchState.gameState.finished=false;
    }
    else
    {
        m_matchState.gameState.finished=true;
        switch (game->Winner().GetVal())
        {
        case Consoden::TankGame::Winner::Enumeration::PlayerOne:
        {
            m_matchState.gameState.winnerPlayerId=game->PlayerOneId().GetVal().GetRawValue();
        }
            break;

        case Consoden::TankGame::Winner::Enumeration::PlayerTwo:
        {
            m_matchState.gameState.winnerPlayerId=game->PlayerTwoId().GetVal().GetRawValue();
        }
            break;

        default:
        {
            m_matchState.gameState.winnerPlayerId=0;
        }
            break;
        }

        //TODO: Reset  to +2 before release.
        auto paintGameEndTime=m_matchState.gameState.lastUpdate+0*m_matchState.gameState.pace;
        m_eventQueue.insert(WorldEvents::value_type(paintGameEndTime, [&]
        {
            //set paint winner, and then set a new event to remove text after 3 sec
            m_matchState.gameState.paintWinner=true;
        }));
    }
}

void GameWorld::Update(const Consoden::TankGame::JoystickConstPtr &joystick)
{
    if (m_matchState.gameState.gameId!=joystick->GameId().GetVal().GetRawValue())
    {
        return;
    }

    size_t tankId=static_cast<size_t>(joystick->TankId().GetVal());
    Joystick& js=m_matchState.gameState.joystics[tankId];
    //js.laser = joystick->FireLaser();
    js.tankId=joystick->TankId().GetVal();
    js.playerId=joystick->PlayerId().GetVal().GetRawValue();
    js.fire=joystick->Fire().IsNull() ? false : joystick->Fire().GetVal();
    js.laser=joystick->FireLaser().IsNull() ? false : joystick->FireLaser().GetVal();
    js.moveDirection=ToDirection(joystick->MoveDirection());
    js.towerDirection=ToDirection(joystick->TowerDirection());
}

/*
 *
 * Does the graphical update part. Gets called every time the screen refreshes.
 *
 */
void GameWorld::Update()
{
    m_moveSpeed=1.0/static_cast<double>(m_matchState.gameState.pace); //square per millisec
    m_towerSpeed=180.0/static_cast<double>(m_matchState.gameState.pace);

    qint64 now=QDateTime::currentMSecsSinceEpoch();
    qint64 nextUpdate=m_matchState.gameState.lastUpdate+m_matchState.gameState.pace;
    qint64 timeToNextUpdate=nextUpdate-now;
    qint64 timeSinceLastAnimationUpdate=now-m_lastAnimationUpdate;
    qreal movement=static_cast<qreal>(timeSinceLastAnimationUpdate)*m_moveSpeed;
    qreal angle=static_cast<qreal>(timeSinceLastAnimationUpdate)*m_towerSpeed;

    HandleEventQueue(now);

    for (auto& dude : m_matchState.gameState.dudes){
        if(dude.just_died){
        	UpdatePositionNoOvershoot(timeToNextUpdate, movement, dude,false);

        }else if(!dude.is_dead){
        	UpdatePosition(timeToNextUpdate, 1*movement, dude);
        }

    }



    //updates each tank.
    for (Tank& tank : m_matchState.gameState.tanks)
    {
    	//if(tank.explosion == SetInFlames){

    	//std::wcout << tank.playerId <<" explosion status is " <<  tank.explosion <<std::endl;

			if(tank.deathCause == tank.Death::HitWall){

				UpdatePositionNoOvershoot(timeToNextUpdate, movement, tank, false);
			}else if(tank.deathCause == tank.Death::HitTank){
				UpdatePositionNoOvershoot(timeToNextUpdate, movement, tank,false);
			}else if(tank.deathCause == tank.Death::HitMissile ){
				UpdatePositionNoOvershoot(timeToNextUpdate, movement, tank,false);
			}else{
				UpdatePosition(timeToNextUpdate, movement, tank);
			}


        UpdateTowerAngle(timeToNextUpdate, angle, tank);
        if (tank.explosion==SetInFlames)
        {
            //new explosion sprite
        	qint64 time;
        	if(tank.deathCause == tank.Death::HitTank){
        		time = now + timeToEvent(tank.position,tank.paintPosition,m_moveSpeed);
        	}else{
        		time = now + timeToNextUpdate/2;
        	}
        	srand(clock());
            m_sprites.push_back(Sprite(m_explosion, tank.position, now+timeToNextUpdate, 1));
            m_sprites.push_back(Sprite(m_explosion, QPointF(tank.position.x()+(((float)(rand() % 600)-300)/600), tank.position.y() + (((float)(rand() % 600)-300)/600)),
            		time + (rand() % 300), 1));
            m_sprites.push_back(Sprite(m_explosion, QPointF(tank.position.x()+(((float)(rand() % 600)-300)/600), tank.position.y() + (((float)(rand() % 600)-300)/600)),
            		time + (rand() % 300), 1));
            m_sprites.push_back(Sprite(m_explosion, QPointF(tank.position.x()+(((float)(rand() % 600)-300)/600), tank.position.y() + (((float)(rand() % 600)-300)/600)),
            		time + (rand() % 300), 1));


            tank.explosion=Burning;
            m_eventQueue.insert(WorldEvents::value_type(time, [&]
            {
                tank.explosion=Destroyed;
            }));

            if (m_soundEnabled)
            {
                auto tankPlayerId=tank.playerId;
                m_eventQueue.insert(WorldEvents::value_type(time, [=]
                {
                    auto p1=GetPlayerOne();
                    if (!p1)
                    {
                        return;
                    }
                    else if (p1->id==tankPlayerId)
                    {
                        m_explosionMediaPlayer1.stop();
                        m_explosionMediaPlayer1.play();
                    }
                    else
                    {
                        m_explosionMediaPlayer2.stop();
                        m_explosionMediaPlayer2.play();
                    }
                }));
            }
        }
    }

    /**
     * Appears to update the missiles
     */
    for (auto& vt : m_matchState.gameState.missiles)
    {
        Missile& missile=vt.second;
        UpdatePositionNoOvershoot(timeToNextUpdate, 2*movement, missile,false); //missiles have double speed
        Tank tank=m_matchState.gameState.tanks[missile.tankId];
        //std::wcout << "And the missile direction is " << directionToString(missile.moveDirection) << std::endl;
        if (missile.paintFire && missile.moveDirection != None)
        {
            QPointF firePos=missile.position;
            QPointF animationMoveSpeed(0,0);
            switch(missile.moveDirection)
            {
            case LeftHeading:
                animationMoveSpeed.setX(-1*m_moveSpeed);
                firePos.setX(firePos.x()+1);
                break;
            case RightHeading:
                animationMoveSpeed.setX(m_moveSpeed);
                firePos.setX(firePos.x()-1);
                break;
            case UpHeading:
                animationMoveSpeed.setY(-1*m_moveSpeed);
                firePos.setY(firePos.y()+1);
                break;
            case DownHeading:
                animationMoveSpeed.setY(m_moveSpeed);
                firePos.setY(firePos.y()-1);
                break;
            case None:
            	//std::wcout << "LOL FAIL " << std::endl;
                break;
            }


            QPointF flame_pos  = tank.position+directionToVector(tank.towerDirection);
            m_sprites.push_back(Sprite(m_tankFire, flame_pos, directionToVector(missile.moveDirection)*m_moveSpeed*2, DirectionToAngle(missile.moveDirection)+90, now +timeToNextUpdate, 1));
            missile.paintFire=false;

            if (m_soundEnabled)
            {
                qint64 missilePlayerId=m_matchState.gameState.tanks[missile.tankId].playerId;
                m_eventQueue.insert(WorldEvents::value_type(nextUpdate, [=]
                {
                    auto p1=GetPlayerOne();
                    if (!p1)
                    {
                        return;
                    }
                    else if (p1->id==missilePlayerId)
                    {
                        m_fireMediaPlayer1.stop();
                        m_fireMediaPlayer1.play();
                    }
                    else
                    {
                        m_fireMediaPlayer2.stop();
                        m_fireMediaPlayer2.play();
                    }
                }));
            }
        }



        if (missile.explosion==SetInFlames)
        {
            //qreal distanceToExplosion=QPointF(missile.position.x()-missile.paintPosition.x(), missile.position.y()-missile.paintPosition.y()).manhattanLength();
            //qint64 explosionTime=static_cast<qint64>(distanceToExplosion/(2*m_moveSpeed));
            m_sprites.push_back(Sprite(m_explosion, missile.position, now+timeToNextUpdate, 1));
            missile.explosion=Burning;

            if (m_soundEnabled)
            {
                qint64 missilePlayerId=m_matchState.gameState.tanks[missile.tankId].playerId;
                m_eventQueue.insert(WorldEvents::value_type(nextUpdate, [=]
                {
                    auto p1=GetPlayerOne();
                    if (!p1)
                    {
                        return;
                    }
                    else if (p1->id==missilePlayerId)
                    {
                        m_explosionMediaPlayer1.stop();
                        m_explosionMediaPlayer1.play();
                    }
                    else
                    {
                        m_explosionMediaPlayer2.stop();
                        m_explosionMediaPlayer2.play();
                    }
                }));
            }
        }
    }








    for(auto a : m_passive_objects){
    	a->updateSprites();
    }


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

    for (auto it=m_screenText.begin(); it!=m_screenText.end();)
    {
        if (it->Finished())
        {
            it=m_screenText.erase(it);
        }
        else
        {
            it->Update();
            ++it;
        }
    }

    m_lastAnimationUpdate=QDateTime::currentMSecsSinceEpoch();
}

bool GameWorld::MatchFinished() const
{
    if (m_matchState.finished && m_eventQueue.empty())
    {
        for (auto& sprite : m_sprites)
        {
            bool spriteFinished=sprite.Repetitions()==0 || sprite.Finished();
            if (!spriteFinished)
            {
            	// std::wcout << "We removed some sprajtz" << std::endl;
                return false; //there are still sprites that will not run forever that has not finished
            }
        }

        return true; //all sprites that are not forever have finished
    }
    return false;
}

void GameWorld::AddPlayer(const Consoden::TankGame::PlayerConstPtr player, qint64 id)
{
    Player p;
    p.id=id;
    p.name=QString::fromWCharArray( player->Name().GetVal().c_str() );
    m_players.insert(std::make_pair(p.id, p));
}

void GameWorld::DeletePlayer(qint64 id)
{
   m_players.erase(id);
}

const Player* GameWorld::GetPlayerByName(const QString& name) const
{
    for (auto& vt : m_players)
    {
        if (vt.second.name==name)
            return &(vt.second);
    }
    return nullptr;
}

void GameWorld::HandleEventQueue(qint64 time)
{
    for (auto it=m_eventQueue.begin(); it!=m_eventQueue.end(); ++it)
    {
        if (it->first>time)
        {
            m_eventQueue.erase(m_eventQueue.begin(), it); //delete handled events
            return;
        }
        it->second(); //execute event handler
    }
    m_eventQueue.clear();
}

void GameWorld::InitMediaPlayers()
{
    m_fireMediaPlayer1.setVolume(35);
    m_explosionMediaPlayer1.setVolume(80);
    m_fireMediaPlayer2.setVolume(35);
    m_explosionMediaPlayer2.setVolume(80);

    const char* runtime=getenv("SAFIR_RUNTIME");
    QString path=QDir::cleanPath(QString(runtime)+QDir::separator()+"data"+QDir::separator()+"tank_game"+QDir::separator()+"sounds");
    QString firePath=QDir::cleanPath(path+QDir::separator()+"shotgun.mp3");
    QString explostionPath=QDir::cleanPath(path+QDir::separator()+"explosion.mp3");
    QString gunPath=QDir::cleanPath(path+QDir::separator()+"gun.mp3");
    QString bigBombPath=QDir::cleanPath(path+QDir::separator()+"big_bomb.mp3");
    QString death_of_dude=QDir::cleanPath(path+QDir::separator()+"dude-dies.mp3");
    QString laser_fire=QDir::cleanPath(path+QDir::separator()+"laser-fire.mp3");

    m_fireMediaPlayer1.setMedia(QUrl::fromLocalFile(firePath));
    m_explosionMediaPlayer1.setMedia(QUrl::fromLocalFile(explostionPath));
    m_fireMediaPlayer2.setMedia(QUrl::fromLocalFile(gunPath));
    m_explosionMediaPlayer2.setMedia(QUrl::fromLocalFile(bigBombPath));
    m_dude_dies_MediaPlayer.setMedia(QUrl::fromLocalFile(death_of_dude));
    m_laser_fire_MediaPlayer.setMedia(QUrl::fromLocalFile(laser_fire));
}

void GameWorld::UpdateTowerAngle(qint64 timeToNextUpdate, qreal movement, Tank& tank)
{

	qreal endAngle = DirectionToAngle(tank.towerDirection);

/*	std::wcout << "Tower direction is     " << tank.towerDirection << std::endl;
	std::wcout << "Old Tower direction is " << tank.towerDirection << std::endl;
	std::wcout << std::endl;

	if(tank.towerDirection == Direction::None){
		endAngle = DirectionToAngle(tank.oldTowerDirection);
	}else{
		endAngle = DirectionToAngle(tank.towerDirection);
		tank.oldTowerDirection = tank.towerDirection;
	}*/


    if (timeToNextUpdate<=m_animationUpdateInterval)
    {
        tank.paintTowerAngle=endAngle;
        return;
    }

    if (tank.paintTowerAngle==endAngle)
    {
        return;
    }

    int sign=(endAngle>tank.paintTowerAngle) ? 1 : -1;

    if (endAngle==0 && tank.paintTowerAngle>180.0)
    {
        sign=1;
        tank.paintTowerAngle-=360.0;
    }
    else if (endAngle>180.0 && tank.paintTowerAngle<90.0)
    {
        sign=-1;
        tank.paintTowerAngle+=360.0;
    }


    tank.paintTowerAngle+=(sign*movement);

    if ((sign>0 && tank.paintTowerAngle>endAngle) || (sign<0 && tank.paintTowerAngle<endAngle))
    {
        tank.paintTowerAngle=endAngle;
    }
}

const Player* GameWorld::GetPlayerById(qint64 id) const
{
    auto it=m_players.find(id);
    if (it!=m_players.end())
    {
        return &(it->second);
    }
    return nullptr;
}

const Player* GameWorld::GetPlayerOne() const
{
    return GetPlayerById(m_matchState.players[0]);
}

const Player* GameWorld::GetPlayerTwo() const
{
    return GetPlayerById(m_matchState.players[1]);
}

const Joystick* GameWorld::GetJoystickOne() const
{
    const auto playerId=m_matchState.players[0];
    auto joystickIt=std::find_if(m_matchState.gameState.joystics.begin(), m_matchState.gameState.joystics.end(), [&](JoystickMap::value_type vt){return vt.second.playerId==playerId;});
    if (joystickIt!=m_matchState.gameState.joystics.end())
    {
        return &(joystickIt->second);
    }
    return nullptr;
}

const Joystick* GameWorld::GetJoystickTwo() const
{
    const auto playerId=m_matchState.players[1];
    auto joystickIt=std::find_if(m_matchState.gameState.joystics.begin(), m_matchState.gameState.joystics.end(), [&](JoystickMap::value_type vt){return vt.second.playerId==playerId;});
    if (joystickIt!=m_matchState.gameState.joystics.end())
    {
        return &(joystickIt->second);
    }
    return nullptr;
}

const Tank* GameWorld::GetTankOne() const
{
    const auto playerId=m_matchState.players[0];
    auto tankIt=std::find_if(m_matchState.gameState.tanks.begin(), m_matchState.gameState.tanks.end(), [&](const Tank& t){return t.playerId==playerId;});
    if (tankIt!=m_matchState.gameState.tanks.end())
    {
        return &(*tankIt);
    }
    return nullptr;
}

const Tank* GameWorld::GetTankTwo() const
{
    const auto playerId=m_matchState.players[1];
    auto tankIt=std::find_if(m_matchState.gameState.tanks.begin(), m_matchState.gameState.tanks.end(), [&](const Tank& t){return t.playerId==playerId;});
    if (tankIt!=m_matchState.gameState.tanks.end())
    {
        return &(*tankIt);
    }
    return nullptr;
}

void GameWorld::SetTextBig(const QStringList& lines)
{
    m_screenText.emplace_back(lines, QPointF(-1, 0), Qt::yellow, 30, 8, QPointF(0, 0), false, 3000);

}

void GameWorld::SetTextSmall(const QStringList& lines)
{
    m_screenText.emplace_back(lines, QPointF(-1, -1), Qt::yellow, 18, 3, QPointF(0, 0), false, 3000);
}

void GameWorld::SetTextPlayer(int playerNumber, const QStringList& lines)
{
    const Tank* t=nullptr;
    if (playerNumber==1)
    {
        t=GetTankOne();
    }
    else
    {
        t=GetTankTwo();
    }

    if (t)
    {
        m_screenText.emplace_back(lines,
                                  QPointF(t->position.x()+0.4, t->position.y()),
                                  Qt::yellow,
                                  30,
                                  8,
                                  QPointF(0, -0.5/1000),
                                  true,
                                  1000);
    }
}

int GameWorld::wrap(int pos, int size){
   	//return pos == 0 ? 0 : (size - std::abs(pos)*(size/std::abs(pos)));
	if(pos > size){
			return 0;
	}else if(pos < 0){
			return size - 1;
	}else{
		return pos;
	}
}

/*void GameWorld::calculateColisionPosition(Tank& own, Tank& enemy){

	qreal offs = 0.25;
	//QPointF diff = own.position - enemy.position;



	//QPoint diffint = (QPoint)diff;
	//std::wcout << "testing "<< diff.x() << "," << diff.y() << std::endl;
	std::wcout << " Own direction = " << directionToString(own.moveDirection) << std::endl;
	std::wcout << " enemy direction = " << directionToString(enemy.moveDirection)<< std::endl << std::endl;
	if(own.moveDirection == None){
		return;
	}

	if(enemy.moveDirection == None){

		switch (own.moveDirection)
		{
		case LeftHeading:
			own.position.setX(own.position.x()+offs);
			break;
		case RightHeading:
			own.position.setX(own.position.x()-offs);
			break;
		case UpHeading:
			own.position.setY(own.position.y()+offs);
			break;
		case DownHeading:
			own.position.setY(own.position.y()-offs);
			break;
		case None:
			break;
		}

		return;
	}else{
		//We need to do stuff difrently if tanks are colliding diagonaly.
		if((directionToVector(own.moveDirection).y() != 0 && directionToVector(enemy.moveDirection).y() == 0 ) ||
				(directionToVector(own.moveDirection).x() != 0 && directionToVector(enemy.moveDirection).x() == 0 )){
			own.position = (own.position - directionToVector(own.moveDirection)*(offs));
		}else{
			own.position = (own.position + directionToVector(own.moveDirection)*(offs));

		}
	}

}*/

QPointF GameWorld::calculateColisionPosition(Tank& own, Tank& enemy){

	qreal offs = 0.25;

	if(own.moveDirection == None){

		return own.position;
	}

	if(enemy.moveDirection == None){


		return (own.position + directionToVector(own.moveDirection)*(.5));

	}else{
		//We need to do stuff difrently if tanks are colliding diagonaly.
		if( (abs(directionToVector(own.moveDirection).y()) != abs(directionToVector(enemy.moveDirection).y())) ||
				(abs(directionToVector(own.moveDirection).x()) != abs(directionToVector(enemy.moveDirection).x()))) {

			//std::wcout << "Kollajded dajägonally" << std::endl;
			return (own.position + directionToVector(own.moveDirection)*(0.5)); //Diagonally

		}else{

			QPointF diff = own.position - enemy.position;
			qreal distance = manhattanDistanceOnTorus(own.position,enemy.position);
			offs = distance > 1 ? 0.75 : 0.25;

			return  (own.position + directionToVector(own.moveDirection)*(offs));

		}
	}

}


QPointF GameWorld::directionToVector(Direction dir){
	switch(dir){
		case UpHeading:
			return QPointF(0,-1);
		case DownHeading:
			return QPointF(0,1);
		case LeftHeading:
			return QPointF(-1,0);

		case RightHeading:
			return QPointF(1,0);
		default:
			return QPointF(0,0);
	}
}

const char* GameWorld::directionToString(Direction dir){
	switch(dir){
		case UpHeading:
			return "Up";
		case DownHeading:
			return "Down";
		case LeftHeading:
			return "Left";

		case RightHeading:
			return "Right";
		default:
			return "None";
	}
}

void GameWorld::collisionOverride(Tank& own, Tank& enemy){
	return;
	//QPointF diff = own.position - enemy.position;
	//int key = (int)(diff.y()*10) + (int)(diff.x());

}


qreal GameWorld::manhattanDistanceOnTorus(QPointF a, QPointF b){
	QPointF a_u = a - QPointF(0,m_matchState.gameState.size.y());
	QPointF a_d = a + QPointF(0,m_matchState.gameState.size.y());
	QPointF a_l = a - QPointF(m_matchState.gameState.size.x(),0);
	QPointF a_r = a + QPointF(m_matchState.gameState.size.x(),0);
	QPointF a_list[5] = {a_u,a_d,a_l,a_r,a};
	qreal min_distance = std::numeric_limits<qreal>::max() ;
	for(int i = 0; i < 5; i++){
		min_distance = simpleDistance(b, a_list[i]) < min_distance ? simpleDistance(b, a_list[i]) : min_distance;
	}
	return min_distance;

}

qreal GameWorld::timeToEvent(QPointF a, QPointF b,qreal speed){
	//qreal distance = manhattanDistanceOnTorus(a,b);
	qreal distance = simpleDistance(a,b);

	//std::wcout << "a is " << a.x() <<","<<a.y() << std::endl;
	//std::wcout << "b is " << b.x() <<","<<b.y() << std::endl;

	//std::wcout << "distance is " << distance << std::endl;
	//std::wcout << "computed time is  " << distance/speed << std::endl << std::endl;
	return distance/speed;

}


qreal GameWorld::simpleDistance(QPointF a, QPointF b){
	QPointF tmp = (a-b);
	//std::wcout << "tmp is " << tmp.x() <<","<<tmp.y() << std::endl;
	//std::wcout << "abs of tmp is " << std::abs((double)tmp.x()) <<","<< std::abs((double)tmp.y()) << std::endl;

	return std::abs((double)tmp.x()) + std::abs((double)tmp.y());
}














