#include "canteen.h"


Canteen::Canteen()
{
    setPos(465, 455);
}

QString Canteen::update(Player *player){}

QString Canteen::applyeffect(Player *player){
    player->addFoodGuage(20);
    return "";
}

QString Canteen::randomEvent(double random, Player *player){
    QString end = "";
    if(random < 0.3){
        end = "中毒";
    } else {
        end = "获得 20 点饱食度";
        applyeffect(player);
    }
    return end;
}
