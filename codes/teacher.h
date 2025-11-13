#ifndef TEACHER_H
#define TEACHER_H
#include"define.h"
#include<QGraphicsPixmapItem>
#include <QGraphicsOpacityEffect>
#include<QPropertyAnimation>
#include<QGraphicsScene>
#include"Player.h"
class teacher:public QGraphicsPixmapItem
{
private:
    QGraphicsOpacityEffect *effect;
    QPropertyAnimation *animation;
public:
    teacher();
    virtual void apply_skill(Player *player)=0;
    virtual void show(QGraphicsScene *scene);
};

#endif // TEACHER_H
