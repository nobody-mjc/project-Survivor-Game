#ifndef BUILDING_H
#define BUILDING_H
#include <QGraphicsPixmapItem>
#include<Qstring>
#include "Player.h"
#include "define.h"

class building:public QGraphicsPixmapItem
{
    public:
        building();
        virtual QString update(Player *player)=0;
        virtual QString applyeffect(Player *player)=0;
        virtual int getTeleportTarget()=0;
};

#endif // BUILDING_H

//下面是各个建筑的IDd
//食堂：事件型建筑 —— 5 NO
//紫色操场：事件型建筑 —— 6 OK
//诚园宿舍：事件型建筑 —— 7 NO
//图书馆：事件型建筑--8 NO
//南雍楼：地图型建筑 —— 3 NO
//全家超市：地图型建筑 —— 4 OK
