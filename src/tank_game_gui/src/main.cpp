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
    bool soundEnabled=true;

    for (int i=1; i<argc; ++i)
    {
        QString param(argv[i]);
        if (param=="--no-sound")
        {
            soundEnabled=false;
        }
        else
        {
            bool isNumber=false;
            int frequency=param.toInt(&isNumber);
            if (isNumber)
            {
                updateFrequency=frequency;
            }
        }
    }

    QApplication a(argc, argv);
    MainWindow w(updateFrequency, soundEnabled);
    w.show();
    return a.exec();
}
