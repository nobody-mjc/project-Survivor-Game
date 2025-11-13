#ifndef CLASSROOM_H
#define CLASSROOM_H
#include "building.h"

class Classroom:public building
{
public:
    Classroom();
    QString update(Player *player);
    QString applyeffect(Player *player);
    int getTeleportTarget() {return 3;}
};

#endif // CLASSROOM_H
