#include "teacher.h"

teacher::teacher() {
    effect = new QGraphicsOpacityEffect();
    effect->setOpacity(0.0);
    setGraphicsEffect(effect);

    animation = new QPropertyAnimation(effect, "opacity");
    animation->setDuration(3000);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->setEasingCurve(QEasingCurve::InOutQuad);
}

void teacher::show(QGraphicsScene *scene){
    if(!scene) return;
    scene->addItem(this);
    animation->start();
}

teacher::~teacher(){
    animation->stop();
    delete animation;
    delete effect;
}

ProbabilityTeacher::ProbabilityTeacher(){
    setPixmap(QPixmap(TEACHER_1_PATH).scaled(200, 200, Qt::KeepAspectRatio));
    setTransformOriginPoint(boundingRect().center());
}

QString ProbabilityTeacher::apply_skill(Player *player){
    player->add_crit_rate(0.01);
    QString end = "获得技能：正态分布！";
    return end;
}

StructureTeacher::StructureTeacher(){
    setPixmap(QPixmap(TEACHER_2_PATH).scaled(200, 200, Qt::KeepAspectRatio));
    setTransformOriginPoint(boundingRect().center());
}

QString StructureTeacher::apply_skill(Player *player){
    player->add_MaxHealth(30);
    QString end = "获得技能：";
    return end;
}

AITeacher::AITeacher(){
    setPixmap(QPixmap(TEACHER_3_PATH).scaled(200, 200, Qt::KeepAspectRatio));
    setTransformOriginPoint(boundingRect().center());
}

QString AITeacher::apply_skill(Player *player){
    QString end = "获得技能：";
    return end;
}

ConvexTeacher::ConvexTeacher(){
    setPixmap(QPixmap(TEACHER_4_PATH).scaled(200, 200, Qt::KeepAspectRatio));
    setTransformOriginPoint(boundingRect().center());
}

QString ConvexTeacher::apply_skill(Player *player){
    QString end = "获得技能：";
    return end;
}

ProgrammingTeacher::ProgrammingTeacher(){
    setPixmap(QPixmap(TEACHER_5_PATH).scaled(200, 200, Qt::KeepAspectRatio));
    setTransformOriginPoint(boundingRect().center());
}

QString ProgrammingTeacher::apply_skill(Player *player){
    QString end = "获得技能：";
    return end;
}

SurpriseTeacher::SurpriseTeacher(){
    setPixmap(QPixmap(TEACHER_6_PATH).scaled(200, 200, Qt::KeepAspectRatio));
    setTransformOriginPoint(boundingRect().center());
}

QString SurpriseTeacher::apply_skill(Player *player){
    QString end = "获得技能：";
    return end;
}
