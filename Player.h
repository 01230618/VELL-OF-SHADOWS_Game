#ifndef PLAYER_H
#define PLAYER_H
#include <QLabel>
#include <QTimer>
#include <QList>
#include <QPixmap>
#include <QImage>
#include <QPixmap>
#include <QKeyEvent>

struct SowrdEffect{
    QLabel *label;
    QList<QPixmap> sowrdPics;
    int curSowrdPic;
    bool alive;
    int x,y;
};
class Player
{
public:
    Player();

public:
    // 人物基础
    int curRole;           //当前角色序号（1，2，3，4）
    int playerPicID;       //人物行走动画帧序号
    int playerDir;         //人物方向
    bool ismoving;         //人物是否行走
    bool up;
    bool down;
    bool left;
    bool right;

    // 动画帧
    QList<QPixmap>playerPicup;
    QList<QPixmap>playerPicdown;
    QList<QPixmap>playerPicleft;
    QList<QPixmap>playerPicright;
    QPixmap playerPicStand;
    QList<QPixmap> playerFightLeft;
    QList<QPixmap> playerFightRight;
    QList<QPixmap> playerAttackedLeft;
    QList<QPixmap> playerAttackedRight;

    //动画逻辑
    int playermoveCount;
    const int maxmoveCount = 3;

    // 打斗状态
    bool isAttacking;       //人物是否正在攻击怪物（人打怪）
    int playerAttackPic;    //人物战斗动画帧序号
    double attackTime;      //一次按键攻击的时间
    int killcount;          //人物消灭怪物数量
    bool isAttacked;        //人物是否被怪物攻击（怪打人）
    bool isSuccessed;       //人物是否获得胜利
    int atkCd;             //冷却
    int hurtTime;          //受伤时间
    bool attackedBack;     //是否被攻击后退
    int BackDir;           //后退方向
    int Backlast;          //后退持续时间
    bool playAttackedAni;  //是否播放后退动画
    int playerBackedPic;    //人物后退动画帧序号

    // 血量
    int hp;        //人物当前血量
    int maxHP;     //人物最多血量

    // 防御
    bool isDefending;         //人物是否正在防御
    bool defenseCoolDown;     //防御是否处于冷却状态
    int defenseLastTime;      //一次防御持续时间
    int defenseCdTime;        //防御冷却时间

    // 隐身
    bool isPlayerStealth;     //人物是否正在隐身
    bool stealthCooldown;     //隐身是否处于冷却状态
    int stealthUseCount;      //隐身还能使用的次数
    int stealthLastTime;      //一次隐身持续时间
    int stealthCdTime;        //隐身冷却时间

    // 剑光特效
    QList<SowrdEffect>sowrdEffectList;
    QList<QPixmap>allSowrdEffectPics;

};

#endif // PLAYER_H
