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

void ProbabilityTeacher::apply_skill(Player *player){}

StructureTeacher::StructureTeacher(){
    setPixmap(QPixmap(TEACHER_2_PATH).scaled(200, 200, Qt::KeepAspectRatio));
    setTransformOriginPoint(boundingRect().center());
}

void StructureTeacher::apply_skill(Player *player){}

AITeacher::AITeacher(){
    setPixmap(QPixmap(TEACHER_3_PATH).scaled(200, 200, Qt::KeepAspectRatio));
    setTransformOriginPoint(boundingRect().center());
}

void AITeacher::apply_skill(Player *player){}

ConvexTeacher::ConvexTeacher(){
    setPixmap(QPixmap(TEACHER_4_PATH).scaled(200, 200, Qt::KeepAspectRatio));
    setTransformOriginPoint(boundingRect().center());
}

void ConvexTeacher::apply_skill(Player *player){}

ProgrammingTeacher::ProgrammingTeacher(){
    setPixmap(QPixmap(TEACHER_5_PATH).scaled(200, 200, Qt::KeepAspectRatio));
    setTransformOriginPoint(boundingRect().center());
}

void ProgrammingTeacher::apply_skill(Player *player){}

SurpriseTeacher::SurpriseTeacher(){
    setPixmap(QPixmap(TEACHER_6_PATH).scaled(200, 200, Qt::KeepAspectRatio));
    setTransformOriginPoint(boundingRect().center());
}

void SurpriseTeacher::apply_skill(Player *player){}
