#include "Level3.h"

Level3::Level3() {
    levelBg = nullptr;
}

void Level3::init(QWidget *parent)
{
    parentWidget = parent;

    // 创建背景
    levelBg = new QLabel(parentWidget);
    levelBg->setPixmap(QPixmap(":/images/level_3.png"));
    levelBg->setGeometry(0,0,960,540);
    levelBg->setScaledContents(1);
    levelBg->lower();
    levelBg->show();

    initObstacles();
    initTrapDead();
    initMonsters(parentWidget);
}

void Level3::clear()
{
    if(levelBg){
        levelBg->hide();
        levelBg->deleteLater();
        levelBg = nullptr;
    }
    obstacleList.clear();
    deadList.clear();
}

QList<QRect>& Level3::getObstacles()
{
    return obstacleList;
}

QList<QRect>& Level3::getDeadZones()
{
    return deadList;
}

void Level3::initObstacles(){
    obstacleList.clear();

    //左侧柱子
    obstacleList.append(QRect(0, 0, 117, 352));

    // 中间上方区域
    obstacleList.append(QRect(117,  0, 744, 196));

    //灯
    obstacleList.append(QRect(78, 352, 100,  156));
    obstacleList.append(QRect(290, 313-150, 45, 117));
    obstacleList.append(QRect(352, 274-150,50,  100));
    obstacleList.append(QRect(565, 274-120, 78,  78));
    obstacleList.append(QRect(626, 313-150, 78,  117));
    obstacleList.append(QRect(790, 313, 117,  156));

    // 右侧柱子
    obstacleList.append(QRect(862,  0,  98, 352));
}

void Level3::initTrapDead(){
    deadList.clear();

    // 左侧水池
    deadList.append(QRect(0, 352-145, 310, 39));
    deadList.append(QRect(0, 390-155, 170, 78));

    // 右侧水池
    deadList.append(QRect(627+50, 352-130, 333, 39));
    deadList.append(QRect(783+50, 390-155, 176, 78));
}

QList<Monster>& Level3::getMonsters()
{
    return monsters;
}

QList<FireBullet>& Level3::getfirebulletList()
{
    return firebulletList;
}

void Level3::loadMonsterPics(){
    Monster mBoss;
    QString basePath = ":/images/monster/level3";
    // 加载站立
    mBoss.picStand.load(basePath + "/boss_stand.png");

    Monster minion1;
    // 加载站立
    minion1.picStand.load(basePath + "/minion_stand.png");

    // 加载向上走
    minion1.monsterpicUp.clear();
    minion1.monsterpicUp << QPixmap(basePath + "/up/1.png")
                         << QPixmap(basePath + "/up/2.png");

    // 加载向下走
    minion1.monsterpicDown.clear();
    minion1.monsterpicDown << QPixmap(basePath + "/down/1.png")
                           << QPixmap(basePath + "/down/2.png");

    // 加载向左走
    minion1.monsterpicLeft.clear();
    minion1.monsterpicLeft << QPixmap(basePath + "/left/1.png")
                           << QPixmap(basePath + "/left/2.png");

    // 加载向右走
    minion1.monsterpicRight.clear();
    minion1.monsterpicRight << QPixmap(basePath + "/right/1.png")
                            << QPixmap(basePath + "/right/2.png");

    // 加载打斗动画
    minion1.monsterPicFightLeft.clear();
    minion1.monsterPicFightLeft << QPixmap(basePath + "/fight/left1.png")
                                << QPixmap(basePath + "/fight/left2.png")
                                << QPixmap(basePath + "/fight/left3.png");
    minion1.monsterPicFightRight.clear();
    minion1.monsterPicFightRight<< QPixmap(basePath + "/fight/right1.png")
                                 << QPixmap(basePath + "/fight/right2.png")
                                 << QPixmap(basePath + "/fight/right3.png");

    monsters.clear();
    monsters.append(mBoss);
    monsters.append(minion1);
    Monster minion2=minion1;
    monsters.append(minion2);
}

void Level3::initMonsters(QWidget *parent){
    if (monsters.isEmpty()) return;

    parentWidget = parent;

    Monster &m1 = monsters[0];
    m1.monsterDir = 0;
    m1.monsterPicID = 0;
    m1.monster_x = 390;
    m1.monster_y = 120;
    m1.isHurt=false;
    m1.isDead=false;
    m1.monsterMaxHP = 25;
    m1.monsterHP = m1.monsterMaxHP;
    m1.beHitOnce=false;

    iniMonsterHP(m1,parentWidget);

    // 创建怪物1-Label
    m1.label = new QLabel(parentWidget);
    m1.label->setScaledContents(1);
    m1.label->setFixedSize(200,200);
    m1.label->move(m1.monster_x, m1.monster_y);
    m1.label->raise();
    m1.label->show();

    Monster &m2 = monsters[1];
    m2.monsterDir = 3;          // 初始方向：3=向左
    m2.monsterPicID = 0;
    m2.monster_x = 800;
    m2.monster_y = 400;
    m2.monsterSpeed = 3;
    m2.isHurt=false;
    m2.isDead=false;
    m2.monsterFightID=0;
    m2.monsterMaxHP = 15;
    m2.monsterHP = m2.monsterMaxHP;
    m2.beHitOnce=false;
    m2.isAlive = false;
    m2.isFighting = false;

    iniMonsterHP(m2,parentWidget);
    m2.monsterHpbar->hide();
    m2.monsterHpBg->hide();

    // 创建怪物2-Label
    m2.label = new QLabel(parentWidget);
    m2.label->setScaledContents(1);
    m2.label->setFixedSize(70,75);
    m2.label->move(m2.monster_x, m2.monster_y);
    m2.label->raise();

    Monster &m3 = monsters[2];
    m3.monsterDir = 4;      //4=右
    m3.monsterPicID = 0;
    m3.monster_x = 100;
    m3.monster_y = 400;
    m3.monsterSpeed = 3;
    m3.isHurt=false;
    m3.isDead=false;
    m3.monsterFightID=0;
    m3.monsterMaxHP = 15;
    m3.monsterHP = m3.monsterMaxHP;
    m3.beHitOnce=false;
    m3.isAlive = false;
    m3.isFighting = false;

    iniMonsterHP(m3,parentWidget);
    m3.monsterHpbar->hide();
    m3.monsterHpBg->hide();

    // 创建怪物3-Label
    m3.label = new QLabel(parentWidget);
    m3.label->setScaledContents(1);
    m3.label->setFixedSize(70,75);
    m3.label->move(m3.monster_x, m3.monster_y);
    m3.label->raise();
}

void Level3::ShootFireBullet(QWidget *parent,int p_x,int p_y){
    Monster &boss = monsters[0];
    if(!boss.isDead)
    {
        int dx = p_x - boss.monster_x;
        int dy = p_y - boss.monster_y;
        double dist = sqrt(dx * dx + dy * dy);

        // 玩家越近，发射越频繁
        int shootInterval = (dist < 200) ? 2 : 3;

        static int shootTimer = 0;
        shootTimer++;
        if(shootTimer >= shootInterval)
        {
            shootTimer = 0;

            double angle = atan2(dy, dx);

            //初始化
            FireBullet fb;
            fb.x = boss.monster_x + 20;
            fb.y = boss.monster_y + 20;
            fb.angle = angle;
            fb.speed = 3;
            fb.lifetime = 120;

            fb.label = new QLabel(parent);
            fb.label->setGeometry(fb.x, fb.y, 30, 30);
            fb.label->setPixmap(QPixmap(":/images/effect/firebullet.png"));
            fb.label->setScaledContents(true);
            fb.label->show();
            fb.label->raise();

            firebulletList.append(fb);
        }
    }
}