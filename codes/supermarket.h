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
    int getTeleportTarget() const override {return 4;}
};

#endif // SUPERMARKET_H
