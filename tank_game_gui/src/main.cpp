/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    int updateFrequency=30;
    if (argc>1)
    {
        updateFrequency=QString(argv[1]).toInt();
    }
    QApplication a(argc, argv);
    MainWindow w(updateFrequency);
    w.show();
    return a.exec();
}
