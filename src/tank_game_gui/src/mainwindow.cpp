/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include <string>
#include <boost/filesystem.hpp>
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "boardparser.h"


MainWindow::MainWindow(int updateFrequency, bool soundEnalbed, bool override_sound, bool override_freq, QWidget *parent)
    :QMainWindow(parent)
	,m_conf("rules.cfg")
	//,m_world(updateFrequency,soundEnalbed,m_conf)
    ,ui(new Ui::MainWindow)
    ,m_updateInterval(1000/updateFrequency) //hz
    ,m_tankGameWidget(NULL)
    //,m_world(m_updateInterval, soundEnalbed)
    ,m_dispatchEvent(static_cast<QEvent::Type>(QEvent::User+666))
    ,m_conThread(&m_dobConnection, this, this, 0)
    ,m_updateTimer(this)

{



	bool audio_enbaled = m_conf.m_audio_enabled;
	if(override_sound){
		audio_enbaled = soundEnalbed;
	}
	int update_freq = m_conf.m_frame_rate;
	if(override_freq){
		update_freq = updateFrequency;
	}

	m_world.reset(new GameWorld(update_freq,audio_enbaled, m_conf));


	this->setWindowState(Qt::WindowMaximized);
    installEventFilter(this);    
    ui->setupUi(this);

    //background-image: url(:/images/clay.png)
    QPalette palette;
    palette.setBrush(this->backgroundRole(),QBrush(QImage(":/images/clay.png")));
    setPalette(palette);

    statusBar()->showMessage("Not connected");
    QObject::connect(&m_conThread, SIGNAL(ConnectedToDob()), this, SLOT(OnConnected()));
    m_conThread.start();

    m_gameNumber=new QLabel("Game: 0/0");
    m_gameNumber->setFont( QFont( "sans", 12, QFont::Bold, false) );
    m_gameNumber->setStyleSheet("QLabel { background-color : white; color : blue; }");
    statusBar()->addPermanentWidget(m_gameNumber);

    m_gameTime=new QLabel("Time: 0");
    m_gameTime->setFont( QFont( "sans", 12, QFont::Bold, false) );
    m_gameTime->setStyleSheet("QLabel { background-color : white; color : blue; }");
    statusBar()->addPermanentWidget(m_gameTime);

    m_tankInfoWidget[0]=new TankInfoWidget(0);
    m_tankInfoWidget[1]=new TankInfoWidget(1);
    m_tankGameWidget=new TankGameWidget(*m_world);

    QVBoxLayout* mainLayout=static_cast<QVBoxLayout*>(centralWidget()->layout());
    mainLayout->addWidget(m_tankInfoWidget[0]);
    mainLayout->setAlignment(m_tankInfoWidget[0], Qt::AlignTop);
    mainLayout->addWidget(m_tankGameWidget);
    mainLayout->addWidget(m_tankInfoWidget[1]);
    mainLayout->setAlignment(m_tankInfoWidget[1], Qt::AlignTop);


    QObject::connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(OnUpdateWorld()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::OnUpdateWorld()
{
    if (m_world->MatchFinished())
    {
        MatchFinished();
    }

    m_world->Update();
    m_tankGameWidget->update();
}

void MainWindow::OnConnected()
{
    m_dobConnection.Open(L"tank_game_gui", QTime::currentTime().toString("hh:mm:ss.zzz").toStdWString(), 0, this, this);
    statusBar()->showMessage("Connected");
    m_dobConnection.SubscribeEntity(Consoden::TankGame::Player::ClassTypeId, this);
    m_dobConnection.SubscribeEntity(Consoden::TankGame::Match::ClassTypeId, this);
    m_dobConnection.SubscribeEntity(Consoden::TankGame::GameState::ClassTypeId, this);
    m_dobConnection.SubscribeEntity(Consoden::TankGame::Joystick::ClassTypeId, this);
}

bool MainWindow::eventFilter(QObject*, QEvent* e)
{
    if (e->type()==m_dispatchEvent)
    {
        m_dobConnection.Dispatch();
        return true;
    }
    return false;
}

//------------------------------------------------------------
// DOB stuff
//------------------------------------------------------------
void MainWindow::OnDoDispatch()
{
    QApplication::postEvent(this, new QEvent(m_dispatchEvent));
}

void MainWindow::OnStopOrder()
{
}

void MainWindow::OnNewEntity(const Safir::Dob::EntityProxy entityProxy)
{
    Consoden::TankGame::PlayerConstPtr player=boost::dynamic_pointer_cast<Consoden::TankGame::Player>(entityProxy.GetEntity());
    if (player)
    {
        m_world->AddPlayer(player, entityProxy.GetInstanceId().GetRawValue());
        return;
    }
    Consoden::TankGame::GameStatePtr game=boost::dynamic_pointer_cast<Consoden::TankGame::GameState>(entityProxy.GetEntity());



    Consoden::TankGame::MatchPtr match=boost::dynamic_pointer_cast<Consoden::TankGame::Match>(entityProxy.GetEntity());
    if (match)
    {
        m_world->Reset(match, entityProxy.GetInstanceId().GetRawValue());

        auto player1=m_world->GetPlayerOne();
        auto player2=m_world->GetPlayerTwo();
        if (player1 && player2)
        {

            m_tankInfoWidget[0]->SetName(player1->name);
            m_tankInfoWidget[0]->Update(m_world->GetJoystickOne());
            m_tankInfoWidget[1]->SetName(player2->name);
            m_tankInfoWidget[1]->Update(m_world->GetJoystickTwo());

            UpdatePoints();
        }
        return;
    }


    if (game)
    {
        if (m_world->GameId()!=0 && !m_world->MatchFinished())
        {
            QMessageBox::information(this, "New game started", "A new game was started while there is still an ongoing game." );
            return;
        }

        m_world->Reset(game, entityProxy.GetInstanceId().GetRawValue());
        m_tankGameWidget->Reset();
        m_updateTimer.start(m_updateInterval);

        UpdateStatusGameNumber();
        UpdateStatusGameTime();
        return;
    }

    Consoden::TankGame::JoystickConstPtr joystick=boost::dynamic_pointer_cast<Consoden::TankGame::Joystick>(entityProxy.GetEntity());
    if (joystick)
    {
        m_world->Update(joystick);
        return;
    }

}

void MainWindow::OnUpdatedEntity(const Safir::Dob::EntityProxy entityProxy)
{
    Consoden::TankGame::GameStatePtr game=boost::dynamic_pointer_cast<Consoden::TankGame::GameState>(entityProxy.GetEntity());
    if (game && entityProxy.GetInstanceId().GetRawValue()==m_world->GameId())
    {
        m_world->Update(game);
        UpdateStatusGameTime();

        m_tankInfoWidget[0]->SetLaserAmmo(game->Tanks()[0]->Lasers());
        m_tankInfoWidget[1]->SetLaserAmmo(game->Tanks()[1]->Lasers());
        m_tankInfoWidget[0]->updateSmoke(game->Tanks()[0]->HasSmoke(),game->Tanks()[0]->SmokeLeft());
		m_tankInfoWidget[1]->updateSmoke(game->Tanks()[1]->HasSmoke(),game->Tanks()[1]->SmokeLeft());

		m_tankInfoWidget[0]->updateReddemerWithTankyStuff(game->Tanks()[0]->HasRedeemer(),game->Tanks()[0]->RedeemerTimerLeft());
		m_tankInfoWidget[1]->updateReddemerWithTankyStuff(game->Tanks()[1]->HasRedeemer(),game->Tanks()[1]->RedeemerTimerLeft());



        return;
    }

    Consoden::TankGame::JoystickConstPtr joystick=boost::dynamic_pointer_cast<Consoden::TankGame::Joystick>(entityProxy.GetEntity());
    if (joystick  )
    {

        m_world->Update(joystick);
        m_tankInfoWidget[0]->Update(m_world->GetJoystickOne());
        m_tankInfoWidget[1]->Update(m_world->GetJoystickTwo());

        return;
    }

    Consoden::TankGame::MatchPtr match=boost::dynamic_pointer_cast<Consoden::TankGame::Match>(entityProxy.GetEntity());
    if (match && entityProxy.GetInstanceId().GetRawValue()==m_world->MatchId())
    {
        //updated match
        m_world->Update(match);
        QTimer::singleShot(m_world->GetMatchState().gameState.pace, this, SLOT(UpdatePoints()));

        return;
    }
}

void MainWindow::OnDeletedEntity(const Safir::Dob::EntityProxy entityProxy, const bool /*del*/)
{
    if (entityProxy.GetTypeId()==Consoden::TankGame::Player::ClassTypeId)
    {
        m_world->DeletePlayer(entityProxy.GetInstanceId().GetRawValue());
    }
    else if (entityProxy.GetTypeId()==Consoden::TankGame::GameState::ClassTypeId)
    {
        if (m_world->GameId()==entityProxy.GetInstanceId().GetRawValue())
        {
            m_world->ClearGameState();
            m_tankGameWidget->Reset();
        }
    }
    else if (entityProxy.GetTypeId()==Consoden::TankGame::Match::ClassTypeId)
    {
        //match deleted
        m_world->Clear();
    }
}

void MainWindow::OnResponse(const Safir::Dob::ResponseProxy responseProxy)
{
}

void MainWindow::OnNotRequestOverflow()
{
}

void MainWindow::MatchFinished()
{
    m_updateTimer.stop();
    QStringList sl;
    sl.append("Match Over!");
    auto winner=m_world->GetPlayerById(m_world->GetMatchState().winnerPlayerId);
    if (winner)
    {
        sl.append("...and the winner is...");
        sl.append(winner->name+"!");
    }
    else
    {
        sl.append("Draw");
    }
    m_world->SetTextBig(sl);
}

void MainWindow::OnActionNewGame()
{
    if (!m_dobConnection.IsOpen())
    {
        QMessageBox::information(this, "Not connected", "Can't start a game because we are not connected to the game engine!");
        return;
    }

    NewGameDialog dlg(m_world->GetPlayers());
    if (dlg.exec()==QDialog::Accepted)
    {
        OnActionStopGame();
        auto files=dlg.Boards();
        if (files.empty())
        {
            return;
        }

        Consoden::TankGame::MatchPtr match=Consoden::TankGame::Match::Create();
        match->GameTime()=dlg.GameTime();
        match->RepeatBoardSequence()=dlg.Repetitions();

        int index=0;
        for (auto& f : files)
        {
            if (index>=match->BoardsArraySize())
            {
                break;
            }

            match->Boards()[index++].SetVal(f.toStdWString());
        }

        auto p1=m_world->GetPlayerByName(dlg.Player1());
        auto p2=m_world->GetPlayerByName(dlg.Player2());
        if (p1!=nullptr && p2!=nullptr)
        {
            match->PlayerOneId()=Safir::Dob::Typesystem::InstanceId(p1->id);
            match->PlayerTwoId()=Safir::Dob::Typesystem::InstanceId(p2->id);
        }

        m_tankInfoWidget[0]->SetPoints(0);
        m_tankInfoWidget[1]->SetPoints(0);
        m_tankInfoWidget[0]->SetLaserAmmo(0);
        m_tankInfoWidget[1]->SetLaserAmmo(0);

        m_dobConnection.CreateRequest(match, Safir::Dob::Typesystem::HandlerId(), this);
    }
}

void MainWindow::OnActionStopGame()
{
    for (auto it=m_dobConnection.GetEntityIterator(Consoden::TankGame::Match::ClassTypeId, false); it!=Safir::Dob::EntityIterator(); ++it)
    {
        m_dobConnection.DeleteRequest((*it).GetEntityId(), this);
    }
}

void MainWindow::UpdatePoints()
{

    m_tankInfoWidget[0]->SetPoints(m_world->GetPlayerOneTotalPoints());
    m_tankInfoWidget[1]->SetPoints(m_world->GetPlayerTwoTotalPoints());

}

void MainWindow::OnActionRestartGame()
{
    auto dummyPtr=Consoden::TankGame::Match::Create();
    m_dobConnection.UpdateRequest(dummyPtr, Safir::Dob::Typesystem::InstanceId(m_world->MatchId()), this);
}

void MainWindow::UpdateStatusGameNumber()
{
    std::ostringstream os;
    os<<"Game "<<m_world->GetMatchState().currentGameNumber<<"/"<<m_world->GetMatchState().totalNumberOfGames;
    m_gameNumber->setText(os.str().c_str());
}

void MainWindow::UpdateStatusGameTime()
{
    m_gameTime->setText( QString("Time ")+QString::number(m_world->GetGameState().elapsedTime));
}
