#ifndef LEVEL2_H
#define LEVEL2_H

#include <QWidget>
#include <QLabel>
#include <QList>
#include <QRect>
#include "StructDef.h"

class Level2
{
public:
    Level2();
    void init(QWidget *parent); // 初始化场景
    void clear();               // 清理场景

    QList<QRect>& getObstacles();

    QList<Monster>& getMonsters();

    void loadMonsterPics();

private:
    QWidget *parentWidget;
    QLabel *levelBg;

    QList<QRect> obstacleList;
    QList<Monster> monsters;

    void initObstacles();
    void initMonsters(QWidget *parent);
};

#endif // LEVEL2_H
