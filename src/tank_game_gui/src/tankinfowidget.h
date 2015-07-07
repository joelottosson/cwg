/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#ifndef TANKINFOWIDGET_H
#define TANKINFOWIDGET_H

#include <QLabel>
#include <QWidget>
#include <QMouseEvent>
#include "gamemodel.h"

namespace Ui {
class TankInfoWidget;
}

class TankInfoWidget : public QWidget
{
    Q_OBJECT

public:

    enum Led {MoveLeft, MoveRight, MoveUp, MoveDown, TowerLeft, TowerRight, TowerUp, TowerDown, Fire};
    explicit TankInfoWidget(int playerNumber, QWidget *parent = 0);
    ~TankInfoWidget();

    void SetName(const QString& name);
    void SetPoints(int points);
    void SetLaserAmmo(int lasers);
    void Update(const Joystick* js);

    void ResetLeds();
    void SetLed(Led led, bool on);

    void turnOfDirectionLeds();
    void turnOfTowerLeds();

private:
    Ui::TankInfoWidget *ui;
    int m_playerNumber;
    QLabel* m_buttons[9];
    QPixmap m_green_on;
    QPixmap m_green_off;
    QPixmap m_blue_on;
    QPixmap m_blue_off;
};

#endif // TANKINFOWIDGET_H
