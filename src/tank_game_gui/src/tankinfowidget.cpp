/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include "tankinfowidget.h"
#include "ui_tankinfowidget.h"
#include <iostream>

#include <qmessagebox.h>

TankInfoWidget::TankInfoWidget(int playerNumber, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TankInfoWidget),
    m_playerNumber(playerNumber),
	m_green_on(":/images/active-green.png"),
	m_green_off(":/images/inactive-green.png"),
    m_blue_on(":/images/active-blue.png"),
    m_blue_off(":/images/inactive-blue.png")


{
    ui->setupUi(this);
    m_buttons[0]=ui->moveLeft;
    m_buttons[1]=ui->moveRight;
    m_buttons[2]=ui->moveUp;
    m_buttons[3]=ui->moveDown;
    m_buttons[4]=ui->towerLeft;
    m_buttons[5]=ui->towerRight;
    m_buttons[6]=ui->towerUp;
    m_buttons[7]=ui->towerDown;
    m_buttons[8]=ui->fireButton;
    ResetLeds();

    if (m_playerNumber==0)
    {
        ui->playerName->setStyleSheet("QLabel { color : blue; }");
    }
    else
    {
        ui->playerName->setStyleSheet("QLabel { color : red; }");
    }
}

TankInfoWidget::~TankInfoWidget()
{
    delete ui;
}

void TankInfoWidget::SetName(const QString& name)
{
    ui->playerName->setText(name);
}

void TankInfoWidget::SetPoints(int points)
{
    ui->pointsLcdNumber->display(points);
}

void TankInfoWidget::ResetLeds()
{
    for (size_t i=0; i<9; ++i)
    {
    	if(i==8){
    		m_buttons[8]->setPixmap(m_blue_off);
    	}else{
    		m_buttons[i]->setPixmap(m_green_off);
    	}

    }
}

void TankInfoWidget::SetLed(Led led, bool on)
{
    size_t index=static_cast<size_t>(led);
    if (on)
    {
        if (led==Fire)
        {
            m_buttons[8]->setPixmap(m_blue_on);
        }
        else
        {
            m_buttons[index]->setPixmap(m_green_on);
        }
    }
    else
    {
    	if(led == Fire){
    		m_buttons[8]->setPixmap(m_blue_off);
    	}else{
    		m_buttons[index]->setPixmap(m_green_off);
    	}
    }
}

void TankInfoWidget::Update(const Joystick* js)
{
    ResetLeds();
    if (!js)
    {
        return;
    }
    if (js->fire)
    {
        SetLed(TankInfoWidget::Fire, true);
    }else{
    	SetLed(TankInfoWidget::Fire, false);
    }
    if (js->moveDirection!=None)
    {
        TankInfoWidget::Led led=static_cast<TankInfoWidget::Led>(js->moveDirection);
        SetLed(led, true);
    }
    if (js->towerDirection!=None)
    {
        TankInfoWidget::Led led=static_cast<TankInfoWidget::Led>(
                    static_cast<int>(TankInfoWidget::TowerLeft)+
                    static_cast<int>(js->towerDirection));
        SetLed(led, true);
    }
}
