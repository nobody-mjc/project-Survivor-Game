#include "playground.h"
#include "define.h"
playground::playground() :
    play_time(0),
    the_time_need(3000)

{
    setPos(185,210);
    media_player=new QMediaPlayer;
    media_player->setSource(QUrl(":/runningman.mp4"));
    widget_player = new QVideoWidget;
    media_player->setVideoOutput(widget_player);
}

playground::~playground(){
    delete media_player;
    delete widget_player;
}

QString playground::update(Player *player){
    QString end="";
    end=applyeffect(player);
    return end;
}
int playground::f(int t){
    return t+1000;
}
QString playground::applyeffect(Player *player){
    player->addDamage(PLAYER_DAMAGE/2);
    widget_player->show();
    media_player->play();
    QAction::connect(media_player,&QMediaPlayer::playbackStateChanged,[=](){
        media_player->stop();
        widget_player->hide();
    });
    player->addDamage(10);
    QString end="攻击力++";
    return end;
}
