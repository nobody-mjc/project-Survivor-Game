#ifndef BUILDING_H
#define BUILDING_H
#include <QGraphicsPixmapItem>
#include<Qstring>
#include "Player.h"
class building:public QGraphicsPixmapItem
{
    public:
        building();
        virtual QString update(Player *player)=0;
        virtual QString applyeffect(Player *player)=0;
};

#endif // BUILDING_H
