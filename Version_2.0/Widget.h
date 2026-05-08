#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QLabel>
#include <Roleselect.h>
#include <QTimer>
#include <QImage>
#include <QList>
#include <QPixmap>
#include <QKeyEvent>
#include "Player.h"
#include "Intro.h"
#include <QMediaPlayer>
#include <QAudioOutput>
#include "Rule.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

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

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

protected:

    //键盘按下事件
    void keyPressEvent(QKeyEvent *event) override;

    //键盘抬起事件
    void keyReleaseEvent(QKeyEvent *event) override;

//槽函数
private slots:

    void on_start_btn_clicked();//“开始游戏”

    void on_role_btn_clicked();//“选择角色”

    void on_level_1_clicked();//“场景一”

    void on_level_2_clicked();//“场景二”

    void on_level_3_clicked();//“场景三”

    void on_back_clicked();//“返回”

    void moveMonster();//与怪物定时器连接

    void movePlayer();//与玩家定时器连接

    void onstartGame();//与介绍场景页面的开始游戏信号绑定，表示开始游戏

    void onHPBlinking();//与人物血条闪烁定时器连接

private:

    //玩家相关
    Player pla;                  //玩家类
    QLabel *player;              //玩家label
    QTimer *playermoveTimer;     //玩家定时器
    QLabel *playerHPBar;         //玩家血条
    QLabel *playerHPBg;          //玩家血条背景
    QLabel *playerHPText;        //玩家血条数字
    QTimer *hpBlinkTimer;        //玩家血条定时器
    bool isHPblinding = false;   //血条是否闪烁
    QLabel* defenseLabel;        //玩家防御特效
    void PlayerPicload();        //加载玩家动画
    void iniPlayerHPBar();       //初始化玩家血条
    void updatePlayerHPBar();    //更新玩家血条
    void createSowrdEffect();    //创建剑光特效
    void updateAllSowrdEffect(); //更新剑光特效

    //怪物相关
    int curLevel;                   // 当前场景：1=场景1，2=场景2，3=场景3，4=场景4
    QList<Monster> level1Monsters;  // 场景1怪物列表（1个怪物）
    QList<Monster> level2Monsters;  // 场景2怪物列表（2个怪物）
    QList<Monster> level3Monsters;  // 场景3怪物列表（3个怪物）
    QTimer *monstermoveTimer;       // 怪物定时器
    void iniMonsterHP(Monster &m);  //初始化怪物血条
    void updateMonsterHP(Monster &m);  //更新怪物血条
    // 怪物资源加载 & 怪物初始化
    void loadLevel1MonsterPics();   // 加载场景1怪物资源（1个）
    void loadLevel2MonsterPics();   // 加载场景2怪物资源（2个）
    void loadLevel3MonsterPics();   // 加载场景3怪物资源（3个）
    void initLevel1Monsters();      // 初始化场景1怪物
    void initLevel2Monsters();      // 初始化场景2怪物
    void initLevel3Monsters();      // 初始化场景3怪物
    void clearCurrentMonsters();    // 清空当前场景怪物
    //怪物特效
    QList<MonsterBullet> bulletlist;  // 场景一特效
    QList<FireBullet> fireBullets;    // 场景三特效
    QTimer * MonsterBulletTimer;      // 场景一怪物特效定时器
    QTimer * MonsterFireBulletTimer;  // 场景三怪物特效定时器
    void MonsterShootCircleBullet();  //connect with 定时器
    void moveMonsterBullets();        //放于主定时器中
    void moveMonsterFireBullets();    //放于主定时器中
    void MonsterShootFireBullet();    //connect with 定时器

    //障碍碰撞等相关
    QList<QRect>m_obstaclelist;    //障碍物列表
    QList<QRect>m_traplist;        //陷阱列表
    QList<QRect>m_deadlist;        //死亡区域列表
    void iniLevel1Obstacles();     //初始化场景一障碍
    void iniLevel1TrapDead();      //初始化场景一陷阱和死亡区域
    void iniLevel2Obstacles();     //初始化场景二障碍
    void iniLevel3Obstacles();     //初始化场景三障碍
    void iniLevel3TrapDead();      //初始化场景三死亡区域

    //场景相关
    //场景图片
    QLabel* levelBg;
    //场景一随机生成 陷阱/安全区域
    Level1Rand trap;
    Level1Rand safeZone;
    //介绍页面
    Intro * intro;
    bool beginGame;
    //规则页面
    Rule * rule;
    //返回主界面
    void backHome();

    //通用
    void clearAll();

    //音乐
    QMediaPlayer *levelBGM;
    QAudioOutput *levelOutput;
    QMediaPlayer *sowrdBGM;
    QAudioOutput *sowrdOutput;
    QMediaPlayer *winBGM;
    QAudioOutput *winOutput;
    void playHomeBGM();
    void playLevel1BGM();
    void playLevel2BGM();
    void playLevel3BGM();

    Ui::Widget *ui;
};
#endif // WIDGET_H
