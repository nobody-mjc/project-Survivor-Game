#include "Player.h"
#include "define.h"
#include <QGraphicsScene>
#include <QPainter>
#include <QRandomGenerator>
#include <QDateTime>
#include <QTimer>

Player::Player(QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent), 
      health(INITIAL_PLAYER_HEALTH), 
      maxHealth(INITIAL_PLAYER_HEALTH), 
      ammo(INITIAL_PLAYER_AMMO), 
      speed(PLAYER_SPEED), 
      damage(PLAYER_DAMAGE), 
      fireRate(FIRE_RATE), 
      lastShootTime(0),
      foodGauge(INITIAL_FOOD_GAUGE)
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
}

void Player::rotateToMouse(const QPointF &targetPos)
{
    // 计算玩家指向鼠标的角度
    QLineF line(pos() + boundingRect().center(), targetPos);
    setRotation(-line.angle());
}

Bullet* Player::shoot(const QPointF &targetPos)
{
    // 检查弹药是否充足
    if (ammo <= 0) return nullptr;
    
    // 检查射击频率
    int currentTime = QDateTime::currentMSecsSinceEpoch();
    if (currentTime - lastShootTime < fireRate) return nullptr;
    
    lastShootTime = currentTime;
    ammo--;
    
    // 旋转玩家朝向鼠标
    rotateToMouse(targetPos);
    
    // 创建子弹
    Bullet *bullet = new Bullet(damage);
    
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
