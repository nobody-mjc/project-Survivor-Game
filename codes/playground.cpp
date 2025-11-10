#include "playground.h"

playground::playground() :
    play_time(0),
    the_time_need(3000)
{
    setPos(185,210);
}
void playground::update(Player *player){
    play_time+=1000;
    if(play_time>=the_time_need){
        play_time=0;
        the_time_need=f(the_time_need);
        applyeffect(player);
    }
}
int playground::f(int t){
    return t+1000;
}
void playground::applyeffect(Player *player){
    player->addDamage(PLAYER_DAMAGE/2);
}
