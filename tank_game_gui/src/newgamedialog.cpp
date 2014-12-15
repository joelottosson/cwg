/******************************************************************************
*
* Copyright Consoden AB, 2014
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include <QMessageBox>
#include <QFileDialog>
#include <QTime>
#include <boost/filesystem.hpp>
#include "newgamedialog.h"
#include "ui_newgamedialog.h"

NewGameDialog::NewGameDialog(const PlayerMap& players, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewGameDialog)
{
    ui->setupUi(this);
    srand(time(NULL));

    if (players.empty())
        return;

    //fill player comboboxes
    ui->player1ComboBox->setEditable(false);
    ui->player2ComboBox->setEditable(false);
    for (auto vt : players)
    {
        ui->player1ComboBox->addItem(vt.second.name);
        ui->player2ComboBox->addItem(vt.second.name);
    }
    ui->player1ComboBox->setCurrentIndex(0);
    ui->player2ComboBox->setCurrentIndex(std::min(1, static_cast<int>(players.size()-1)));
}

NewGameDialog::~NewGameDialog()
{
    delete ui;
}

void NewGameDialog::OnAddRandomBoard()
{
    ui->boardListWidget->addItem("<generate_random>");
}

QString NewGameDialog::Player1() const
{
    return ui->player1ComboBox->currentText();
}

QString NewGameDialog::Player2() const
{
    return ui->player2ComboBox->currentText();
}

int NewGameDialog::Repetitions() const
{
    return 1;
}

QStringList NewGameDialog::Boards() const
{
    QStringList sl;
    sl.append("<generate_random>");
    return sl;
}

int NewGameDialog::GameTime() const
{
    return 30;
}
