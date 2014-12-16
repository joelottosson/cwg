/******************************************************************************
*
* Copyright Consoden AB, 2014
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

MainWindow::MainWindow(int updateFrequency, QWidget *parent)
    :QMainWindow(parent)
    ,ui(new Ui::MainWindow)
    ,m_updateInterval(1000/updateFrequency) //hz
    ,m_tankGameWidget(NULL)
    ,m_world(m_updateInterval)
    ,m_dispatchEvent(static_cast<QEvent::Type>(QEvent::User+666))
    ,m_conThread(&m_dobConnection, this, this, 0)
    ,m_updateTimer(this)
    ,m_currentMatch()
{
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

    m_tankInfoWidget[0]=new TankInfoWidget(0);
    m_tankInfoWidget[1]=new TankInfoWidget(1);
    m_tankGameWidget=new TankGameWidget(m_world);

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
    if (m_world.Finished())
    {
        MatchFinished();
    }

    m_world.Update();
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
        m_world.AddPlayer(player, entityProxy.GetInstanceId().GetRawValue());
        return;
    }

    Consoden::TankGame::MatchPtr match=boost::dynamic_pointer_cast<Consoden::TankGame::Match>(entityProxy.GetEntity());
    if (match)
    {
        m_currentMatch=match;
        m_world.Reset(match, entityProxy.GetInstanceId().GetRawValue());
        m_tankInfoWidget[0]->SetPoints(0);
        m_tankInfoWidget[1]->SetPoints(0);

        QStringList sl;
        sl.append("Start new match");
        m_world.SetTextBig(sl);
        return;
    }

    Consoden::TankGame::GameStatePtr game=boost::dynamic_pointer_cast<Consoden::TankGame::GameState>(entityProxy.GetEntity());
    if (game)
    {
        if (m_world.GameId()!=0 && !m_world.Finished())
        {
            QMessageBox::information(this, "New game started", "A new game was started while there is still an ongoing game." );
            return;
        }

        m_world.Reset(game, entityProxy.GetInstanceId().GetRawValue());

        if (game->Counter().GetVal()<=1)
        {
            QStringList sl;
            if (m_world.GetCurrentGameNumber()>1)
            {
                sl.append("Get ready for game "+QString::number(m_world.GetCurrentGameNumber())+" of "+QString::number(m_world.GetTotalNumberOfGames()));
                sl.append("Score: " + QString::number(m_world.GetPlayerOneTotalPoints())+" - "+QString::number(m_world.GetPlayerTwoTotalPoints()));
            }
            else
            {
                sl.append("Get ready...");
            }

            m_world.SetTextBig(sl);
        }

        m_world.Update(game);

        auto player1=m_world.GetPlayerOne();
        auto player2=m_world.GetPlayerTwo();
        if (player1 && player2)
        {
            m_tankInfoWidget[0]->SetName(player1->name);
            m_tankInfoWidget[0]->Update(m_world.GetJoystickOne());
            m_tankInfoWidget[1]->SetName(player2->name);
            m_tankInfoWidget[1]->Update(m_world.GetJoystickTwo());
        }

        m_tankGameWidget->Reset();

        m_updateTimer.start(m_updateInterval);

        return;
    }

    Consoden::TankGame::JoystickConstPtr joystick=boost::dynamic_pointer_cast<Consoden::TankGame::Joystick>(entityProxy.GetEntity());
    if (joystick)
    {
        m_world.Update(joystick);
        return;
    }

}

void MainWindow::OnUpdatedEntity(const Safir::Dob::EntityProxy entityProxy)
{
    Consoden::TankGame::GameStatePtr game=boost::dynamic_pointer_cast<Consoden::TankGame::GameState>(entityProxy.GetEntity());
    if (game && entityProxy.GetInstanceId().GetRawValue()==m_world.GameId())
    {
        if (!m_world.GetTextBig().isEmpty())
        {
            m_world.SetTextBig(QStringList());
        }
        m_world.Update(game);
    }

    Consoden::TankGame::JoystickConstPtr joystick=boost::dynamic_pointer_cast<Consoden::TankGame::Joystick>(entityProxy.GetEntity());
    if (joystick)
    {
        m_world.Update(joystick);
        m_tankInfoWidget[0]->Update(m_world.GetJoystickOne());
        m_tankInfoWidget[1]->Update(m_world.GetJoystickTwo());
        return;
    }

    Consoden::TankGame::MatchPtr match=boost::dynamic_pointer_cast<Consoden::TankGame::Match>(entityProxy.GetEntity());
    if (match && entityProxy.GetInstanceId().GetRawValue()==m_world.MatchId())
    {
        //updated match
    }
}

void MainWindow::OnDeletedEntity(const Safir::Dob::EntityProxy entityProxy, const bool /*del*/)
{
    if (entityProxy.GetTypeId()==Consoden::TankGame::Player::ClassTypeId)
    {
        m_world.DeletePlayer(entityProxy.GetInstanceId().GetRawValue());
    }
    else if (entityProxy.GetTypeId()==Consoden::TankGame::GameState::ClassTypeId)
    {
        if (m_world.GameId()==entityProxy.GetInstanceId().GetRawValue())
        {
            m_world.ClearGameState();
            m_tankGameWidget->Reset();
        }
    }
    else if (entityProxy.GetTypeId()==Consoden::TankGame::Match::ClassTypeId)
    {
        //match deleted
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
    sl.append("Game Over!");
    m_world.SetTextBig(sl);

//    m_updateTimer.stop();
//    auto player1=m_world.GetPlayerOne();
//    auto player2=m_world.GetPlayerTwo();
//    if (!player1 || !player2)
//    {
//        return;
//    }

//    m_tankInfoWidget[0]->SetPoints(m_currentMatch->PlayerOneTotalPoints().GetVal());
//    m_tankInfoWidget[1]->SetPoints(m_currentMatch->PlayerTwoTotalPoints().GetVal());

//    QStringList sl;
//    sl.append("Game Over!");
//    sl.append(player1->name+" vs "+player2->name);
//    sl.append(QString::number(m_world.GetPlayerOneTotalPoints())+" - "+QString::number(m_world.GetPlayerTwoTotalPoints()));
//    m_world.SetTextBig(sl);
}

void MainWindow::OnActionNewGame()
{
    if (!m_dobConnection.IsOpen())
    {
        QMessageBox::information(this, "Not connected", "Can't start a game because we are not connected to the game engine!");
        return;
    }

    NewGameDialog dlg(m_world.GetPlayers());
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

        auto p1=m_world.GetPlayerByName(dlg.Player1());
        auto p2=m_world.GetPlayerByName(dlg.Player2());
        if (p1!=nullptr && p2!=nullptr)
        {
            match->PlayerOneId()=Safir::Dob::Typesystem::InstanceId(p1->id);
            match->PlayerTwoId()=Safir::Dob::Typesystem::InstanceId(p2->id);
        }

        m_tankInfoWidget[0]->SetPoints(0);
        m_tankInfoWidget[1]->SetPoints(0);

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

void MainWindow::OnActionRestartGame()
{
//    if (m_match.NumberOfGames()<=0)
//    {
//        QMessageBox::information(this, "No game to restart", "There is no info about last game. Please start a new game.");
//        return;
//    }
//    if (!m_dobConnection.IsOpen())
//    {
//        QMessageBox::information(this, "Not connected", "Can't start a game because we are not connected to the game engine!");
//        return;
//    }

//    //ReverseTanks()
//    m_match.Reset();
//    if (m_match.NextGame())
//    {
//        SendNewGameRequest();
//    }
}

void MainWindow::OnActionSaveGame()
{
//    if (m_match.NumberOfGames()<=0)
//    {
//        QMessageBox::information(this, "No game to save", "There is no info about last game.");
//        return;
//    }

//    QString path;
//    const char* runtime=getenv("SAFIR_RUNTIME");
//    if (runtime)
//    {
//        path=QDir::cleanPath(QString(runtime)+QDir::separator()+"data"+QDir::separator()+"tank_game");
//    }
//    else
//    {
//        path=".";
//    }

//    path+=QDir::separator()+QString("tank_game_")+QTime::currentTime().toString();

//    for (int i=0; i<m_match.NumberOfGames(); ++i)
//    {
//        QString file=QDir::cleanPath(path+"#"+QString::number(i)+".txt");
//        m_match.BoardAt(i)->Save(file.toStdString());
//    }
//    QString fileNames=QDir::cleanPath(path+"#0.."+QString::number(m_match.NumberOfGames()-1)+".txt");
//    QMessageBox::information(this, "Game field saved", "Game field has been saved to file: "+fileNames);
}
