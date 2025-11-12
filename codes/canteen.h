#ifndef CANTEEN_H
#define CANTEEN_H
#include "building.h"

class Canteen:public building
{
public:
    Canteen();
    QString update(Player *player);
    QString applyeffect(Player *player);
    int getTeleportTarget() {return 5;}
    QString randomEvent(double random, Player *player);
};

#endif // CANTEEN_H
