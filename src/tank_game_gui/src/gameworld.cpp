/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include "gameworld.h"
#include "PassiveGroup.h"

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
        return LeftHeading;
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
	,m_sprites_dude()
    ,m_explosion()
    ,m_tankFire()
    ,m_fireMediaPlayer1()
    ,m_explosionMediaPlayer1()
    ,m_fireMediaPlayer2()
    ,m_explosionMediaPlayer2()
    ,m_tookCoinMediaPlayer()
    ,m_wilhelmScreamMediaPlayer()
	,m_passive_coins(m_matchState,":/images/coin_sheet.png", 8, 72, 72,1000,0,0)


{
	//m_passive_coins = PassiveGroup(m_matchState,NULL,":/images/coin_sheet.png", 8, 72, 72,1000,0,0);

	//m_passive_coins = PassiveGroup(m_matchState,":/images/coin_sheet.png", 8, 72, 72,1000,0,0);

    if (m_soundEnabled)
    {
        InitMediaPlayers();
    }

    //load sprite images
    m_explosion.image=QPixmap(":/images/explosion_sheet.png");
    m_explosion.lifeTime=1000;
    for (int i=0; i<11; ++i)
    {
        m_explosion.fragments.push_back(QRectF(i*72, 0, 72, 72));
    }

    m_tankFire.image=QPixmap(":/images/tank_fire_sheet.png");
    m_tankFire.lifeTime=300;
    for (int i=0; i<5; ++i)
    {
        m_tankFire.fragments.push_back(QRectF(i*72, 0, 72, 72));
    }

    m_coin.image=QPixmap(":/images/coin_sheet.png");
    m_coin.lifeTime=1000;
    for (int i=0; i<8; ++i)
    {
       m_coin.fragments.push_back(QRectF(i*72, 0, 72, 72));
    }


    m_dude.image=QPixmap(":/images/tux-anim.png");
	m_dude.lifeTime=500;
	for (int i=0; i < 3; ++i)
	{
		m_dude.fragments.push_back(QRectF(i*72, 0, 72, 72));
	}

    m_laser_ammo.image=QPixmap(":/images/laser-ammo.png");
	m_laser_ammo.lifeTime=1200;
	for (int i=0; i < 27; ++i)
	{
		m_laser_ammo.fragments.push_back(QRectF(i*66, 0, 66, 67));
	}

    m_laser_middle.image=QPixmap(":/images/laser-middle.png");
	m_laser_middle.lifeTime=1000;
	m_laser_middle.fragments.push_back(QRectF(0, 0, 72, 72));

    m_laser_start.image=QPixmap(":/images/laser-start.png");
	m_laser_start.lifeTime=1000;
	m_laser_start.fragments.push_back(QRectF(0, 0, 72, 72));

	m_passive_coins.setSoundPlayer("coin.mp3",soundEnabled);




}


std::vector<Sprite> GameWorld::getPassiveSprites() const{
	return m_passive_coins.m_sprites;
}
void GameWorld::Clear()
{
    m_matchState=MatchState();
    m_matchState.finished=true;
    m_passive_coins.clear();
    m_sprites.clear();
    m_sprites_dude.clear();
    m_screenText.clear();
}

void GameWorld::ClearGameState()
{
    m_matchState.gameState=GameState();
    m_matchState.gameState.finished=true;
    m_passive_coins.clear();
    m_sprites.clear();
    m_sprites_dude.clear();
    m_screenText.clear();
}

void GameWorld::Reset(const Consoden::TankGame::MatchPtr& match, boost::int64_t id)
{
    m_matchState=MatchState();
    m_matchState.machId=id;
    m_matchState.players[0]=match->PlayerOneId().GetVal().GetRawValue();
    m_matchState.players[1]=match->PlayerTwoId().GetVal().GetRawValue();
    m_matchState.totalNumberOfGames=match->TotalNumberOfGames();
    m_passive_coins.clear();
    m_sprites.clear();
    m_sprites_dude.clear();
    m_screenText.clear();
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
    m_sprites_dude.clear();
    m_screenText.clear();
    m_passive_coins.clear();

    if (!game->GamePace().IsNull())
    {
        m_matchState.gameState.pace=static_cast<int>(game->GamePace().GetVal()*1000.0f);
    }

    Board boardParser(&game->Board().GetVal()[0], game->Width().GetVal(), game->Height().GetVal());
    m_matchState.gameState.size.setX(boardParser.GetXSize());
    m_matchState.gameState.size.setY(boardParser.GetYSize());
    m_matchState.gameState.walls.insert(m_matchState.gameState.walls.begin(), boardParser.Walls().begin(), boardParser.Walls().end());
    m_matchState.gameState.poison.insert(m_matchState.gameState.poison.begin(), boardParser.Poison().begin(), boardParser.Poison().end());


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

void  GameWorld::UpdateCoins(const Board& boardParser){
    if (boardParser.Coins().size()!=m_matchState.gameState.coins.size())
    {
        if (m_matchState.gameState.coins.empty())
        {
            //first time after start, immediately place coins on board
            m_matchState.gameState.coins.insert(m_matchState.gameState.coins.begin(), boardParser.Coins().begin(), boardParser.Coins().end());
            for (auto pos : m_matchState.gameState.coins)
            {
                m_sprites.push_back(Sprite(m_coin, pos, QDateTime::currentMSecsSinceEpoch(), 0));
            	//m_sprites.push_back(Sprite(m_coin, pos, QPointF(1.0, 0), 0.0, QDateTime::currentMSecsSinceEpoch(), 0));
            }
        }
        else
        {
            //coin has changed, we do the update after a halfSquare-time to make it look nicer.
            m_eventQueue.insert(WorldEvents::value_type(m_matchState.gameState.lastUpdate+m_matchState.gameState.pace*0.75, [=]
            {
                //update coin positions
                m_matchState.gameState.coins.clear();
                m_matchState.gameState.coins.insert(m_matchState.gameState.coins.begin(), boardParser.Coins().begin(), boardParser.Coins().end());

                for (auto spriteIt=m_sprites.begin(); spriteIt!=m_sprites.end();)
                {
                    bool remove=true;
                    if (spriteIt->Data()==&m_coin)
                    {
                        for (auto& coin : m_matchState.gameState.coins)
                        {
                            if (coin==spriteIt->Position())
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
                if (m_soundEnabled)
                {
                    m_tookCoinMediaPlayer.stop();
                    m_tookCoinMediaPlayer.play();
                }
            }));
        }
    }
}


void  GameWorld::UpdateLaserAmmo(const Board& boardParser)
{
    if (boardParser.LaserAmmo().size()!=m_matchState.gameState.laser_ammo.size())
    {
        if (m_matchState.gameState.laser_ammo.empty())
        {

            //first time after start, immediately place laser ammo on board
            m_matchState.gameState.laser_ammo.insert(m_matchState.gameState.laser_ammo.begin(), boardParser.LaserAmmo().begin(), boardParser.LaserAmmo().end());
            for (auto pos : m_matchState.gameState.laser_ammo)
            {

                //m_sprites.push_back(Sprite(m_laser_ammo, pos, QDateTime::currentMSecsSinceEpoch(), 0));
            	//m_sprites.push_back(Sprite(m_coin, pos, QPointF(1.0, 0), 0.0, QDateTime::currentMSecsSinceEpoch(), 0));
            }
        }
        else
        {
            //laser ammo has changed, we do the update after a halfSquare-time to make it look nicer.
            m_eventQueue.insert(WorldEvents::value_type(m_matchState.gameState.lastUpdate+m_matchState.gameState.pace*0.75, [=]
            {
                //update laser ammo positions
                m_matchState.gameState.laser_ammo.clear();
                m_matchState.gameState.laser_ammo.insert(m_matchState.gameState.laser_ammo.begin(), boardParser.LaserAmmo().begin(), boardParser.LaserAmmo().end());

                for (auto spriteIt=m_sprites.begin(); spriteIt!=m_sprites.end();)
                {
                    bool remove=true;
                    if (spriteIt->Data()==&m_laser_ammo)
                    {
                        for (auto& laser : m_matchState.gameState.laser_ammo)
                        {
                            if (laser==spriteIt->Position())
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
                if (m_soundEnabled)
                {
                    m_laser_pickup_MediaPlayer.stop();
                    m_laser_pickup_MediaPlayer.play();
                }
            }));
        }
    }
}

void  GameWorld::UpdatePoison(const Board& boardParser)
{
    if (boardParser.Poison().size()!=m_matchState.gameState.poison.size())
    {
        //coin has changed, we do the update after a halfSquare-time to make it look nicer.
        m_eventQueue.insert(WorldEvents::value_type(m_matchState.gameState.lastUpdate+m_matchState.gameState.pace*0.75, [=]
        {
            //update coin positions
            m_matchState.gameState.poison.clear();
            m_matchState.gameState.poison.insert(m_matchState.gameState.poison.begin(), boardParser.Poison().begin(), boardParser.Poison().end());
            //play sound
            if (m_soundEnabled)
            {
                m_wilhelmScreamMediaPlayer.stop();
                m_wilhelmScreamMediaPlayer.play();
            }
        }));
    }
}

//void  GameWorld::UpdateDudes(const Board& boardParser){}

void  GameWorld::UpdateDudes(const Board& boardParser)
{
	return;
        	//std::wcout << "There are " << m_matchState.gameState.dudes.size() << " Lights!" << std::endl;
//            for (auto& dude : m_matchState.gameState.dudes)
//            {
                //m_sprites.push_back(Sprite(m_dude, pos, QPointF(1.0, 0), 0.0, QDateTime::currentMSecsSinceEpoch(), 0));
            	//std::wcout << "Dude is at: " << dude.position.x() <<","<< dude.position.y() <<
            	//		" and its paint position is : "<< dude.paintPosition.x() <<","<< dude.paintPosition.y()<< std::endl;
                //m_sprites.push_back(Sprite(m_dude, dude.paintPosition, QDateTime::currentMSecsSinceEpoch(), 0));
            	//m_sprites.push_back(Sprite(m_dude, dude.paintPosition, QPointF(1.0, 0), 5.0, QDateTime::currentMSecsSinceEpoch(), 0));
//            }

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

void GameWorld::UpdateDude(const Consoden::TankGame::DudePtr& dude){
	Dude& d = m_matchState.gameState.dudes.front();
	d.moveDirection = ToDirection(dude->Direction());


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


    qreal rot = 0;
    qreal dx = 0;
    qreal dy = 0;
    //Compensate position for move direction to make
    //the laser appear at the correct instance and deal with neutral tower direction
    switch (t.moveDirection){
    	case Direction::UpHeading:
    		dy = -1;
    		y_pos--;
    		rot = 0;
    		break;
    	case Direction::DownHeading:
			dy = 1;
			y_pos++;
			rot = 180;
			break;
    	case Direction::LeftHeading:
			dx = -1;
			x_pos--;
			rot = 270;
			break;
    	case Direction::RightHeading:
			dx = 1;
			x_pos++;
			rot = 90;
			break;
    	default:
    		return;
    }

    if(t.towerDirection != Direction::None){
    	dy = 0;
    	dx = 0;
    }

    Tank& enemy=m_matchState.gameState.tanks[(i + 1) %2];
    QPointF enemy_position = enemy.position;



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
    		//return;
    }


    m_sprites.push_back(Sprite(m_laser_start, QPointF(x_pos,y_pos),QPointF(0,0),rot, m_matchState.gameState.lastUpdate+m_matchState.gameState.pace*0.75, 1));
    //x_pos += dx;
    //y_pos += dy;

    while(true){
    	x_pos = wrap(x_pos + dx,m_matchState.gameState.size.x());
    	y_pos = wrap(y_pos + dy,m_matchState.gameState.size.y());
    	//std::wcout << "x = "<< x_pos <<" y = "<< y_pos << std::endl;
    	//y_pos += dy;
    	if(board.isWall(x_pos,y_pos)){
    		//Reaced wally thing play sparks
    		break;
    	}else if(enemy_position == QPointF(x_pos,y_pos)){
    		//Hit enemy tank. just chill
    		m_sprites.push_back(Sprite(m_laser_middle, QPointF(x_pos,y_pos),QPointF(0,0),rot, m_matchState.gameState.lastUpdate+m_matchState.gameState.pace*0.75, 1));
    		break;
    	}else if(t.position == QPointF(x_pos,y_pos)){
			//Hit enemy tank. just chill
    		m_sprites.push_back(Sprite(m_laser_middle, QPointF(x_pos,y_pos),QPointF(0,0),rot, m_matchState.gameState.lastUpdate+m_matchState.gameState.pace*0.75, 1));
			break;
    	//}else if(x_pos > m_matchState.gameState.size.x() || x_pos < 0 || y_pos > m_matchState.gameState.size.y() || y_pos < 0){
    	//	//outside.
    	//	break;
    	}
    	m_sprites.push_back(Sprite(m_laser_middle, QPointF(x_pos,y_pos),QPointF(0,0),rot, m_matchState.gameState.lastUpdate+m_matchState.gameState.pace*0.75, 1));

    }
    m_eventQueue.insert(WorldEvents::value_type(m_matchState.gameState.lastUpdate+m_matchState.gameState.pace*0.75, [=]
    {
        //update coin positions
        //play sound
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
    UpdateTankWrapping(tank, t);
    t.moveDirection=ToDirection(tank->MoveDirection());
    t.fires=tank->Fire().GetVal();
    t.towerDirection=ToDirection(tank->TowerDirection());

    if(!tank->FireLaser().IsNull() && tank->FireLaser() ){
    	std::wcout << "is fiering laser" << std::endl;
    	DrawLaser(tank,board);
    }

    if (tank->InFlames().GetVal())
    {
        switch (t.explosion)
        {
        case NotInFlames:
        {
            t.explosion=SetInFlames;

            if (t.deathCause==Tank::HitWall || t.deathCause==Tank::HitTank)
            {
                return;
            }

            t.paintPosition=t.position;
            t.position=QPointF(tank->PosX().GetVal(), tank->PosY().GetVal());

            bool hasSpecialEndPos=false;
            qreal specialEndPos=0;
            if (tank->HitWall().IsNull()==false && tank->HitWall().GetVal()==true)
            {
                t.deathCause=Tank::HitWall;
                specialEndPos=0.5;
                hasSpecialEndPos=true;
            }
            else if (tank->HitTank().IsNull()==false && tank->HitTank().GetVal()==true)
            {
                t.deathCause=Tank::HitTank;
                specialEndPos=0.25;
                hasSpecialEndPos=true;
            }

            //if tank hit wall or drove into other tank, we calculate a position half inside the
            //next square before start explosion.
            if (hasSpecialEndPos)
            {
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
            break;
        default:
            break;
        }



        if (!tank->HitWall().IsNull() && tank->HitWall())
        {
            t.deathCause=Tank::HitWall;
        }
        else if (!tank->HitMine().IsNull() && tank->HitMine())
        {
            t.deathCause=Tank::HitMine;
        }
        else if (!tank->HitMissile().IsNull() && tank->HitMissile())
        {
            t.deathCause=Tank::HitMissile;
        }
        else if (!tank->HitTank().IsNull() && tank->HitTank())
        {
            t.deathCause=Tank::HitTank;
        }
        else
        {
            t.deathCause=Tank::None;
        }
    }
    else if (t.explosion!=NotInFlames)
    {
        t.explosion=Destroyed;
    }
    else
    {

        t.paintPosition=t.position;
        t.position=QPointF(tank->PosX().GetVal(), tank->PosY().GetVal());
    }
}


/*
 * Method updating all of the entities.
 *
 * Gets called when the game state is updated (every second by default)
 *
 * Unfortunatley it might appear as if this function needs to match the behaviour of the function in Engine.cpp....whose name i have forgotten
 *
 */
void GameWorld::Update(const Consoden::TankGame::GameStatePtr &game)
{

    m_matchState.gameState.lastUpdate=QDateTime::currentMSecsSinceEpoch();
    m_matchState.gameState.elapsedTime=static_cast<int>(game->ElapsedTime().GetVal());

    m_matchState.gameState.mines.clear();

    //Todo: apparently the insert the mines here for some cryptic reason.
    Board boardParser(&game->Board().GetVal()[0], game->Width().GetVal(), game->Height().GetVal());
    m_matchState.gameState.mines.insert(m_matchState.gameState.mines.begin(), boardParser.Mines().begin(), boardParser.Mines().end());

    UpdateLaserAmmo(boardParser);
    //UpdateCoins(boardParser);
    std::wcout << "We are entering the update with change function now " << std::endl;
    m_passive_coins.updateGroupOnChange(boardParser.Coins(),m_matchState.gameState, m_eventQueue);



    //UpdateDudes(boardParser);


    if(!game->TheDude().IsNull()){
    	auto& dude_game = game->TheDude().GetPtr();
    	auto& dude = m_matchState.gameState.dudes.front();

    	if(!dude.dying && dude_game->Dying().GetVal() && m_soundEnabled){
    		m_dude_dies_MediaPlayer.play();
    	}else{
    		m_dude_dies_MediaPlayer.stop();
    	}

    	if(dude.just_died && dude_game->Dying() && !dude.dying){
    		dude.just_died = false;
    		dude.dying = true;
    	}

    	if(!dude.dying && dude_game->Dying()){
    		dude.just_died = true;

    	}

    	if(!dude.dying){
    		dude.position.setX(dude_game->PosX());
    		dude.position.setY(dude_game->PosY());
    		dude.moveDirection = ToDirection(dude_game->Direction());
    	}


    }

    //if hit poison gas square, play a terrible sound
    if (game->Tanks()[0].GetPtr()->HitPoisonGas().GetVal() || game->Tanks()[1].GetPtr()->HitPoisonGas().GetVal())
    {
        UpdatePoison(boardParser);
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

    //creates and updates missiles
    for (Safir::Dob::Typesystem::ArrayIndex i=0; i < game->MissilesArraySize(); i++)
    {
        if (game->Missiles()[i].IsNull())
        {
            continue;
        }

        const Consoden::TankGame::MissileConstPtr& missile=game->Missiles()[i].GetPtr();

        auto inserted=m_matchState.gameState.missiles.insert(std::make_pair(missile->MissileId().GetVal(), Missile()));
        Missile& m=inserted.first->second;
        if (inserted.second)
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
        }

        m.moveDirection=ToDirection(missile->Direction());

        if (missile->InFlames().GetVal())
        {
            m.explosion=m.explosion==NotInFlames ? SetInFlames : Burning;
        }
        else if (m.explosion!=NotInFlames)
        {
            m.explosion=Destroyed;
        }
    }

    //Updates the tanks
    for (int i=0; i<game->TanksArraySize(); ++i)
    {
        if (!game->Tanks()[i].IsNull())
        {
            const Consoden::TankGame::TankPtr& tank=game->Tanks()[i].GetPtr();
            UpdateTank(tank, boardParser);
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

        auto paintGameEndTime=m_matchState.gameState.lastUpdate+2*m_matchState.gameState.pace;
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
    js.fire=joystick->FireLaser().IsNull() ? false : joystick->FireLaser().GetVal();
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


    //updates each tank.
    for (Tank& tank : m_matchState.gameState.tanks)
    {
    	//if(tank.explosion == SetInFlames){

			if(tank.deathCause == tank.Death::HitWall){

				UpdatePosition(timeToNextUpdate, movement*0.5, tank);
			}else{
				UpdatePosition(timeToNextUpdate, movement, tank);
			}


        UpdateTowerAngle(timeToNextUpdate, angle, tank);
        if (tank.explosion==SetInFlames)
        {
            //new explosion sprite
            m_sprites.push_back(Sprite(m_explosion, tank.position, now+timeToNextUpdate, 1));
            m_sprites.push_back(Sprite(m_explosion, QPointF(tank.position.x()+0.3f, tank.position.y()+0.3f), now+timeToNextUpdate+300, 1));
            m_sprites.push_back(Sprite(m_explosion, QPointF(tank.position.x()-0.2f, tank.position.y()+0.2f), now+timeToNextUpdate+500, 1));
            m_sprites.push_back(Sprite(m_explosion, QPointF(tank.position.x()-0.3f, tank.position.y()-0.3f), now+timeToNextUpdate+800, 1));
            tank.explosion=Burning;
            m_eventQueue.insert(WorldEvents::value_type(nextUpdate, [&]
            {
                tank.explosion=Destroyed;
            }));

            if (m_soundEnabled)
            {
                auto tankPlayerId=tank.playerId;
                m_eventQueue.insert(WorldEvents::value_type(now+timeToNextUpdate, [=]
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
        UpdatePosition(timeToNextUpdate, 2*movement, missile); //missiles have double speed

        if (missile.paintFire)
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
                break;
            }

            m_sprites.push_back(Sprite(m_tankFire, firePos, animationMoveSpeed, DirectionToAngle(missile.moveDirection), nextUpdate, 1));
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
            m_sprites.push_back(Sprite(m_explosion, missile.position, nextUpdate, 1));
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


    //TODO:
    //update of our dude
    for (auto& dude : m_matchState.gameState.dudes){
        if(dude.just_died){
        	UpdatePosition(timeToNextUpdate, .5*movement, dude,false);

        }else if(!dude.dying){
        	UpdatePosition(timeToNextUpdate, 1*movement, dude);
        }

    }

    //Todo: Updating passive sprites. Move this to PasiveGroup class
    for (auto it=m_passive_coins.m_sprites.begin(); it!=m_passive_coins.m_sprites.end();)
    {
        if (it->Finished())
        {
            it=m_passive_coins.m_sprites.erase(it);
        }
        else
        {
        	//std::wcout << "We updated a sprite." << std::endl;
            it->Update();

            ++it;
        }
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
    m_tookCoinMediaPlayer.setVolume(100);
    m_wilhelmScreamMediaPlayer.setVolume(80);

    const char* runtime=getenv("SAFIR_RUNTIME");
    QString path=QDir::cleanPath(QString(runtime)+QDir::separator()+"data"+QDir::separator()+"tank_game"+QDir::separator()+"sounds");
    QString firePath=QDir::cleanPath(path+QDir::separator()+"shotgun.mp3");
    QString explostionPath=QDir::cleanPath(path+QDir::separator()+"explosion.mp3");
    QString gunPath=QDir::cleanPath(path+QDir::separator()+"gun.mp3");
    QString bigBombPath=QDir::cleanPath(path+QDir::separator()+"big_bomb.mp3");
    QString tookCoin=QDir::cleanPath(path+QDir::separator()+"coin.mp3");
    QString wilhelmScream=QDir::cleanPath(path+QDir::separator()+"wilhelm_scream.mp3");
    QString death_of_dude=QDir::cleanPath(path+QDir::separator()+"dude-dies.mp3");
    QString laser_pickup=QDir::cleanPath(path+QDir::separator()+"laser-pickup.mp3");
    QString laser_fire=QDir::cleanPath(path+QDir::separator()+"laser-fire.mp3");

    m_fireMediaPlayer1.setMedia(QUrl::fromLocalFile(firePath));
    m_explosionMediaPlayer1.setMedia(QUrl::fromLocalFile(explostionPath));
    m_fireMediaPlayer2.setMedia(QUrl::fromLocalFile(gunPath));
    m_explosionMediaPlayer2.setMedia(QUrl::fromLocalFile(bigBombPath));
    m_tookCoinMediaPlayer.setMedia(QUrl::fromLocalFile(tookCoin));
    m_wilhelmScreamMediaPlayer.setMedia(QUrl::fromLocalFile(wilhelmScream));
    m_dude_dies_MediaPlayer.setMedia(QUrl::fromLocalFile(death_of_dude));
    m_laser_pickup_MediaPlayer.setMedia(QUrl::fromLocalFile(laser_pickup));
    m_laser_fire_MediaPlayer.setMedia(QUrl::fromLocalFile(laser_fire));
}

void GameWorld::UpdateTowerAngle(qint64 timeToNextUpdate, qreal movement, Tank& tank)
{
    qreal endAngle=DirectionToAngle(tank.towerDirection);

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
