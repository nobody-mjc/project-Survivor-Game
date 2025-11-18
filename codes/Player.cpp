#include "Player.h"
#include "define.h"
#include <QGraphicsScene>
#include <QPainter>
#include <QRandomGenerator>
#include <QDateTime>
#include <QTimer>
#include <QTransform>


Player::Player(QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent),
    health(INITIAL_PLAYER_HEALTH),
    maxHealth(INITIAL_PLAYER_HEALTH),
    ammo(INITIAL_PLAYER_AMMO),
    speed(PLAYER_SPEED),
    damage(PLAYER_DAMAGE),
    fireRate(FIRE_RATE),
    foodGauge(INITIAL_FOOD_GAUGE),
    time_for_hunger(0),
    money(INITIAL_MONEY),
    crit_rate(0.1),
    crit(2)
{
    loadSprite();
    setTransformOriginPoint(boundingRect().center());
}

Player::~Player()
{
}

void Player::loadSprite()
{
    // 尝试加载图片文件
    QPixmap pixmap(PLAYER_IMAGE_PATH);

    // 如果图片加载失败，使用默认绘制的图形
    if (pixmap.isNull()) {
        pixmap = QPixmap(PLAYER_SIZE, PLAYER_SIZE);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setBrush(PLAYER_COLOR);
        painter.drawEllipse(5, 5, PLAYER_SIZE - 10, PLAYER_SIZE - 10);
        painter.setBrush(Qt::white);
        painter.drawEllipse(15, 15, 8, 8); // 眼睛
        painter.drawEllipse(27, 15, 8, 8);
        painter.setBrush(Qt::red);
        painter.drawRect(15, 30, 20, 5);  // 枪
    }

    setPixmap(pixmap);
}

void Player::updateMovement(bool keys[])
{
    // 根据按键状态移动玩家
    int dx = 0, dy = 0;
    if (keys[0]) dy -= speed; // 上
    if (keys[1]) dy += speed; // 下
    if (keys[2]) dx -= speed; // 左
    if (keys[3]) dx += speed; // 右

    // 计算新位置
    QPointF newPos = pos() + QPointF(dx, dy);

    // 边界检查
    if (newPos.x() < 0) newPos.setX(0);
    if (newPos.x() > GAME_WIDTH - boundingRect().width())
        newPos.setX(GAME_WIDTH - boundingRect().width());
    if (newPos.y() < 0) newPos.setY(0);
    if (newPos.y() > GAME_HEIGHT - boundingRect().height())
        newPos.setY(GAME_HEIGHT - boundingRect().height());

    setPos(newPos);
    time_for_hunger++;
    if(time_for_hunger>=time_need_for_hunger){
        takeFoodGauge(FOOD_GAUGE_CONSUME);
        time_for_hunger=0;
    }
    // 左右移动改变朝向
    if (dx > 0) {
        facingRight = true;
        QTransform t;
        setTransform(t);
    } else if (dx < 0) {
        facingRight = false;
        QTransform t(-1, 0, 0, 1, boundingRect().width(), 0);
        setTransform(t);
    }
}

void Player::rotateToMouse(const QPointF &targetPos)
{
    qreal centerX = pos().x() + boundingRect().width() / 2.0;

    if (targetPos.x() >= centerX) {
        facingRight = true;
        QTransform t;
        setTransform(t);
    } else {
        facingRight = false;
        QTransform t(-1, 0, 0, 1, boundingRect().width(), 0);
        setTransform(t);
    }
}

Bullet* Player::shoot(const QPointF &targetPos)
{
    // 检查弹药是否充足
    if (ammo <= 0) return nullptr;

    ammo--;

    // 旋转玩家朝向鼠标
    rotateToMouse(targetPos);

    // 创建子弹
    float randomVal = QRandomGenerator::global()->bounded(1, 101);
    int d=damage;
    if(randomVal/100>crit_rate)d*=crit;
    Bullet *bullet = new Bullet(d);

    // 设置子弹初始位置（玩家中心）
    bullet->setPos(pos() + boundingRect().center() - bullet->boundingRect().center());

    // 计算子弹方向
    QLineF line(pos() + boundingRect().center(), targetPos);
    bullet->setRotation(-line.angle());

    // 设置子弹速度方向
    bullet->setDirection(line.unitVector().dx(), line.unitVector().dy());

    return bullet;
}

void Player::takeDamage(int damage)
{
    health -= damage;
    if (health < 0) health = 0;

    // 受伤闪烁效果
    setOpacity(0.7);
    QTimer::singleShot(100, [=]() {
        setOpacity(1.0);
    });
}

void Player::takeFoodGauge(float decrease){
    foodGauge -= decrease;
    if(foodGauge < 0) foodGauge = 0;
}

void Player::addHealth(int amount)
{
    health += amount;
    if (health > maxHealth) health = maxHealth;
}

void Player::addAmmo(int amount)
{
    ammo += amount;
    if (ammo < 0) ammo = 0;
}
void Player::addDamage(int addition){
    damage+=addition;
}

void Player::addFoodGuage(float increase){
    foodGauge += increase;
    if(foodGauge > INITIAL_FOOD_GAUGE) foodGauge = INITIAL_FOOD_GAUGE;
}
void Player::addMoney(float increase){
    money += increase;
}
void Player::add_crit_rate(float increase){
    crit_rate+=increase;
}
void Player::add_MaxHealth(int increase){
    maxHealth+=increase;
}
void Player::save(std::string saving_name,int mapId){
    saving_name+=".txt";
    std::ofstream savingFile("qrc:/saves/"+saving_name,std::ios::out);
    savingFile<<facingRight<<std::endl;
    savingFile<<health<<std::endl;
    savingFile<<maxHealth<<std::endl;
    savingFile<<ammo<<std::endl;
    savingFile<<speed<<std::endl;
    savingFile<<damage<<std::endl;
    savingFile<<fireRate<<std::endl;
    savingFile<<lastShootTime<<std::endl;
    savingFile<<time_for_hunger<<std::endl;
    savingFile<<foodGauge<<std::endl;
    savingFile<<money<<std::endl;
    savingFile<<crit<<std::endl;
    savingFile<<crit_rate<<std::endl;
    savingFile<<pos().x()<<" "<<pos().y()<<std::endl;
    savingFile<<mapId<<std::endl;
}
int Player::read_saving(std::string saving_path){
    std::ifstream savingFile(saving_path);
    int x,y,mapId;
    savingFile>>facingRight
        >>health
        >>maxHealth
        >>ammo
        >>speed
        >>damage
        >>fireRate
        >>lastShootTime
        >>time_for_hunger
        >>foodGauge
        >>money
        >>crit
        >>crit_rate
        >>x
        >>y
        >>mapId;

    setPos(x,y);
    return mapId;
}
