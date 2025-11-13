#ifndef PLAYGROUND_H
#define PLAYGROUND_H
#include "building.h"
#include "Player.h"
#include "define.h"
#include <QMediaPlayer>
#include <QVideoWidget>
class playground:public building
{
public:
    playground();
    ~playground();
    int play_time;
    int the_time_need;
    int f(int t);//升级所需时间的递增函数
    QString update(Player *player) override;
    QString applyeffect(Player *player) override;
    int getTeleportTarget() {return 6;}
    QMediaPlayer* media_player;
    QVideoWidget* widget_player;
};

#endif // PLAYGROUND_H
