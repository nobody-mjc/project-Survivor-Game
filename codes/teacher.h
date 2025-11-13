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
    ~teacher();
    virtual void apply_skill(Player *player)=0;
    virtual void show(QGraphicsScene *scene);
};

class ProbabilityTeacher: public teacher
{
public:
    ProbabilityTeacher();
    void apply_skill(Player *player);
};

class StructureTeacher: public teacher
{
public:
    StructureTeacher();
    void apply_skill(Player *player);
};

class AITeacher: public teacher
{
public:
    AITeacher();
    void apply_skill(Player *player);
};

class ConvexTeacher: public teacher
{
public:
    ConvexTeacher();
    void apply_skill(Player *player);
};

class ProgrammingTeacher: public teacher
{
public:
    ProgrammingTeacher();
    void apply_skill(Player *player);
};

class SurpriseTeacher: public teacher
{
public:
    SurpriseTeacher();
    void apply_skill(Player *player);
};

#endif // TEACHER_H

// 1 概率论 （增加暴击率）
// 2 数据结构  (增加生命上限)
// 3 人工智能导论
// 4 凸优化
// 5 程设实训
// 6 surprise
