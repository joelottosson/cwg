/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QEvent>
#include <QThread>
#include <QTime>
#include <qtimer.h>
#include <Safir/Dob/Connection.h>
#include <Consoden/TankGame/Match.h>
#include "tankgamewidget.h"
#include "tankinfowidget.h"
#include "gamemodel.h"
#include "newgamedialog.h"

//----------------------------------------
class DobConnector : public QThread
{
    Q_OBJECT
public:
    DobConnector(Safir::Dob::Connection* con, Safir::Dob::StopHandler* s, Safir::Dob::Dispatcher* d, QObject* /*parent*/)
        :m_con(con)
        ,m_stop(s)
        ,m_disp(d)
    {
    }

signals:
    void ConnectedToDob();

protected:
    virtual void run()
    {
        m_con->Open(L"tank_game_gui", QTime::currentTime().toString("hh:mm:ss.zzz").toStdWString(), 0, m_stop, m_disp);
        m_con->Close();
        emit ConnectedToDob();
    }

private:
    Safir::Dob::Connection* m_con;
    Safir::Dob::StopHandler* m_stop;
    Safir::Dob::Dispatcher* m_disp;
};
//----------------------------------------

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow,
        public Safir::Dob::StopHandler,
        public Safir::Dob::Dispatcher,
        public Safir::Dob::EntitySubscriber,
        public Safir::Dob::Requestor
{
    Q_OBJECT
    
public:
    explicit MainWindow(int updateFrequency, QWidget *parent = 0);
    ~MainWindow();

    virtual bool eventFilter(QObject* o, QEvent* e);

    //Dob stuff
    virtual void OnDoDispatch();
    virtual void OnStopOrder();
    virtual void OnNewEntity(const Safir::Dob::EntityProxy entityProxy);
    virtual void OnUpdatedEntity(const Safir::Dob::EntityProxy entityProxy);
    virtual void OnDeletedEntity(const Safir::Dob::EntityProxy entityProxy, const bool /*del*/);
    virtual void OnResponse(const Safir::Dob::ResponseProxy responseProxy);
    virtual void OnNotRequestOverflow();

private:
    Ui::MainWindow *ui;
    const int m_updateInterval;
    TankGameWidget* m_tankGameWidget;
    TankInfoWidget* m_tankInfoWidget[2];
    GameWorld m_world;
    Safir::Dob::Connection m_dobConnection;
    QEvent::Type m_dispatchEvent;
    DobConnector m_conThread;
    QTimer m_updateTimer;

    void SendNewGameRequest();
    void MatchFinished();

private slots:
    void OnUpdateWorld();
    void OnConnected();
    void OnActionNewGame();
    void OnActionRestartGame();
    void OnActionStopGame();
    void UpdatePoints();
};



#endif // MAINWINDOW_H
