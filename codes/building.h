#ifndef BUILDING_H
#define BUILDING_H
#include <QGraphicsPixmapItem>
#include "Player.h"
class building:public QGraphicsPixmapItem
{
    public:
        building();
        virtual void update(Player *player)=0;

};

#endif // BUILDING_H
