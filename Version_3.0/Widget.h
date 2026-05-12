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
#include "Level1.h"
#include "StructDef.h"
#include "Level2.h"
#include "Level3.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

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
    QTimer *monstermoveTimer;       // 怪物定时器
    void updateMonsterHP(Monster &m);  //更新怪物血条
    // 怪物清理
    void clearCurrentMonsters();    // 清空当前场景怪物
    //怪物特效
    QTimer * MonsterBulletTimer;      // 场景一怪物特效定时器
    QTimer * MonsterFireBulletTimer;  // 场景三怪物特效定时器
    void MonsterShootCircleBullet();  //connect with 定时器
    void moveMonsterBullets();        //放于主定时器中
    void moveMonsterFireBullets();    //放于主定时器中
    void MonsterShootFireBullet();    //connect with 定时器

    //场景相关
    Level1 level1;
    Level2 level2;
    Level3 level3;
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
