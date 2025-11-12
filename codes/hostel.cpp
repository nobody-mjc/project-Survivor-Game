#include "hostel.h"

hostel::hostel()
{
    setPos(405, 230);
}

QString hostel::update(Player *player){
    QString end="";
    end=applyeffect(player);
    return end;
}

QString hostel::applyeffect(Player *player){
    int healthBefore = player->getHealth();
    player->addHealth(HEALTH_RECOVER);
    int healthAfter = player->getHealth();
    QString end="生命值" + QString::number(healthAfter - healthBefore);
    return end;
}
