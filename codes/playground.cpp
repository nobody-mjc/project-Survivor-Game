#include "playground.h"

playground::playground() :
    play_time(0),
    the_time_need(3000)
{
    setPos(185,210);
}

QString playground::update(Player *player){
    play_time+=1000;
    QString end="";
    if(play_time>=the_time_need){
        play_time=0;
        the_time_need=f(the_time_need);
        end=applyeffect(player);
    }
    return end;
}
int playground::f(int t){
    return t+1000;
}
QString playground::applyeffect(Player *player){
    player->addDamage(PLAYER_DAMAGE/2);
    QString end="攻击力++";
    return end;
}
