#ifndef HOSTEL_H
#define HOSTEL_H
#include"building.h"
class hostel:public building
{
public:
    hostel();
    QString update(Player *player);
    QString applyeffect(Player *player);
    int getTeleportTarget() {return 2;}
    int rest_time;
    int the_time_need;
};

#endif // HOSTEL_H
