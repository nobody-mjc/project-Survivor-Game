#include "SurvivorGame.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SurvivorGame w;
    w.show();
    
    return a.exec();
}