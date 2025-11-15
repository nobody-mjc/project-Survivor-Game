#ifndef SUPERMARKET_H
#define SUPERMARKET_H
#include "building.h"

class Supermarket: public building
{
public:
    Supermarket();
    ~Supermarket();
    QString update (Player *player);
    QString applyeffect(Player *player);
    int getTeleportTarget(){return 4;}
    bool buyFood(Player *player);
    bool buyBullet(Player *player);
};

#endif // SUPERMARKET_H
