#include "Level2.h"

Level2::Level2() {
    levelBg = nullptr;
}

void Level2::init(QWidget *parent)
{
    parentWidget = parent;

    // 创建背景
    levelBg = new QLabel(parentWidget);
    levelBg->setPixmap(QPixmap(":/images/level_2.png"));
    levelBg->setGeometry(0,0,960,540);
    levelBg->setScaledContents(1);
    levelBg->lower();
    levelBg->show();

    initObstacles();
    initMonsters(parentWidget);
}

void Level2::clear()
{
    if(levelBg){
        levelBg->hide();
        levelBg->deleteLater();
        levelBg = nullptr;
    }
    obstacleList.clear();
}

QList<QRect>& Level2::getObstacles()
{
    return obstacleList;
}

void Level2::initObstacles(){
    obstacleList.clear();

    //左侧帷幕
    obstacleList.append(QRect(0, 0, 192+19, 540));
    //左侧柱子
    obstacleList.append(QRect(192, 0, 76, 462-120));
    //最上方区域
    obstacleList.append(QRect(268, 0, 422, 231));
    //中间供台
    obstacleList.append(QRect(346, 0, 269, 350));
    //右侧帷幕
    obstacleList.append(QRect(691, 0, 269, 462-120));
    obstacleList.append(QRect(768, 462, 192, 100));
}

QList<Monster>& Level2::getMonsters()
{
    return monsters;
}

void Level2::loadMonsterPics(){
    Monster m1;

    QString basePath = ":/images/monster/level2";

    // 加载站立帧
    m1.picStand.load(basePath + "/stand.png");
    // 加载向上飞
    m1.monsterpicUp.clear();
    m1.monsterpicUp << QPixmap(basePath + "/up/1.png")
                    << QPixmap(basePath + "/up/2.png");

    // 加载向下飞
    m1.monsterpicDown.clear();
    m1.monsterpicDown << QPixmap(basePath + "/down/1.png")
                      << QPixmap(basePath + "/down/2.png");

    // 加载向左飞
    m1.monsterpicLeft.clear();
    m1.monsterpicLeft << QPixmap(basePath + "/left/1.png")
                      << QPixmap(basePath + "/left/2.png");

    // 加载向右飞
    m1.monsterpicRight.clear();
    m1.monsterpicRight << QPixmap(basePath + "/right/1.png")
                       << QPixmap(basePath + "/right/2.png");
    // 加载受伤动画
    m1.monsterPicHurtLeft.clear();
    m1.monsterPicHurtLeft << QPixmap(basePath + "/hurt/left1.png")
                          << QPixmap(basePath + "/hurt/left2.png");
    m1.monsterPicHurtRight.clear();
    m1.monsterPicHurtRight<< QPixmap(basePath + "/hurt/right1.png")
                           << QPixmap(basePath + "/hurt/right2.png");
    m1.monsterPicHurtFront.clear();
    m1.monsterPicHurtFront<< QPixmap(basePath + "/hurt/front1.png")
                           << QPixmap(basePath + "/hurt/front2.png");


    monsters.clear();
    monsters.append(m1);

    Monster m2=m1;
    monsters.append(m2);
}

void Level2::initMonsters(QWidget *parent){
    if (monsters.isEmpty()) return;
    parentWidget = parent;

    Monster &m1 = monsters[0];
    m1.monsterDir = 4;          // 初始方向:向右
    m1.monsterPicID = 0;
    m1.monster_x = 200;
    m1.monster_y = 200;
    m1.monsterSpeed = 3;
    m1.isHurt=false;
    m1.isDead=false;
    m1.monsterHurtID=0;
    m1.monsterMaxHP = 10;
    m1.monsterHP = m1.monsterMaxHP;
    m1.beHitOnce=false;

    iniMonsterHP(m1,parentWidget);

    // 创建怪物1-Label
    m1.label = new QLabel(parentWidget);
    m1.label->setScaledContents(1);
    m1.label->setFixedSize(50,65);
    m1.label->move(m1.monster_x, m1.monster_y);
    m1.label->raise();
    m1.label->show();

    Monster &m2 = monsters[1];
    m2.monsterDir = 3;          // 初始方向：3=向左
    m2.monsterPicID = 0;
    m2.monster_x = 700;
    m2.monster_y = 200;
    m2.monsterSpeed = 3;
    m2.isHurt=false;
    m2.isDead=false;
    m2.monsterHurtID=0;
    m2.monsterMaxHP = 10;
    m2.monsterHP = m2.monsterMaxHP;
    m2.beHitOnce=false;

    iniMonsterHP(m2,parentWidget);

    // 创建怪物2-Label
    m2.label = new QLabel(parentWidget);
    m2.label->setScaledContents(1);
    m2.label->setFixedSize(50,65);
    m2.label->move(m2.monster_x, m2.monster_y);
    m2.label->raise();
    m2.label->show();
}