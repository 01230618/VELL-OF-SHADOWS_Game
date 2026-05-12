#ifndef STRUCTDEF_H
#define STRUCTDEF_H
#include <QLabel>
#include <QPixmap>
#include <QList>
#include <QRect>

struct Monster {
    QLabel *label;                 // 怪物标签
    int monsterDir;                // 方向：0=站立，1=下，2=上，3=左，4=右
    int monsterPicID;              // 怪物行走动画帧序号
    int monster_x, monster_y;      // 怪物坐标
    int monsterSpeed;              // 移动速度
    QList<QPixmap> monsterpicDown; // 向下走帧
    QList<QPixmap> monsterpicUp;   // 向上走帧
    QList<QPixmap> monsterpicLeft; // 向左走帧
    QList<QPixmap> monsterpicRight;// 向右走帧
    QPixmap picStand;              // 站立帧
    bool isHurt;            // 怪物是否受伤害
    bool isDead;            // 怪物是否死亡
    int monsterHurtID;      // 怪物受伤害动画帧序号
    QList<QPixmap> monsterPicHurtLeft;   // 向左受伤害
    QList<QPixmap> monsterPicHurtRight;  // 向右受伤害
    QList<QPixmap> monsterPicHurtFront;  // 向前受伤害
    int monsterHP;            // 怪物实时血量
    int monsterMaxHP;         // 怪物最大血量
    QLabel* monsterHpbar;     // 怪物血条
    QLabel* monsterHpBg;      //血条背景
    bool beHitOnce;           // 怪物是否被攻击一次（防止timer时间内多次被攻击）
    bool isAlive;             // 场景三使用——怪物是否现形
    bool isFighting;          // 场景三使用——怪物是否正在打斗
    int monsterFightID;       // 场景三——怪物打斗动画帧序号
    QList<QPixmap> monsterPicFightLeft;      // 场景三——怪物向左打斗
    QList<QPixmap> monsterPicFightRight;     // 场景三——怪物向右打斗
};

//场景一随机区域加载
struct Level1Rand{
    QLabel*label;   //区域图片
    int x;
    int y;
    int width;
    int height;
    bool isAlive;   //区域是否被系统随机生成
    int lifetime;   //区域单次停留时间
};

//场景一怪物攻击
struct MonsterBullet{
    QLabel*label;   //攻击特效图片
    int x;
    int y;
    double angle;   //攻击角度
    int speed;      //攻击速度
    int lifetime;   //单个特效存在时间
};


//场景三火焰攻击
struct FireBullet {
    QLabel*label;   //攻击特效图片
    int x;
    int y;
    double angle;   //攻击角度
    int speed;      //攻击速度
    int lifetime;   //单个特效存在时间
};

void updateMonsterAnim(Monster &monster,int CurLevel);
void iniMonsterHP(Monster &m,QWidget *parent);

#endif // STRUCTDEF_H
