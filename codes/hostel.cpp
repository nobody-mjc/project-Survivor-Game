#include "hostel.h"

hostel::hostel() :
    rest_time(0),
    the_time_need(3000)
{

}
QString hostel::update(Player *player){
    QString end="";
    rest_time+=1000;
    if(rest_time>=the_time_need){
        rest_time=0;
        end=applyeffect(player);
    }
    return end;
}
QString hostel::applyeffect(Player *player){
    player->addHealth(20);
    QString end="生命值+20";
    return end;
}
