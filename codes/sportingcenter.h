#ifndef SPORTINGCENTER_H
#define SPORTINGCENTER_H

#include "building.h"

class SportingCenter: public building
{
public:
    SportingCenter();
    QString update (Player *player);
    QString applyeffect(Player *player);
    int getTeleportTarget(){return 9;}
};

#endif // SPORTINGCENTER_H
