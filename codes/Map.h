#ifndef MAP_H
#define MAP_H

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

class Map : public QGraphicsPixmapItem
{
public:
    Map(int mapId, QGraphicsScene *scene, QGraphicsItem *parent = nullptr);
    ~Map();
    
    int getMapId() const;    
    void addTeleportPortal(int targetMapId, const QPointF &position, const QSizeF &size);
    void handleTeleport(int portalId);
    
private:
    int mapId;
    QGraphicsScene *scene;
    
    struct Portal {
        int id;
        int targetMapId;
        QRectF area;
    };
    
    QList<Portal> portals;
    int nextPortalId;
    
    void loadMap();
    void drawBackground();
    void generateObstacles();
    void drawPortals();
};

#endif // MAP_H