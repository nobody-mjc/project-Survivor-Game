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
    player->addHealth(HEALTH_RECOVER);
    QString end="生命值+100";
    return end;
}
