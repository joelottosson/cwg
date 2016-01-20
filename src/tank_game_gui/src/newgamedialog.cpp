/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include <QMessageBox>
#include <QFileDialog>
#include <QTime>
#include <Consoden/TankGame/Boards.h>
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

void NewGameDialog::on_browseButton_clicked()
{
    QString path=QDir::cleanPath(QString(Safir::Dob::Typesystem::Utilities::ToUtf8(Consoden::TankGame::Boards::Path()).c_str()));
    QStringList fileNames=QFileDialog::getOpenFileNames(this, "Open game file", path, "Files (*.txt)" , 0, QFileDialog::DontUseNativeDialog);
    ui->boardListWidget->addItems(fileNames);
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
    return ui->repeatSpinBox->value();
}

QStringList NewGameDialog::Boards() const
{
    QStringList sl;
    for (int i=0; i<ui->boardListWidget->count(); ++i)
    {
        sl.append(ui->boardListWidget->item(i)->text());
    }

    return sl;
}

int NewGameDialog::GameTime() const
{
    return ui->gameTimeSpinBox->value();
}
