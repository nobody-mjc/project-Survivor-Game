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

//下面是各个建筑的ID
//食堂：事件型建筑 —— 5
//紫色操场：事件型建筑 —— 6
//诚园宿舍：事件型建筑 —— 7
//南雍楼：地图型建筑 —— 3
//全家超市：地图型建筑 —— 4
