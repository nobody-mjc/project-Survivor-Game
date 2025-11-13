#ifndef HOSTEL_H
#define HOSTEL_H
#include"building.h"
class hostel:public building
{
public:
    hostel();
    QString update(Player *player);
    QString applyeffect(Player *player);
    int getTeleportTarget() {return 7;}
};

#endif // HOSTEL_H
