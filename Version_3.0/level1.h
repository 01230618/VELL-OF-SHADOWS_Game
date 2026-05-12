#ifndef LEVEL1_H
#define LEVEL1_H

#include <QWidget>
#include <QLabel>
#include <QList>
#include <QRect>
#include <QPixmap>
#include "StructDef.h"

class Level1
{
public:
    Level1();
    void init(QWidget *parent); // 初始化场景
    void clear();               // 清理场景

    QList<QRect>& getObstacles();
    QList<QRect>& getTraps();
    QList<QRect>& getDeadZones();
    QList<Level1Rand>& getRandTraps();
    QList<Level1Rand>& getRandsafeZones();

    QList<Monster>& getMonsters();
    QList<MonsterBullet>& getbulletList();

    void loadMonsterPics();
    void ShootCircleBullet(QWidget *parent);

private:
    QWidget *parentWidget;
    QLabel *levelBg;

    QList<QRect> obstacleList;
    QList<QRect> trapList;
    QList<QRect> deadList;
    QList<Level1Rand> trap;
    QList<Level1Rand> safeZone;

    QList<Monster> monsters;
    QList<MonsterBullet> bulletList;  // 场景一特效

    void initObstacles();
    void initTrapDead();
    void initMonsters(QWidget *parent);
    void initRandomArea();
};

#endif // LEVEL1_H
