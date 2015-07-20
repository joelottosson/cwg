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
	ui->laserActive->setPixmap(m_blue_off);
	ui->smokeAvailiable->setPixmap(m_green_off);
	ui->SmokeActive->setPixmap(m_blue_off);
	ui->redeemerActive->setPixmap(m_blue_off);
	ui->redeemerAvaliable->setPixmap(m_green_off);

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

void TankInfoWidget::turnOfDirectionLeds(){
    m_buttons[0]->setPixmap(m_green_off);
    m_buttons[1]->setPixmap(m_green_off);
    m_buttons[2]->setPixmap(m_green_off);
    m_buttons[3]->setPixmap(m_green_off);
}

void TankInfoWidget::turnOfTowerLeds(){
    m_buttons[4]->setPixmap(m_green_off);
    m_buttons[5]->setPixmap(m_green_off);
    m_buttons[6]->setPixmap(m_green_off);
    m_buttons[7]->setPixmap(m_green_off);
}

void TankInfoWidget::SetLaserAmmo(int lasers){
	ui->laserLcdNumber->display(lasers);
}

void TankInfoWidget::updateSmoke(bool has_smoke, int smoke_left){
    if(has_smoke){
    	ui->smokeAvailiable->setPixmap(m_green_on);
    }else{
    	ui->smokeAvailiable->setPixmap(m_green_off);
    }
    if(smoke_left > 0){
    	ui->SmokeActive->setPixmap(m_blue_on);
    }else{
    	ui->SmokeActive->setPixmap(m_blue_off);
    }
    ui->smokeLcd->display(smoke_left);
}

void TankInfoWidget::updateReddemerWithTankyStuff(bool available, int timer){
	if(available){
		ui->redeemerAvaliable->setPixmap(m_green_on);
	}else{
		ui->redeemerAvaliable->setPixmap(m_green_off);
	}

    ui->redeemerTimer->display(timer);
    if(timer > 0 ){
    	ui->redeemerActive->setPixmap(m_blue_on);
    }else{
    	ui->redeemerActive->setPixmap(m_blue_off);
    }


}

void TankInfoWidget::Update(const Joystick* js){



    if (!js)
    {
        return;
    }

    ResetLeds();

    if (js->fire)
    {
        SetLed(TankInfoWidget::Fire, true);
    }else{
    	SetLed(TankInfoWidget::Fire, false);
    }

    if(js->laser){
    	ui->laserActive->setPixmap(m_blue_on);
    }else{
    	ui->laserActive->setPixmap(m_blue_off);

    }

    if (js->moveDirection != None){
        TankInfoWidget::Led led=static_cast<TankInfoWidget::Led>(js->moveDirection);
        SetLed(led, true);
    }else{
    	turnOfDirectionLeds();
    }

    if (js->towerDirection!=None){
        TankInfoWidget::Led led=static_cast<TankInfoWidget::Led>(
                    static_cast<int>(TankInfoWidget::TowerLeft)+
                    static_cast<int>(js->towerDirection));
        SetLed(led, true);
    }else{
    	turnOfTowerLeds();
    }
}
