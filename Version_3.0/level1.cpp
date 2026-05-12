#include "Level1.h"

Level1::Level1() {
    levelBg = nullptr;
}

void Level1::init(QWidget *parent)
{
    parentWidget = parent;

    // 创建背景
    levelBg = new QLabel(parentWidget);
    levelBg->setPixmap(QPixmap(":/images/level_1.png"));
    levelBg->setGeometry(0, 0, 960, 540);
    levelBg->setScaledContents(true);
    levelBg->lower(); // 放到底层
    levelBg->show();

    initObstacles();
    initTrapDead();
    initRandomArea();
    initMonsters(parentWidget);
}

void Level1::clear()
{
    if(levelBg){
        levelBg->hide();
        levelBg->deleteLater();
        levelBg = nullptr;
    }
    obstacleList.clear();
    trapList.clear();
    deadList.clear();
}

QList<QRect>& Level1::getObstacles()
{
    return obstacleList;
}

QList<QRect>& Level1::getTraps()
{
    return trapList;
}

QList<QRect>& Level1::getDeadZones()
{
    return deadList;
}

QList<Level1Rand>& Level1::getRandTraps()
{
    return trap;
}

QList<Level1Rand>& Level1::getRandsafeZones()
{
    return safeZone;
}

QList<Monster>& Level1::getMonsters()
{
    return monsters;
}

QList<MonsterBullet>& Level1:: getbulletList()
{
    return bulletList;
}

void Level1::initObstacles(){
    obstacleList.clear();

    // 左侧截断
    obstacleList.append(QRect(0, 0, 230, 540));
    // 左侧上方红帘
    obstacleList.append(QRect(230, 0, 153-50, 192-150));
    // 中间红帘
    obstacleList.append(QRect(480, 0, 38, 347-150));
    // 中间立柱
    obstacleList.append(QRect(537, 192, 19, 289-150));
    //右上方高台
    obstacleList.append(QRect(518, 0, 441, 192));
    // 右侧红帘
    obstacleList.append(QRect(691+100, 193, 267, 193-38));
    // 右方底部
    obstacleList.append(QRect(655, 385-150, 307, 77));
}

void Level1::initTrapDead(){
    trapList.clear();
    deadList.clear();

    //中间木架陷阱（踩入切换场景二）
    trapList.append(QRect(231, 231-150, 150, 115));

    //左下角蓝色水坑（踩入死亡重启）
    deadList.append(QRect(192, 385-150, 126-20, 115));
    deadList.append(QRect(153, 501, 100, 38));
}

void Level1::ShootCircleBullet(QWidget *parent){
    auto &m = monsters[0];//场景一就一个怪物

    //一次显示十二枚子弹
    int bulletCount = 12;
    for(int i = 0; i < bulletCount; i++)
    {
        // 计算每一发子弹角度
        double Pi=3.14;
        double ang = i * 30.0 * Pi / 180.0;

        MonsterBullet b;
        //每次调用都初始化子弹
        b.x = m.monster_x + 20;
        b.y = m.monster_y + 20;
        b.angle = ang;
        b.speed = 3;
        b.lifetime = 80;

        // 创建子弹标签
        b.label = new QLabel(parent);
        b.label->setGeometry(b.x, b.y, 12, 12);
        b.label->setPixmap(QPixmap(":/images/monster/level1/effect.png"));
        b.label->setScaledContents(true);
        b.label->show();
        b.label->raise();
        bulletList.append(b);
    }
}

void Level1::initRandomArea(){

    // 场景一陷阱初始化
    Level1Rand randtrap;
    randtrap.label = nullptr;
    randtrap.x = 0;
    randtrap.y = 0;
    randtrap.width = 60;
    randtrap.height = 180;
    randtrap.isAlive = false;
    randtrap.lifetime = 0;
    trap.append(randtrap);

    // 场景一安全区初始化
    Level1Rand randsafeZone;
    randsafeZone.label = nullptr;
    randsafeZone.x = 0;
    randsafeZone.y = 0;
    randsafeZone.width = 100;
    randsafeZone.height = 180;
    randsafeZone.isAlive = false;
    randsafeZone.lifetime = 0;
    safeZone.append(randsafeZone);
}

void Level1::initMonsters(QWidget *parent){
    if(monsters.isEmpty()) return;

    parentWidget = parent;

    Monster &m = monsters[0];
    m.monsterDir = 1;          // 初始向下
    m.monsterPicID = 0;
    m.monster_x = 400;
    m.monster_y = 200;
    m.monsterSpeed = 2;

    // 创建怪物
    m.label = new QLabel(parentWidget);
    m.label->setScaledContents(1);
    m.label->setFixedSize(60, 60);
    m.label->move(m.monster_x, m.monster_y);
    m.label->raise();
    m.label->show();
}

void Level1::loadMonsterPics(){
    Monster m;

    QString basePath = ":/images/monster/level1";

    // 加载站立帧
    m.picStand.load(basePath + "/stand.png");
    // 加载向下走帧
    m.monsterpicDown.clear();
    m.monsterpicDown << QPixmap(basePath + "/down/1.png")
                     << QPixmap(basePath + "/down/2.png")
                     << QPixmap(basePath + "/down/3.png");
    // 加载向上走帧
    m.monsterpicUp.clear();
    m.monsterpicUp << QPixmap(basePath + "/up/1.png")
                   << QPixmap(basePath + "/up/2.png")
                   << QPixmap(basePath + "/up/3.png");

    monsters.clear();
    monsters.append(m);
}