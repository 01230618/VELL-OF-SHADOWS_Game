#ifndef LEVEL3_H
#define LEVEL3_H

#include <QWidget>
#include <QLabel>
#include <QList>
#include <QRect>
#include "StructDef.h"

class Level3
{
public:
    Level3();
    void init(QWidget *parent); // 初始化场景
    void clear();               // 清理场景

    QList<QRect>& getObstacles();
    QList<QRect>& getDeadZones();

    QList<Monster>& getMonsters();
    QList<FireBullet>& getfirebulletList();

    void loadMonsterPics();
    void ShootFireBullet(QWidget *parent,int p_x,int p_y);

private:
    QWidget *parentWidget;
    QLabel *levelBg;

    QList<QRect> obstacleList;
    QList<QRect> deadList;

    QList<Monster> monsters;
    QList<FireBullet> firebulletList;

    void initObstacles();
    void initTrapDead();
    void initMonsters(QWidget *parent);
};

#endif // LEVEL3_H
