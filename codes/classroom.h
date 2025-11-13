#ifndef CLASSROOM_H
#define CLASSROOM_H
#include "building.h"
#include "teacher.h"

class Classroom:public building
{
public:
    Classroom();
    QString update(Player *player);
    QString applyeffect(Player *player);
    int getTeleportTarget() {return 3;}
private:
    teacher *oneTeacher;
};

#endif // CLASSROOM_H
