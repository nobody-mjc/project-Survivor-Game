#ifndef LIBRARY_H
#define LIBRARY_H

#include "building.h"

class Library : public building
{
public:
    Library();
    QString update(Player *player) override;
    QString applyeffect(Player *player) override;
    int getTeleportTarget() override { return 8; }
};

#endif // LIBRARY_H
