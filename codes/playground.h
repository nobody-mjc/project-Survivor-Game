#ifndef PLAYGROUND_H
#define PLAYGROUND_H
#include "building.h"
#include "Player.h"
#include "define.h"
class playground:public building
{
public:
    playground();

    int play_time;
    int the_time_need;
    int f(int t);//升级所需时间的递增函数
    void update(Player *player) override;
    void applyeffect(Player *player);

};

#endif // PLAYGROUND_H
