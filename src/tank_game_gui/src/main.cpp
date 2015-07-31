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
    bool override_freq = false;
    bool override_sound = false;


    for (int i=1; i<argc; ++i)
    {
        QString param(argv[i]);
        if (param=="--no-sound")
        {
            soundEnabled=false;
            override_sound = true;
        }
        else
        {
            bool isNumber=false;
            int frequency=param.toInt(&isNumber);
            if (isNumber)
            {
                updateFrequency=frequency;
                override_freq = true;
            }
        }
    }

    QApplication a(argc, argv);
    MainWindow w(updateFrequency, soundEnabled,override_sound, override_freq);
    w.show();
    return a.exec();
}
