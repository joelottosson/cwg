/******************************************************************************
*
* Copyright Consoden AB, 2014
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#ifndef NEWGAMEDIALOG_H
#define NEWGAMEDIALOG_H

#include <QDialog>
#include <QString>
#include <QComboBox>
#include <boost/shared_ptr.hpp>
#include "gamemodel.h"

namespace Ui {
class NewGameDialog;
}

class NewGameDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit NewGameDialog(const PlayerMap& players, QWidget *parent = 0);
    ~NewGameDialog();

    QString Player1() const;
    QString Player2() const;
    int Repetitions() const;
    QStringList Boards() const;
    int GameTime() const;
    
private slots:
    void OnAddRandomBoard();
    void on_browseButton_clicked();

private:
    Ui::NewGameDialog *ui;
};

#endif // NEWGAMEDIALOG_H
