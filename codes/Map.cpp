#include "Map.h"
#include "define.h"
#include <QGraphicsScene>
#include <QPainter>
#include <QRandomGenerator>
#include <QRandomGenerator>

Map::Map(int mapId, QGraphicsScene *scene, QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent), mapId(mapId), scene(scene), nextPortalId(0)
{
    loadMap();
}

Map::~Map()
{
}

void Map::loadMap()
{
    // 根据地图ID加载对应的背景图片
    QPixmap backgroundImage;
    
    if (mapId == 1) {
        backgroundImage = QPixmap(MAP_1_BACKGROUND_PATH);
    } else if (mapId == 2) {
        backgroundImage = QPixmap(MAP_2_BACKGROUND_PATH);
    } else if(mapId == 3) {
        backgroundImage = QPixmap(MAP_3_BACKGROUND_PATH);
    } else {
        backgroundImage = QPixmap(MAP_4_BACKGROUND_PATH);
    }

    if (!backgroundImage.isNull()) {
        // 如果成功加载图片，设置为背景
        setPixmap(backgroundImage.scaled(GAME_WIDTH, GAME_HEIGHT, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        // 添加到场景并设置Z值
        scene->addItem(this);
        setZValue(-1); // 确保背景在最底层
    } else {
        // 如果没有图片，则使用默认的渐变背景
        drawBackground();
    }
    
    // 添加传送门
    if (mapId == 1 || mapId == 3 || mapId == 4) {
        addTeleportPortal(2, QPointF(TELEPORT_MAP_1_POS_X, TELEPORT_MAP_1_POS_Y), QSizeF(TELEPORT_WIDTH, TELEPORT_HEIGHT));
        drawPortals();
    } else if (mapId == 2) {
        // 地图2 → 地图1
        addTeleportPortal(1, QPointF(TELEPORT_MAP_2_POS_X, TELEPORT_MAP_2_POS_Y), QSizeF(TELEPORT_WIDTH, TELEPORT_HEIGHT));
        drawPortals();
    }

    // 生成障碍物
    generateObstacles();
}

void Map::drawBackground()
{
    // 创建一个渐变背景
    QPixmap pixmap(GAME_WIDTH, GAME_HEIGHT);
    QPainter painter(&pixmap);
    
    // 绘制渐变背景
    QLinearGradient gradient(0, 0, 0, GAME_HEIGHT);
    gradient.setColorAt(0, MAP_BG_TOP_COLOR);
    gradient.setColorAt(1, MAP_BG_BOTTOM_COLOR);
    painter.fillRect(0, 0, GAME_WIDTH, GAME_HEIGHT, gradient);
    
    // 绘制网格
    painter.setPen(MAP_GRID_COLOR);
    for (int i = 0; i <= GAME_WIDTH; i += MAP_GRID_SIZE) {
        painter.drawLine(i, 0, i, GAME_HEIGHT);
    }
    for (int i = 0; i <= GAME_HEIGHT; i += MAP_GRID_SIZE) {
        painter.drawLine(0, i, GAME_WIDTH, i);
    }
    
    setPixmap(pixmap);
    scene->addItem(this);
    setZValue(-1); // 确保背景在最底层
}

void Map::generateObstacles()
{
    // 第一张地图生成障碍物，第二张地图暂时不生成
    if (mapId == 1) {
        for (int i = 0; i < MAP_OBSTACLE_COUNT; ++i) {
            int x = QRandomGenerator::global()->bounded(GAME_WIDTH - OBSTACLE_SIZE * 2) + OBSTACLE_SIZE;
            int y = QRandomGenerator::global()->bounded(GAME_HEIGHT - OBSTACLE_SIZE * 2) + OBSTACLE_SIZE;
            
            // 确保不与玩家初始位置和传送门重叠
            bool tooClose = false;
            if (qAbs(x - GAME_WIDTH / 2) < 100 && qAbs(y - GAME_HEIGHT / 2) < 100) {
                tooClose = true;
            }
            
            // 检查是否与传送门重叠
            for (const auto &portal : portals) {
                if (QRectF(x - OBSTACLE_SIZE/2, y - OBSTACLE_SIZE/2, OBSTACLE_SIZE, OBSTACLE_SIZE).intersects(portal.area)) {
                    tooClose = true;
                    break;
                }
            }
            
            if (tooClose) {
                continue;
            }
            
            QGraphicsPixmapItem *obstacle = new QGraphicsPixmapItem;
            
            // 尝试加载障碍物图片
            QPixmap pixmap(OBSTACLE_IMAGE_PATH);
            
            // 如果图片加载失败，使用默认绘制的图形
            if (pixmap.isNull()) {
                pixmap = QPixmap(OBSTACLE_SIZE, OBSTACLE_SIZE);
                pixmap.fill(Qt::transparent);
                QPainter painter(&pixmap);
                painter.setBrush(OBSTACLE_COLOR);
                painter.drawRect(0, 0, OBSTACLE_SIZE, OBSTACLE_SIZE);
            }
            
            obstacle->setPixmap(pixmap);
            obstacle->setPos(x, y);
            scene->addItem(obstacle);
            
            // 存储障碍物以便后续使用
            //obstacles.push_back(obstacle);
        }
    }

    if(mapId==2){

    }
}

int Map::getMapId() const
{
    return mapId;
}

void Map::addTeleportPortal(int targetMapId, const QPointF &position, const QSizeF &size)
{
    Portal portal;
    portal.id = nextPortalId++;
    portal.targetMapId = targetMapId;
    portal.area = QRectF(position.x() - size.width()/2, position.y() - size.height()/2, size.width(), size.height());
    portals.append(portal);
}

void Map::handleTeleport(int portalId)
{
    // 这个方法将由SurvivorGame类调用，以实际处理地图切换
}

void Map::drawPortals()
{
    for (const auto &portal : portals) {
        QGraphicsPixmapItem *portalItem = new QGraphicsPixmapItem;
        QPixmap portalsImage = QPixmap(PORTALS_PATH);

        if(portalsImage.isNull()){
        // 绘制传送门（简单的紫色矩形）
        QPixmap pixmap(portal.area.width(), portal.area.height());
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        
        // 创建渐变效果使传送门看起来更炫酷
        QRadialGradient gradient(pixmap.rect().center(), pixmap.width()/2);
        gradient.setColorAt(0, TELEPORT_COLOR);
        gradient.setColorAt(1, Qt::transparent);
        
        painter.setBrush(gradient);
        painter.drawEllipse(pixmap.rect().adjusted(5, 5, -5, -5));
        
        // 添加边框
        painter.setPen(QPen(TELEPORT_COLOR, 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(pixmap.rect().adjusted(2, 2, -2, -2));
        
        portalItem->setPixmap(pixmap);
        portalItem->setPos(portal.area.topLeft());
        portalItem->setZValue(1); // 确保传送门在前景
        scene->addItem(portalItem);
        }else{
            portalsImage = portalsImage.scaled(portal.area.width(), portal.area.height(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
            portalItem->setPixmap(portalsImage);
            portalItem->setPos(portal.area.topLeft());
            portalItem->setZValue(1); // 确保传送门在前景
            scene->addItem(portalItem);
        }
    }
}
