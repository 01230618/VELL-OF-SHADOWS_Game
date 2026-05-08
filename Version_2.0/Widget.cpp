#include "Widget.h"
#include "ui_widget.h"
#include <QDialog>
#include <QDebug>
#include <QKeyEvent>
#include <QMessageBox>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setFixedSize(960,540);
    this->setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);//禁止最大最小化

    //游戏主界面背景
    ui->mainBg->setPixmap(QPixmap(":/images/mainBg.png"));
    ui->mainBg->setGeometry(0,0,this->width(),this->height());
    ui->mainBg->setScaledContents(1);
    ui->mainBg->lower();
    ui->mainBg->show();

    //游戏按钮
    ui->level_1->hide();
    ui->level_2->hide();
    ui->level_3->hide();
    ui->role_btn->hide();
    ui->back->hide();

    //玩家初始化
    PlayerPicload();//加载角色动作帧
    playermoveTimer=new QTimer(this);
    connect(playermoveTimer,&QTimer::timeout,this,&Widget::movePlayer);//连接qtimer超时信号和槽函数：玩家移动
    //玩家防御
    defenseLabel = nullptr;
    //玩家血条
    playerHPBg = nullptr;
    playerHPBar = nullptr;
    playerHPText = nullptr;
    hpBlinkTimer = nullptr;
    isHPblinding = false;

    //怪物初始化
    curLevel=0;
    monstermoveTimer = new QTimer(this);
    connect(monstermoveTimer,&QTimer::timeout,this,&Widget::moveMonster);
    //连接qtimer超时信号和槽函数：怪物移动

    //特效初始化
    //人物武器：剑
    pla.allSowrdEffectPics.clear();
    pla.allSowrdEffectPics << QPixmap(":/images/effect/s1.png")
                       << QPixmap(":/images/effect/s2.png")
                       << QPixmap(":/images/effect/s3.png");
    //怪物
    //场景一怪物
    MonsterBulletTimer = new QTimer(this);
    connect(MonsterBulletTimer, &QTimer::timeout, this, &Widget::MonsterShootCircleBullet);
    //场景三怪物
    MonsterFireBulletTimer = new QTimer(this);
    connect(MonsterFireBulletTimer, &QTimer::timeout, this, &Widget::MonsterShootFireBullet);

    //场景
    levelBg = nullptr;
    //规则介绍页面
    rule = new Rule(this);
    rule->hide();
    //场景介绍页面
    beginGame=false;
    intro = new Intro(this);
    intro->hide();
    connect(intro,&Intro::startGame,this,&Widget::onstartGame);
    // 场景一陷阱初始化
    trap.label = nullptr;
    trap.x = 0;
    trap.y = 0;
    trap.width = 60;
    trap.height = 180;
    trap.isAlive = false;
    trap.lifetime = 0;
    // 场景一安全区初始化
    safeZone.label = nullptr;
    safeZone.x = 0;
    safeZone.y = 0;
    safeZone.width = 100;
    safeZone.height = 180;
    safeZone.isAlive = false;
    safeZone.lifetime = 0;

    //音乐
    levelBGM = new QMediaPlayer(this);
    levelOutput = new QAudioOutput(this);
    levelBGM->setAudioOutput(levelOutput);
    levelBGM->setLoops(QMediaPlayer::Infinite);
    levelOutput->setVolume(0.4);
    playHomeBGM();
    sowrdBGM = new QMediaPlayer(this);
    sowrdOutput = new QAudioOutput(this);
    sowrdBGM->setAudioOutput(sowrdOutput);
    sowrdOutput->setVolume(0.8);
    sowrdBGM->setSource(QUrl("qrc:/music/playerFight.mp3"));
    winBGM = new QMediaPlayer(this);
    winOutput = new QAudioOutput(this);
    winBGM->setAudioOutput(winOutput);
    winOutput->setVolume(0.3);
    winBGM->setSource(QUrl("qrc:/music/winBGM.ogg"));
}

//析构函数
Widget::~Widget()
{
    delete ui;
}

//场景一设置
void Widget::on_level_1_clicked()
{
    //怪物相关
    clearCurrentMonsters();
    loadLevel1MonsterPics();
    initLevel1Monsters();
    curLevel = 1;
    monstermoveTimer->start(70);
    //特效设置
    if(MonsterBulletTimer){
        MonsterBulletTimer->start(1500);
    }
    if(MonsterFireBulletTimer){
        MonsterFireBulletTimer->stop();
    }

    //按钮相关
    ui->level_1->hide();
    ui->level_2->hide();
    ui->level_3->hide();
    ui->role_btn->hide();
    ui->start_btn->hide();
    ui->back->show();
    ui->mainBg->hide();

    if(levelBg){
        levelBg->hide();
        levelBg->deleteLater();
        levelBg = nullptr;
    }
    // 创建背景
    levelBg = new QLabel(this);
    levelBg->setPixmap(QPixmap(":/images/level_1.png"));
    levelBg->setGeometry(0, 0, 960, 540);
    levelBg->setScaledContents(true);
    levelBg->lower(); // 放到底层
    levelBg->show();

    //障碍物相关
    iniLevel1Obstacles();
    iniLevel1TrapDead();

    //玩家动画设置
    PlayerPicload();
    pla.playerDir=0;
    pla.playerPicID=0;
    pla.ismoving = false;
    pla.up=false;
    pla.down=false;
    pla.left=false;
    pla.right=false;

    //玩家属性设置
    player=new QLabel(this);
    player->setObjectName("player");
    player->move(450,300);
    player->show();
    player->raise();
    playermoveTimer->start(60);

    //战斗属性设置
    iniPlayerHPBar();
    pla.isSuccessed=false;
    pla.isAttacking=false;
    pla.isAttacked=false;
    pla.playerAttackPic=0;
    pla.maxHP=100;
    pla.hp=pla.maxHP;
    pla.attackTime=0;
    pla.killcount=0;
    pla.attackedBack = false;
    pla.Backlast = 0;
    pla.playAttackedAni = false;
    pla.playerBackedPic = 0;

    //介绍页面
    beginGame=false;
    if(!beginGame){
        intro->setIntroText("欢迎来到场景一！"
                            "\n "
                            "\n红帘深垂，灯影摇曳，你踏入了这片诡谲的古楼回廊。"
                            "\n "
                            "\n此刻的你毫无攻防之力，廊间游荡的怪物是你唯一的威胁。"
                            "\n "
                            "\n未知威胁随机生成。这片空间里，危险与庇护随机共生，没有固定的安全角落。"
                            "\n"
                            "\n保持警惕，躲避每一次危机，找到出口，逃离这片迷楼。");
        intro->raise();
        intro->show();
    }
    ui->role_btn->setEnabled(false);

    //播放场景一音乐
    playLevel1BGM();
}

//场景一障碍物初始化
void Widget::iniLevel1Obstacles(){

    m_obstaclelist.clear();

    // 左侧截断
    m_obstaclelist.append(QRect(0, 0, 230, 540));
    // 左侧上方红帘
    m_obstaclelist.append(QRect(230, 0, 153-50, 192-150));
    // 中间红帘
    m_obstaclelist.append(QRect(480, 0, 38, 347-150));
    // 中间立柱
    m_obstaclelist.append(QRect(537, 192, 19, 289-150));
    //右上方高台
    m_obstaclelist.append(QRect(518, 0, 441, 192));
    // 右侧红帘
    m_obstaclelist.append(QRect(691+100, 193, 267, 193-38));
    // 右方底部
    m_obstaclelist.append(QRect(655, 385-150, 307, 77));

}

//场景一陷阱和死亡区域初始化
void Widget::iniLevel1TrapDead(){

    m_traplist.clear();
    m_deadlist.clear();

    //中间木架陷阱（踩入切换场景二）
    m_traplist.append(QRect(231, 231-150, 150, 115));

    //左下角蓝色水坑（踩入死亡重启）
    m_deadlist.append(QRect(192, 385-150, 126-20, 115));
    m_deadlist.append(QRect(153, 501, 100, 38));
}

//场景一怪物特效，主定时器超时调用
void Widget::moveMonsterBullets()
{
    if(curLevel != 1) return;
    if(!player) return;
    if(pla.hp<=0) return;

    // 遍历子弹移动
    for(int i = bulletlist.size() - 1; i >= 0; i--)
    {
        MonsterBullet &b = bulletlist[i];

        if(!b.label){
            bulletlist.removeAt(i);
            continue;
        }

        b.x += cos(b.angle) * b.speed;
        b.y += sin(b.angle) * b.speed;
        b.label->move(b.x, b.y);

        b.lifetime--;

        if(b.lifetime <= 0)
        {
            b.label->hide();
            delete b.label;
            b.label = nullptr;
            bulletlist.removeAt(i);
            continue;
        }

        //子弹碰到玩家：扣血
        QRect bulletRect(b.x, b.y, 12, 12);
        QRect playerRect(player->x(), player->y(), player->width(), player->height());

        if(bulletRect.intersects(playerRect))
        {
            bool insafe = false;
            if (safeZone.isAlive && safeZone.label != nullptr)
            {
                if (player->geometry().intersects(safeZone.label->geometry()))
                {
                    insafe = true;
                }
            }

            if(insafe){
                if(b.label){
                    b.label->hide();
                    delete b.label;
                    b.label = nullptr;
                    bulletlist.removeAt(i);
                }
                continue;
            }

            pla.hp -= 5;
            if(pla.hp < 0) pla.hp = 0;
            updatePlayerHPBar();

            // 命中后清理子弹
            b.label->hide();
            delete b.label;
            b.label = nullptr;
            bulletlist.removeAt(i);
            continue;

            // 玩家死亡判断
            if(pla.hp <= 0)
            {
                qDebug() << "你死了！";
                QMessageBox::information(this, "提示", "你被怪物击败，挑战失败！");
                pla.ismoving = false;
                clearAll();
                backHome();
                return;
            }
        }
    }
}

//场景一怪物特效槽函数，特效定时器超时调用
void Widget::MonsterShootCircleBullet()
{
    if(!beginGame) return;//不然在介绍页面就会有图片显示
    if(curLevel != 1) return;

    auto &m =level1Monsters[0];//场景一就一个怪物

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
        b.label = new QLabel(this);
        b.label->setGeometry(b.x, b.y, 12, 12);
        b.label->setPixmap(QPixmap(":/images/monster/level1/effect.png"));
        b.label->setScaledContents(true);
        b.label->show();
        b.label->raise();
        bulletlist.append(b);
    }
}

//场景二设置
void Widget::on_level_2_clicked()
{
    //按钮设置
    ui->level_1->hide();
    ui->level_2->hide();
    ui->level_3->hide();
    ui->role_btn->hide();
    ui->start_btn->hide();
    ui->back->show();
    ui->mainBg->hide();

    //场景二背景设置
    if(levelBg){
        levelBg->hide();
        delete levelBg;
        levelBg = nullptr;
    }
    levelBg = new QLabel(this);
    levelBg->setPixmap(QPixmap(":/images/level_2.png"));
    levelBg->setGeometry(0,0,960,540);
    levelBg->setScaledContents(1);
    levelBg->lower();
    levelBg->show();

    //场景二人物设置
    //玩家动画设置
    PlayerPicload();
    pla.playerDir=0;
    pla.playerPicID=0;
    pla.ismoving = false;
    pla.up=false;
    pla.down=false;
    pla.left=false;
    pla.right=false;

    //玩家属性设置
    player=new QLabel(this);
    player->setObjectName("player");
    player->move(480,360);
    player->show();
    player->raise();
    playermoveTimer->start(50);

    //场景二怪物设置
    clearCurrentMonsters();
    loadLevel2MonsterPics();
    initLevel2Monsters();
    curLevel = 2;
    if (monstermoveTimer) {
        monstermoveTimer->start(70);
    }
    //怪物特效设置
    if(MonsterBulletTimer){
        MonsterBulletTimer->stop();
    }
    if(MonsterFireBulletTimer){
        MonsterFireBulletTimer->stop();
    }

    //战斗属性设置
    pla.isSuccessed=false;
    pla.isAttacking=false;
    pla.isAttacked=false;
    pla.playerAttackPic=0;
    pla.maxHP=100;
    pla.hp=pla.maxHP;
    pla.attackTime=0;
    pla.killcount=0;
    iniPlayerHPBar();
    pla.hurtTime = 0;
    pla.attackedBack = false;
    pla.Backlast = 0;
    pla.playAttackedAni = false;
    pla.playerBackedPic = 0;

    //障碍物设置
    iniLevel2Obstacles();

    //介绍页面设置
    beginGame=false;
    if(!beginGame){
        intro->setIntroText("欢迎来到场景二！\n"
                            "\n"
                            "\n踏入全新险境，你已然觉醒攻防之力。"
                            "\n"
                            "\n此地魔物极具灵智，会全程锁定、实时追踪你的行踪。"
                            "\n"
                            "\n不再只能被动躲避，挥动战力肃清所有追兵"
                            "\n"
                            "\n剿灭全部怪物，即可通关前行。");
        intro->raise();
        intro->show();
    }

    ui->role_btn->setEnabled(false);
    //播放场景二音乐
    playLevel2BGM();
}

void Widget::iniLevel2Obstacles()
{
    // 清空上一关的障碍物
    m_obstaclelist.clear();

    //左侧帷幕
    m_obstaclelist.append(QRect(0, 0, 192+19, 540));
    //左侧柱子
    m_obstaclelist.append(QRect(192, 0, 76, 462-120));
    //最上方区域
    m_obstaclelist.append(QRect(268, 0, 422, 231));
    //中间供台
    m_obstaclelist.append(QRect(346, 0, 269, 350));
    //右侧帷幕
    m_obstaclelist.append(QRect(691, 0, 269, 462-120));
    m_obstaclelist.append(QRect(768, 462, 192, 100));

}

//场景三设置
void Widget::on_level_3_clicked()
{
    //场景三怪物设置
    clearCurrentMonsters();
    loadLevel3MonsterPics();
    initLevel3Monsters();
    curLevel = 3;
    if (monstermoveTimer) {
        monstermoveTimer->start(70);
    }
    //怪物特效
    if(MonsterBulletTimer){
        MonsterBulletTimer->stop();
    }
    if(MonsterFireBulletTimer){
        MonsterFireBulletTimer->start(1000);
    }

    //按钮设置
    ui->level_1->hide();
    ui->level_2->hide();
    ui->level_3->hide();
    ui->role_btn->hide();
    ui->start_btn->hide();
    ui->back->show();
    ui->mainBg->hide();

    //场景三背景设置
    if(levelBg){
        levelBg->hide();
        delete levelBg;
        levelBg = nullptr;
    }
    levelBg = new QLabel(this);
    levelBg->setPixmap(QPixmap(":/images/level_3.png"));
    levelBg->setGeometry(0,0,960,540);
    levelBg->setScaledContents(1);
    levelBg->lower();
    levelBg->show();

    //场景三人物设置
    //玩家动画设置
    PlayerPicload();
    pla.playerDir=0;
    pla.playerPicID=0;
    pla.ismoving = false;
    pla.up=false;
    pla.down=false;
    pla.left=false;
    pla.right=false;

    //玩家属性设置
    player=new QLabel(this);
    player->setObjectName("player");
    player->move(480,320);
    player->show();
    player->raise();
    playermoveTimer->start(50);

    //战斗属性设置
    pla.isSuccessed=false;
    pla.isAttacking=false;
    pla.isAttacked=false;
    pla.attackTime = 0;
    pla.playerAttackPic=0;
    pla.maxHP=100;
    pla.hp=pla.maxHP;
    pla.attackTime=0;
    pla.killcount=0;
    iniPlayerHPBar();
    pla.hurtTime = 0;
    pla.attackedBack = false;
    pla.Backlast = 0;
    pla.playAttackedAni = false;
    pla.playerBackedPic = 0;

    //障碍物设置
    iniLevel3Obstacles();
    iniLevel3TrapDead();

    //介绍页面设置
    beginGame=false;
    if(!beginGame){
        intro->setIntroText("欢迎来到场景三！\n"
                            "\n"
                            "\n暗影深域降临，你已集齐攻击、防御、隐身三重能力。"
                            "\n"
                            "\n烈焰魔物的火攻会死死追踪你的踪迹，危机步步紧逼。"
                            "\n"
                            "\n暗处更潜藏着无形魅影，靠近才会骤然显现。"
                            "\n"
                            "\n善用隐身匿踪，规避火焰追击，肃清明处与暗处所有怪物，方可冲破此域。");
        intro->raise();
        intro->show();
    }

    ui->role_btn->setEnabled(false);
    //音乐设置
    winBGM->stop();
    playLevel3BGM();
}

//场景三障碍物初始化
void Widget::iniLevel3Obstacles(){

    m_obstaclelist.clear();

    //左侧柱子
    m_obstaclelist.append(QRect(0, 0, 117, 352));

    // 中间上方区域
    m_obstaclelist.append(QRect(117,  0, 744, 196));

    //灯
    m_obstaclelist.append(QRect(78, 352, 100,  156));
    m_obstaclelist.append(QRect(290, 313-150, 45, 117));
    m_obstaclelist.append(QRect(352, 274-150,50,  100));
    m_obstaclelist.append(QRect(565, 274-120, 78,  78));
    m_obstaclelist.append(QRect(626, 313-150, 78,  117));
    m_obstaclelist.append(QRect(790, 313, 117,  156));

    // 右侧柱子
    m_obstaclelist.append(QRect(862,  0,  98, 352));

}

//场景三死亡区域初始化
void Widget::iniLevel3TrapDead(){
    m_deadlist.clear();

    // 左侧水池
    m_deadlist.append(QRect(0, 352-145, 310, 39));
    m_deadlist.append(QRect(0, 390-155, 170, 78));

    // 右侧水池
    m_deadlist.append(QRect(627+50, 352-130, 333, 39));
    m_deadlist.append(QRect(783+50, 390-155, 176, 78));
}

//场景三怪物特效，主定时器超时调用
void Widget::moveMonsterFireBullets(){

    if(curLevel != 3) return;
    if(!player) return;
    if(pla.hp<=0) return;

    QRect playerRect(player->x(), player->y(),player->width(), player->height());

    for(int i = fireBullets.size()-1; i >= 0; i--)
    {
        FireBullet &fb = fireBullets[i];

        fb.x += cos(fb.angle) * fb.speed;
        fb.y += sin(fb.angle) * fb.speed;
        fb.lifetime--;

        fb.label->move(fb.x, fb.y);

        // 生命周期结束
        if(fb.lifetime <= 0)
        {
            fb.label->hide();
            delete fb.label;
            fb.label = nullptr;
            fireBullets.removeAt(i);
            continue;
        }

        // 玩家不隐身时才会被特效击中
        if(!pla.isPlayerStealth)
        {
            QRect bulletRect(fb.x, fb.y, 20, 20);
            if(bulletRect.intersects(playerRect))
            {
                // 防御状态免伤
                if(pla.isDefending)
                {
                    if(fb.label){
                        fb.label->hide();
                        delete fb.label;
                        fb.label=nullptr;
                    }
                    fireBullets.removeAt(i);
                    continue;
                }

                // 扣血
                pla.hp -= 5;
                if(pla.hp < 0) pla.hp = 0;
                updatePlayerHPBar();

                fb.label->hide();
                delete fb.label;
                fb.label = nullptr;
                fireBullets.removeAt(i);

                if(pla.hp <= 0)
                {
                    qDebug() << "你死了！";
                    QMessageBox::information(this, "提示", "你被大Boss的火焰击败！");
                    pla.ismoving = false;
                    clearAll();
                    backHome();
                    return;
                }
            }
        }
    }
}

//场景三怪物特效槽函数，特效定时器超时调用
void Widget::MonsterShootFireBullet()
{
    if(!beginGame) return;
    if(curLevel!=3) return;

    QRect playerRect(player->x(), player->y(), 50, 150);
    int px = player->x();
    int py = player->y();

    Monster &boss = level3Monsters[0];
    if(!boss.isDead)
    {
        int dx = px - boss.monster_x;
        int dy = py - boss.monster_y;
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

            fb.label = new QLabel(this);
            fb.label->setGeometry(fb.x, fb.y, 30, 30);
            fb.label->setPixmap(QPixmap(":/images/effect/firebullet.png"));
            fb.label->setScaledContents(true);
            fb.label->show();
            fb.label->raise();

            fireBullets.append(fb);
        }
    }
}

void Widget::playHomeBGM(){
    levelBGM->stop();
    levelBGM->setSource(QUrl("qrc:/music/mainBGM.ogg"));
    levelBGM->play();
}

void Widget::playLevel1BGM(){
    levelBGM->stop();
    levelBGM->setSource(QUrl("qrc:/music/level_1BGM.ogg"));
    levelBGM->play();
}

void Widget::playLevel2BGM(){
    levelBGM->stop();
    levelBGM->setSource(QUrl("qrc:/music/level_2BGM.ogg"));
    levelBGM->play();
}
void Widget::playLevel3BGM(){
    levelBGM->stop();
    levelBGM->setSource(QUrl("qrc:/music/level_3BGM.ogg"));
    levelBGM->play();
}

void Widget::onstartGame(){
    beginGame=true;
    this->update();
}

//清理函数
void Widget::clearAll(){

    if (monstermoveTimer) {
        monstermoveTimer->stop();
    }
    if (playermoveTimer) {
        playermoveTimer->stop();
    }
    if(MonsterBulletTimer){
        MonsterBulletTimer->stop();
    }
    if(MonsterFireBulletTimer){
        MonsterFireBulletTimer->stop();
    }

    if(player){
        player->hide();
        delete player;
        player=nullptr;
    }

    clearCurrentMonsters();

    if (playerHPBar)
    {
        playerHPBar->hide();
        delete playerHPBar;
        playerHPBar = nullptr;
    }
    if (playerHPBg)
    {
        playerHPBg->hide();
        delete playerHPBg;
        playerHPBg = nullptr;
    }
    if(playerHPText){
        playerHPText->hide();
        delete playerHPText;
        playerHPText = nullptr;
    }
    // 重置玩家血量
    pla.hp = pla.maxHP;
    pla.isAttacked = false;
    pla.isAttacking = false;
    pla.isSuccessed = false;
    pla.attackedBack = false;

    //清理当前剑光特效
    for(int i = 0; i < pla.sowrdEffectList.size(); i++)
    {
        if(pla.sowrdEffectList[i].label){
            pla.sowrdEffectList[i].label->hide();
            delete pla.sowrdEffectList[i].label;
            pla.sowrdEffectList[i].label=nullptr;
        }
    }
    pla.sowrdEffectList.clear();

    // 清空怪物子弹
    for(int i = 0; i < bulletlist.size(); i++)
    {
        if(bulletlist[i].label){
            bulletlist[i].label->hide();
            delete bulletlist[i].label;
            bulletlist[i].label=nullptr;
        }
    }
    bulletlist.clear();

    if(defenseLabel){
        defenseLabel->hide();
        delete defenseLabel;
        defenseLabel=nullptr;
    }
    pla.isDefending=false;

    // 清理陷阱和安全区
    if (trap.label)
    {
        trap.label->hide();
        delete trap.label;
        trap.label = nullptr;
    }
    if (safeZone.label)
    {
        safeZone.label->hide();
        delete safeZone.label;
        safeZone.label = nullptr;
    }
    trap.isAlive = false;
    safeZone.isAlive = false;

    for(int i = 0; i < fireBullets.size(); i++)
    {
        if(fireBullets[i].label){
            fireBullets[i].label->hide();
            delete fireBullets[i].label;
            fireBullets[i].label=nullptr;
        }
    }
    fireBullets.clear();

    pla.isPlayerStealth = false;
    pla.stealthCooldown = false;
    pla.stealthUseCount = 3;

    m_obstaclelist.clear();
    m_traplist.clear();
    m_deadlist.clear();

    if(levelBg){
        levelBg->hide();
        delete levelBg;
        levelBg = nullptr;
    }
}

//血条闪烁槽函数
void Widget::onHPBlinking()
{
    if(!playerHPBar) return;

    double ratio = (double)pla.hp / pla.maxHP;

    // 血量正常停止
    if(ratio >= 0.3)
    {
        hpBlinkTimer->stop();
        return;
    }

    isHPblinding = !isHPblinding;

    // 血条闪烁，切换颜色
    if(isHPblinding)
    {
        playerHPBar->setStyleSheet(R"(
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
            stop:0 #ff2222, stop:1 #ff5555);
            border-radius: 4px;
        )");
    }
    else
    {
        playerHPBar->setStyleSheet(R"(
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
            stop:0 #991b1b, stop:1 #c92a2a);
            border-radius: 4px;
        )");
    }
}

//初始化玩家血条
void Widget::iniPlayerHPBar()
{
    //先清理
    if(playerHPBg)
    {
        delete playerHPBg;
        playerHPBg = nullptr;
    }
    if(playerHPBar)
    {
        delete playerHPBar;
        playerHPBar = nullptr;
    }
    if(playerHPText)
    {
        delete playerHPText;
        playerHPText = nullptr;
    }
    if(hpBlinkTimer)
    {
        hpBlinkTimer->stop();
        delete hpBlinkTimer;
        hpBlinkTimer = nullptr;
    }

    //重置血量
    pla.hp = pla.maxHP;

    // 血条底板
    playerHPBg = new QLabel(this);
    playerHPBg->setGeometry(15, 15, 200, 24);
    playerHPBg->setStyleSheet(R"(
        background-color: #2b2b2b;
        border: 1px solid #1a1a1a;
        border-radius: 6px;
    )");
    playerHPBg->lower();
    playerHPBg->show();

    // 血量条
    playerHPBar = new QLabel(this);
    playerHPBar->setGeometry(17, 17, 196, 20);
    playerHPBar->show();

    // 血量文字
    playerHPText = new QLabel(this);
    playerHPText->setGeometry(15, 15, 200, 24);
    playerHPText->setAlignment(Qt::AlignCenter);
    playerHPText->setStyleSheet(R"(
        color: #f2e9d8;
        font-size: 10px;
        font-weight: bold;
        background: transparent;
    )");
    playerHPText->raise();
    playerHPText->show();

    // 闪烁定时器
    isHPblinding = false;
    hpBlinkTimer = new QTimer(this);
    hpBlinkTimer->setInterval(450);
    connect(hpBlinkTimer, &QTimer::timeout, this, &Widget::onHPBlinking);

    updatePlayerHPBar();
}

//更新玩家血条
void Widget::updatePlayerHPBar()
{
    if (!playerHPBar || !playerHPBg || !playerHPText) return;

    double ratio = (double)pla.hp / pla.maxHP;
    int barFullW = 196;
    int nowW = barFullW * ratio;

    playerHPBar->setFixedWidth(nowW);
    playerHPText->setText(QString("%1 / %2").arg(pla.hp).arg(pla.maxHP));

    // 低血量开启闪烁
    if(ratio < 0.3)
    {
        if(!hpBlinkTimer->isActive())
        {
            hpBlinkTimer->start();
        }
    }
    else
    {
        hpBlinkTimer->stop();
        // 正常血量颜色
        if(ratio <= 0)
        {
            playerHPBar->setStyleSheet(R"(background-color: #111111;border-radius: 4px;)");
        }
        else if(ratio < 0.6)
        {
            playerHPBar->setStyleSheet(R"(
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #b8860b, stop:1 #d4af37);
                border-radius: 4px;
            )");
        }
        else
        {
            playerHPBar->setStyleSheet(R"(
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #205c20, stop:1 #378b37);
                border-radius: 4px;
            )");
        }
    }
}

//加载角色动画
void Widget::PlayerPicload()
{
    pla.playerPicdown.clear();
    pla.playerPicup.clear();
    pla.playerPicleft.clear();
    pla.playerPicright.clear();
    pla.playerFightLeft.clear();
    pla.playerFightRight.clear();
    pla.playerAttackedLeft.clear();
    pla.playerAttackedRight.clear();

    QString basePath;
    switch(pla.curRole)
    {
    case 1: // 角色1
        basePath = ":/role1";
        break;
    case 2: // 角色2
        basePath = ":/role2";
        break;
    case 3: // 角色3
        basePath = ":/role3";
        break;
    case 4: // 角色4
        basePath = ":/role4";
        break;
    default:
        basePath = ":/role1"; // 默认角色1
        break;
    }
    //加载站立帧
    pla.playerPicStand.load(basePath + "/stand.png");

    //加载向上帧
    pla.playerPicup << QPixmap(basePath + "/up/1.png")
                << QPixmap(basePath + "/up/2.png")
                << QPixmap(basePath + "/up/3.png");

    //加载向下帧
    pla.playerPicdown << QPixmap(basePath + "/down/1.png")
                  << QPixmap(basePath + "/down/2.png")
                  << QPixmap(basePath + "/down/3.png");

    //加载向左帧
    pla.playerPicleft << QPixmap(basePath + "/left/1.png")
                  << QPixmap(basePath + "/left/2.png")
                  << QPixmap(basePath + "/left/3.png");

    //加载向右帧
    pla.playerPicright << QPixmap(basePath + "/right/1.png")
                   << QPixmap(basePath + "/right/2.png")
                   << QPixmap(basePath + "/right/3.png");

    //加载战斗帧
    pla.playerFightLeft<< QPixmap(basePath + "/fight/left1.png")
                    << QPixmap(basePath + "/fight/left2.png")
                    << QPixmap(basePath + "/fight/left3.png");
    pla.playerFightRight<< QPixmap(basePath + "/fight/right1.png")
                     << QPixmap(basePath + "/fight/right2.png")
                     << QPixmap(basePath + "/fight/right3.png");

    //加载后退帧
    pla.playerAttackedLeft<< QPixmap(basePath + "/back/left1.png")
                           << QPixmap(basePath + "/back/left2.png")
                           << QPixmap(basePath + "/back/left3.png");
    pla.playerAttackedRight<< QPixmap(basePath + "/back/right1.png")
                            << QPixmap(basePath + "/back/right2.png")
                            << QPixmap(basePath + "/back/right3.png");
}

//加载场景1怪物图片
void Widget::loadLevel1MonsterPics()
{
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

    level1Monsters.clear();
    level1Monsters.append(m);
}

//加载场景2怪物图片
void Widget::loadLevel2MonsterPics(){

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


    level2Monsters.clear();
    level2Monsters.append(m1);

    Monster m2=m1;
    level2Monsters.append(m2);
}

void Widget::loadLevel3MonsterPics(){

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

    level3Monsters.clear();
    level3Monsters.append(mBoss);
    level3Monsters.append(minion1);
    Monster minion2=minion1;
    level3Monsters.append(minion2);
}

//初始化怪物血量
void Widget::iniMonsterHP(Monster &m)
{
    //血条底板
    m.monsterHpBg = new QLabel(this);
    m.monsterHpBg->setFixedSize(70, 8);
    m.monsterHpBg->setStyleSheet(R"(
    background-color: #222222;
    border-radius: 4px;
)");

    //血量条
    m.monsterHpbar = new QLabel(this);
    m.monsterHpbar->setFixedSize(66, 6);
    m.monsterHpbar->setStyleSheet(R"(
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
    stop:0 #a82020, stop:1 #d13030);
    border-radius: 3px;
)");
    //初始位置
    m.monsterHpBg->move(m.monster_x, m.monster_y - 18);
    m.monsterHpbar->move(m.monster_x + 2, m.monster_y - 17);
    m.monsterHpBg->lower();
    m.monsterHpbar->raise();
    m.monsterHpBg->show();
    m.monsterHpbar->show();
}

//更新怪物血条
void Widget::updateMonsterHP(Monster &m)
{
    if(!m.monsterHpBg || !m.monsterHpbar) return;

    // 血量比例
    double ratio = (double)m.monsterHP / m.monsterMaxHP;
    int fullW = 66;
    int curW = fullW * ratio;

    // 刷新血条宽度
    m.monsterHpbar->setFixedWidth(curW);

    // 跟随怪物移动
    m.monsterHpBg->move(m.monster_x, m.monster_y - 18);
    m.monsterHpbar->move(m.monster_x + 2, m.monster_y - 17);

    // 低血量变色警告
    if(ratio < 0.3)
    {
        m.monsterHpbar->setStyleSheet(R"(
            background: #ff3333;
            border-radius: 3px;
        )");
    }
    else
    {
        m.monsterHpbar->setStyleSheet(R"(
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
            stop:0 #a82020, stop:1 #d13030);
            border-radius: 3px;
        )");
    }
}

// 场景1怪物初始化
void Widget::initLevel1Monsters()
{
    if(level1Monsters.isEmpty()) return;

    Monster &m = level1Monsters[0];
    m.monsterDir = 1;          // 初始向下
    m.monsterPicID = 0;
    m.monster_x = 400;
    m.monster_y = 200;
    m.monsterSpeed = 2;

    // 创建怪物
    m.label = new QLabel(this);
    m.label->setScaledContents(1);
    m.label->setFixedSize(60, 60);
    m.label->move(m.monster_x, m.monster_y);
    m.label->raise();
    m.label->show();
}

// 场景2怪物初始化
void Widget::initLevel2Monsters(){

    if (level2Monsters.isEmpty()) return;

    Monster &m1 = level2Monsters[0];
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

    iniMonsterHP(m1);

    // 创建怪物1-Label
    m1.label = new QLabel(this);
    m1.label->setScaledContents(1);
    m1.label->setFixedSize(50,65);
    m1.label->move(m1.monster_x, m1.monster_y);
    m1.label->raise();
    m1.label->show();

    Monster &m2 = level2Monsters[1];
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

    iniMonsterHP(m2);

    // 创建怪物2-Label
    m2.label = new QLabel(this);
    m2.label->setScaledContents(1);
    m2.label->setFixedSize(50,65);
    m2.label->move(m2.monster_x, m2.monster_y);
    m2.label->raise();
    m2.label->show();
}

// 场景3怪物初始化
void Widget::initLevel3Monsters(){

    if (level3Monsters.isEmpty()) return;

    Monster &m1 = level3Monsters[0];
    m1.monsterDir = 0;
    m1.monsterPicID = 0;
    m1.monster_x = 390;
    m1.monster_y = 120;
    m1.isHurt=false;
    m1.isDead=false;
    m1.monsterMaxHP = 25;
    m1.monsterHP = m1.monsterMaxHP;
    m1.beHitOnce=false;

    iniMonsterHP(m1);

    // 创建怪物1-Label
    m1.label = new QLabel(this);
    m1.label->setScaledContents(1);
    m1.label->setFixedSize(200,200);
    m1.label->move(m1.monster_x, m1.monster_y);
    m1.label->raise();
    m1.label->show();

    Monster &m2 = level3Monsters[1];
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

    iniMonsterHP(m2);
    m2.monsterHpbar->hide();
    m2.monsterHpBg->hide();

    // 创建怪物2-Label
    m2.label = new QLabel(this);
    m2.label->setScaledContents(1);
    m2.label->setFixedSize(70,75);
    m2.label->move(m2.monster_x, m2.monster_y);
    m2.label->raise();

    Monster &m3 = level3Monsters[2];
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

    iniMonsterHP(m3);
    m3.monsterHpbar->hide();
    m3.monsterHpBg->hide();

    // 创建怪物3-Label
    m3.label = new QLabel(this);
    m3.label->setScaledContents(1);
    m3.label->setFixedSize(70,75);
    m3.label->move(m3.monster_x, m3.monster_y);
    m3.label->raise();
}

//清空当前所有怪物
void Widget::clearCurrentMonsters()
{
    if (monstermoveTimer) {
        monstermoveTimer->stop();
    }

    // 根据当前关卡，清空对应怪物列表
    if (curLevel == 1) {
        for (Monster &m : level1Monsters) {
            if (m.label) {
                m.label->hide();
                m.label->setParent(nullptr);
                delete m.label;
                m.label = nullptr;
            }
        }
        level1Monsters.clear();
    }
    else if (curLevel == 2) {
        for (Monster &m : level2Monsters) {
            if(m.monsterHpBg){
                m.monsterHpBg->hide();
                delete m.monsterHpBg;
                m.monsterHpBg=nullptr;
            }
            if(m.monsterHpbar){
                m.monsterHpbar->hide();
                delete m.monsterHpbar;
                m.monsterHpbar=nullptr;
            }
            if (m.label) {
                m.label->hide();
                delete m.label;
                m.label = nullptr;
            }
        }
        level2Monsters.clear();
    }
    else if(curLevel==3){
        for (Monster &m : level3Monsters) {
            if(m.monsterHpBg){
                m.monsterHpBg->hide();
                delete m.monsterHpBg;
                m.monsterHpBg=nullptr;
            }
            if(m.monsterHpbar){
                m.monsterHpbar->hide();
                delete m.monsterHpbar;
                m.monsterHpbar=nullptr;
            }
            if (m.label) {
                m.label->hide();
                delete m.label;
                m.label = nullptr;
            }
        }
        level3Monsters.clear();
    }

    curLevel = 0;
}


void Widget::on_start_btn_clicked()
{
    rule->move(160,90);
    rule->show();
    ui->level_1->show();
    ui->level_2->show();
    ui->level_3->show();
    ui->role_btn->show();
    ui->level_1->setEnabled(1);
    ui->role_btn->setEnabled(1);
    ui->back->hide();
    ui->start_btn->hide();
}

void Widget::on_role_btn_clicked()
{
    ui->level_1->hide();
    ui->level_2->hide();
    ui->level_3->hide();
    ui->start_btn->hide();
    ui->role_btn->hide();
    ui->back->hide();

    RoleSelect dialog(this);
    if(dialog.exec()==QDialog::Accepted){
        pla.curRole=dialog.getRole();
        qDebug()<<"当前选中角色： "<<pla.curRole;
    }

    ui->level_1->show();
    ui->level_2->show();
    ui->level_3->show();
    ui->role_btn->show();
}

//角色移动
void Widget::movePlayer(){

    if(!beginGame) return;
    if(!player) return;

    int originalX=player->x();
    int originalY=player->y();
    int x=originalX;
    int y=originalY;
    pla.ismoving=false;
    int step=5;
    if(pla.up){
        pla.ismoving=true;
        pla.playerDir=2;
        y-=step;
    }
    if(pla.down){
        pla.ismoving=true;
        pla.playerDir=1;
        y+=step;
    }
    if(pla.left){
        pla.ismoving=true;
        pla.playerDir=3;
        x-=step;
    }
    if(pla.right){
        pla.ismoving=true;
        pla.playerDir=4;
        x+=step;
    }
    if(x<0) x=0;
    if(y<0) y=0;
    if(x>width()-player->width()) x=width()-player->width();
    if(y>height()-player->height()) y=height()-player->height();

    player->move(x,y);

    bool hasCollided=false;
    QRect playerRect=player->geometry();
    for(const QRect&obsRect:m_obstaclelist){
        if(playerRect.intersects(obsRect)){
            hasCollided=true;
            break;
        }
    }
    if(hasCollided){
        player->move(originalX,originalY);
        return;
    }
    for (const QRect &trap : m_traplist) {
        if (playerRect.intersects(trap)) {

            qDebug() << "触发陷阱，请求切换场景二";
            ui->level_2->setEnabled(true);

            int ans = QMessageBox::question(this,"场景切换提示","你触发了陷阱，是否进入场景二？",
                                            QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
            if (ans == QMessageBox::Yes) {
                qDebug() << "用户确认，切换到场景二";
                clearAll();
                on_level_2_clicked();
            }
            else {
                qDebug() << "用户取消，停留在场景一";
                player->move(440,380);
            }
            return;
        }
    }
    for (const QRect &water : m_deadlist) {
        if (playerRect.intersects(water)) {

            qDebug() << "掉进水坑，死亡重启";
            QMessageBox::information(this, "提示", "你掉入了水中，挑战失败！");
            pla.ismoving = false;
            clearAll();
            backHome();
            return;
        }
    }

    QPixmap curPic;
    if(pla.isAttacking)
    {
        pla.attackTime++;
        if(pla.playerDir==3){
            pla.playerAttackPic++;
            if(pla.playerAttackPic >= pla.playerFightLeft.size()){
                pla.playerAttackPic = pla.playerFightLeft.size()-1;
            }
            curPic=pla.playerFightLeft[pla.playerAttackPic];
        }
        else if(pla.playerDir==4){
            pla.playerAttackPic++;
            if(pla.playerAttackPic >= pla.playerFightRight.size()){
                pla.playerAttackPic = pla.playerFightRight.size()-1;
            }
            curPic=pla.playerFightRight[pla.playerAttackPic];
        }

        player->setPixmap(curPic);
        player->setScaledContents(1);
        player->setFixedSize(80,150);

        if(pla.attackTime > 3 )
        {
            pla.isAttacking = false;
            pla.attackTime = 0;
            pla.playerAttackPic = 0;
            curPic=pla.playerPicStand;
            player->setPixmap(curPic);
            player->setScaledContents(1);
            player->setFixedSize(50,150);
        }
    }
    else if(pla.attackedBack)
    {
        bool canmove = true;
        int step = 10;

        if(pla.BackDir == 3)
        {
            if(curLevel == 2&&player->x() - step < 270){
                canmove = false;
            }
            if(curLevel == 3&&player->x() - step < 335){
                canmove = false;
            }
            if(canmove){
                player->move(player->x() - step, player->y());
            }
        }
        if(pla.BackDir == 4)
        {
            if(curLevel == 2&&player->x() + step +80 > 690){
                canmove = false;
            }
            if(curLevel == 3&&player->x() + step +80 > 625){
                canmove = false;
            }
            if(canmove){
                player->move(player->x() + step, player->y());
            }
        }
        if(pla.playerBackedPic < pla.playerAttackedRight.size())
        {
            if(pla.BackDir == 3)
            {
                curPic = pla.playerAttackedRight[pla.playerBackedPic];
            }
            else
            {
                curPic = pla.playerAttackedLeft[pla.playerBackedPic];
            }
            player->setPixmap(curPic);
            player->setScaledContents(1);
            player->setFixedSize(100,150);
            pla.playerBackedPic++;
        }
        else{
            pla.attackedBack = false;
            pla.playerBackedPic = 0;
            curPic=pla.playerPicStand;
            player->setPixmap(curPic);
            player->setScaledContents(1);
            player->setFixedSize(50,150);
        }
    }
    else{
        if(pla.ismoving) {

            pla.playermoveCount++;
            if(pla.playermoveCount >= pla.maxmoveCount){
                pla.playermoveCount = 0;
                pla.playerPicID = (pla.playerPicID+1) % 3;
            }

            switch(pla.playerDir){
            case 1:
                curPic=pla.playerPicdown[pla.playerPicID];
                break;
            case 2:
                curPic=pla.playerPicup[pla.playerPicID];
                break;
            case 3:
                curPic=pla.playerPicleft[pla.playerPicID];
                break;
            case 4:
                curPic=pla.playerPicright[pla.playerPicID];
                break;
            default:
                curPic=pla.playerPicStand;
                break;
            }
        }
        else{
            curPic=pla.playerPicStand;
            pla.playerPicID =0;
            pla.playermoveCount = 0;
        }
        player->setPixmap(curPic);
        player->setScaledContents(1);
        player->setFixedSize(50,150);
    }
}

//怪物移动
void Widget::moveMonster(){

    if(!beginGame) return;

    //根据当前场景，选择对应的怪物列表
    QList<Monster> *curMonsters = nullptr;
    if (curLevel == 1)
        curMonsters = &level1Monsters;
    else if (curLevel == 2)
        curMonsters = &level2Monsters;
    else if(curLevel == 3){
        curMonsters = &level3Monsters;
    }

    //未排查出程序崩溃原因时，AI提供防崩溃代码
    if (!curMonsters || curMonsters->isEmpty())
        return;

    //无论进入哪个场景，都设置动画帧
    for (Monster &monster : *curMonsters)
    {
        if (!monster.label)  continue;

        // 动画帧
        // 站立状态
        if (monster.monsterDir == 0)
        {
            monster.label->setPixmap(monster.picStand);
            monster.monsterPicID = 0;
            continue;
        }
        // 走路状态
        int PicCount = 0;
        if (monster.monsterDir == 1) { // 向下
            PicCount = monster.monsterpicDown.size();
        } else if (monster.monsterDir == 2) { // 向上
            PicCount = monster.monsterpicUp.size();
        } else if (monster.monsterDir == 3) { // 向左
            PicCount = monster.monsterpicLeft.size();
        } else if (monster.monsterDir == 4) { // 向右
            PicCount = monster.monsterpicRight.size();
        }
        monster.monsterPicID = (monster.monsterPicID + 1) % PicCount;

        QPixmap currentPic;
        // 1=下，2=上，3=左，4=右
        switch (monster.monsterDir)
        {
        case 1: // 向下
            currentPic = monster.monsterpicDown[monster.monsterPicID];
            break;
        case 2: // 向上
            currentPic = monster.monsterpicUp[monster.monsterPicID];
            break;
        case 3: // 向左
            currentPic = monster.monsterpicLeft[monster.monsterPicID];
            break;
        case 4: // 向右
            currentPic = monster.monsterpicRight[monster.monsterPicID];
            break;
        default:
            currentPic = monster.picStand;
            break;
        }
        if(monster.isHurt && curLevel !=3)
        {
            if(monster.monsterDir==3){
                if(monster.monsterPicHurtLeft.isEmpty()){
                    continue;
                }
                monster.monsterHurtID = (monster.monsterHurtID + 1) % monster.monsterPicHurtLeft.size();
                currentPic = monster.monsterPicHurtLeft[monster.monsterHurtID];
            }
            else if(monster.monsterDir==4){
                if(monster.monsterPicHurtRight.isEmpty()){
                    continue;
                }
                monster.monsterHurtID = (monster.monsterHurtID + 1) % monster.monsterPicHurtRight.size();
                currentPic = monster.monsterPicHurtRight[monster.monsterHurtID];
            }
            else{
                if(monster.monsterPicHurtFront.isEmpty()){
                    continue;
                }
                monster.monsterHurtID = (monster.monsterHurtID + 1) % monster.monsterPicHurtFront.size();
                currentPic = monster.monsterPicHurtFront[monster.monsterHurtID];
            }
        }
        if(curLevel ==3 &&monster.isAlive && monster.isFighting){
            if(monster.monsterDir==3){
                if(monster.monsterPicFightLeft.isEmpty()){
                    continue;
                }
                monster.monsterFightID = (monster.monsterFightID + 1) % monster.monsterPicFightLeft.size();
                currentPic = monster.monsterPicFightLeft[monster.monsterFightID];
            }
            else if(monster.monsterDir==4){
                if(monster.monsterPicFightRight.isEmpty()){
                    continue;
                }
                monster.monsterFightID = (monster.monsterFightID + 1) % monster.monsterPicFightRight.size();
                currentPic = monster.monsterPicFightRight[monster.monsterFightID];
            }
        }
        monster.label->setPixmap(currentPic);
    }

    //每个场景的具体设置
    if (curLevel == 1){

        for (Monster &monster : *curMonsters){

            monster.label->show();
            // 位置移动
            // 获取怪物当前坐标
            int x = monster.monster_x;
            int y = monster.monster_y;

            // 巡逻怪按方向移动（1=下，2=上）
            switch (monster.monsterDir)
            {
            case 1: // 向下
                y += monster.monsterSpeed;
                if (y > 420)
                    monster.monsterDir = 2;
                break;
            case 2: // 向上
                y -= monster.monsterSpeed;
                if (y < 180)
                    monster.monsterDir = 1;
                break;
            default:
                break;
            }

            // 更新怪物坐标
            monster.monster_x = x;
            monster.monster_y = y;
            monster.label->move(x, y);

            moveMonsterBullets();
        }
        //陷阱刷新
        trap.lifetime++;
        if (trap.lifetime >= 30)
        {
            trap.lifetime = 0;

            // 清理旧陷阱
            if (trap.label)
            {
                trap.label->hide();
                delete trap.label;
                trap.label = nullptr;
            }

            // 随机位置生成新陷阱
            trap.x = 230+rand() % 450;
            trap.y = 150+rand() % 250;

            trap.label = new QLabel(this);
            trap.label->setGeometry(trap.x, trap.y, trap.width, trap.height);
            trap.label->setPixmap(QPixmap(":/images/effect/trap.png"));
            trap.label->setScaledContents(true);
            trap.label->show();

            trap.isAlive = true;
        }

        //安全区刷新
        safeZone.lifetime++;
        if (safeZone.lifetime >= 50)
        {
            safeZone.lifetime = 0;

            // 清理旧安全区
            if (safeZone.label)
            {
                safeZone.label->hide();
                delete safeZone.label;
                safeZone.label = nullptr;
            }

            // 随机位置生成新安全区
            safeZone.x = 230+rand() % 450;
            safeZone.y = 150+rand() % 250;

            safeZone.label = new QLabel(this);
            safeZone.label->setGeometry(safeZone.x, safeZone.y, safeZone.width, safeZone.height);
            safeZone.label->setPixmap(QPixmap(":/images/effect/safezone.png"));
            safeZone.label->setScaledContents(true);
            safeZone.label->show();

            safeZone.isAlive = true;
        }

        //玩家踩陷阱扣血
        QRect playerRect(player->x(), player->y(), 50, 150);
        if (trap.isAlive && trap.label != nullptr)
        {
            if (playerRect.intersects(trap.label->geometry()))
            {
                pla.hp -= 2;
                if (pla.hp < 0) pla.hp = 0;
                updatePlayerHPBar();
            }
        }

        // 玩家死亡
        if(pla.hp<=0){
            pla.hp=0;
            updatePlayerHPBar();
            qDebug() << "你死了！";
            QMessageBox::information(this, "提示", "你被烈焰侵蚀了身体，挑战失败！");
            pla.ismoving = false;
            clearAll();
            backHome();
        }
    }

    else if(curLevel == 2){

        if(pla.atkCd>0) pla.atkCd--;

        int px = player->x();
        int py = player->y();

        // 场景2怪物
        for(int i = 0; i < level2Monsters.size(); i++)
        {

            if(pla.isSuccessed){
                return;
            }

            Monster &m = level2Monsters[i];

            if(m.isDead){
                if(m.label){
                    m.label->hide();
                    delete m.label;
                    m.label = nullptr;
                }
                if(m.monsterHpbar){
                    m.monsterHpbar->hide();
                    delete m.monsterHpbar;
                    m.monsterHpbar = nullptr;
                }
                if(m.monsterHpBg){
                    m.monsterHpBg->hide();
                    delete m.monsterHpBg;
                    m.monsterHpBg = nullptr;
                }
                continue;
            }

            // 追踪玩家
            int dx = px - m.monster_x;
            int dy = py - m.monster_y;

            if(abs(dx) > abs(dy))
            {
                if(dx > 0)
                    m.monsterDir = 4; // 右
                else
                    m.monsterDir = 3; // 左
            }
            else
            {
                if(dy > 0)
                    m.monsterDir = 1; // 下
                else
                    m.monsterDir = 2; // 上
            }

            double smooth=0.08;
            int newX = m.monster_x + dx * smooth;
            int newY = m.monster_y + dy * smooth;

            QRect monsterNewRect(newX, newY, 40, 50);//怪物矩形比怪物尺寸小，这样怪物才有可能攻击到玩家

            bool canMove = true;

            //排斥玩家：离玩家过近就不移动,防止与玩家重叠
            QRect playerRect(px+10, py+10, 30, 120);
            if (monsterNewRect.intersects(playerRect))
            {
                canMove = false;
            }

            //排斥其他怪物：离其他怪物过近就不移动，防止互相重叠
            for (auto &other : level2Monsters)
            {
                if (&other == &m || other.isDead || !other.label) continue;

                QRect otherRect(other.monster_x, other.monster_y, 40, 50);
                if (monsterNewRect.intersects(otherRect))
                {
                    canMove = false;
                    break;
                }
            }

            if (canMove)
            {
                m.monster_x = newX;
                m.monster_y = newY;
                if(m.label){
                    m.label->move(m.monster_x, m.monster_y);
                }
            }

            if(m.monsterHpbar&&m.monsterHpBg){
                updateMonsterHP(m);
            }

            if(m.label&&m.label->geometry().intersects(player->geometry()))
            {
                if(pla.isDefending||pla.isAttacked){
                    continue;
                }

                pla.isAttacked=true;
                pla.hp-=3;
                qDebug()<<"玩家被攻击，当前血量："<<pla.hp;
                updatePlayerHPBar();

                if(!pla.isAttacking){
                    int dx = player->x() - m.monster_x;
                    if(dx > 0)
                    {
                        pla.BackDir = 4;
                    }
                    else
                    {
                        pla.BackDir = 3;
                    }

                    pla.attackedBack = true;
                    pla.playerBackedPic = 0;
                }

                // 玩家死亡
                if(pla.hp<=0){
                    pla.hp=0;
                    updatePlayerHPBar();
                    for(int k=pla.sowrdEffectList.size()-1;k>=0;k--){
                        auto & sowrd = pla.sowrdEffectList[k];
                        if(sowrd.label){
                            sowrd.label->hide();
                            delete sowrd.label;
                            sowrd.label = nullptr;
                        }
                    }
                    pla.sowrdEffectList.clear();
                    qDebug() << "你死了！";
                    QMessageBox::information(this, "提示", "你被怪物击败，挑战失败！");
                    clearAll();
                    backHome();
                }
            }
        }
        if(pla.isAttacked){
            pla.hurtTime++;
            if(pla.hurtTime>2){
                pla.isAttacked=false;
                pla.hurtTime = 0;
                pla.attackedBack = false;
                pla.playerBackedPic = 0;
            }
        }
        if(pla.isAttacking&&pla.atkCd<=0)
        {
            pla.attackTime++;

            //根据玩家方向，动态生成攻击范围
            QRect attackArea;
            if (pla.playerDir == 4) {     // 向右攻击
                attackArea = player->geometry().adjusted(20, 0, 20, 0);
            } else if (pla.playerDir == 3) { // 向左攻击
                attackArea = player->geometry().adjusted(-20, 0, 20, 0);
            } else if (pla.playerDir == 1) { // 向下攻击
                attackArea = player->geometry().adjusted(0, 20, 0, 20);
            } else if (pla.playerDir == 2) { // 向上攻击
                attackArea = player->geometry().adjusted(0, -20, 0, 20);
            }

            bool haveHitOne = false;
            // 遍历所有怪物，检查是否被攻击
            for(int i = 0; i < level2Monsters.size(); i++)
            {
                if(haveHitOne) {
                    break;
                }
                Monster &m = level2Monsters[i];
                if(m.isDead||!m.label) continue;

                if(attackArea.intersects(m.label->geometry())&&!m.beHitOnce)
                {
                    // 扣血 ，触发受伤动画
                    m.monsterHP--;
                    m.isHurt = true;
                    m.beHitOnce = true;
                    haveHitOne = true;

                    if(m.monsterHP <= 0)
                    {
                        m.isDead = true;

                        if(m.label)
                        {
                            m.label->hide();
                            delete m.label;
                            m.label = nullptr;
                        }
                        if(m.monsterHpbar)
                        {
                            m.monsterHpbar->hide();
                            delete m.monsterHpbar;
                            m.monsterHpbar = nullptr;
                        }
                        if(m.monsterHpBg){
                            m.monsterHpBg->hide();
                            delete m.monsterHpBg;
                            m.monsterHpBg = nullptr;
                        }
                        pla.killcount++;
                    }
                }
            }
            // 攻击动作持续一段时间结束
            if(pla.attackTime > 3 )
            {
                pla.isAttacking = false;
                pla.attackTime=0;
                pla.atkCd=3;
                for(int i = 0; i < level2Monsters.size(); i++){
                    Monster &m = level2Monsters[i];
                    m.beHitOnce = false;
                    m.isHurt = false;
                }
                //清理当前剑光特效(正向遍历程序崩溃)
                for(int k=pla.sowrdEffectList.size()-1;k>=0;k--){
                    auto & sowrd = pla.sowrdEffectList[k];
                    if(sowrd.label){
                        sowrd.label->hide();
                        delete sowrd.label;
                        sowrd.label = nullptr;
                    }
                }
                pla.sowrdEffectList.clear();
            }
        }

        if(pla.killcount == 2)
        {
            pla.isSuccessed = true;
            for(int k=pla.sowrdEffectList.size()-1;k>=0;k--){
                auto & sowrd = pla.sowrdEffectList[k];
                if(sowrd.label){
                    sowrd.label->hide();
                    delete sowrd.label;
                    sowrd.label = nullptr;
                }
            }
            pla.sowrdEffectList.clear();

            if(pla.isSuccessed){
                pla.isAttacking = false;
                levelBGM->stop();
                winBGM->setPosition(0);
                winBGM->play();
                qDebug() << "打怪胜利，恭喜解锁场景三";
                ui->level_3->setEnabled(true);

                int ans = QMessageBox::question(this,"场景切换提示","你战胜了怪物，是否进入场景三？",
                                                QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
                if (ans == QMessageBox::Yes) {
                    qDebug() << "用户确认，切换到场景三";

                    clearAll();
                    on_level_3_clicked();
                }
                else {
                    winBGM->stop();
                    levelBGM->setPosition(0);
                    levelBGM->play();
                    qDebug() << "用户取消，停留在场景二";
                    player->move(440,380);
                }
                return;
            }
        }
        // 防御屏障跟随玩家实时移动
        if(pla.isDefending && defenseLabel && player)
        {
            defenseLabel->move(player->x()-15, player->y()-10);
        }
    }
    else if(curLevel == 3)
    {
        QRect playerRect(player->x(), player->y(), 50, 150);
        int px = player->x();
        int py = player->y();

        for(int i = 1; i <= 2; i++)
        {
            if(pla.isSuccessed){
                return;
            }

            Monster &m = level3Monsters[i];

            if(m.isDead){
                if(m.label){
                    m.label->hide();
                    delete m.label;
                    m.label = nullptr;
                }
                if(m.monsterHpbar){
                    m.monsterHpbar->hide();
                    delete m.monsterHpbar;
                    m.monsterHpbar = nullptr;
                }
                if(m.monsterHpBg){
                    m.monsterHpBg->hide();
                    delete m.monsterHpBg;
                    m.monsterHpBg = nullptr;
                }
                m.isAlive = false;
                m.monster_x =0;
                m.monster_y =0;
                continue;
            }

            int dx = px - m.monster_x;
            int dy = py - m.monster_y;
            double dist = sqrt(dx * dx + dy * dy);

            if(dx>0){
                m.monsterDir=4;
            }
            else{
                m.monsterDir=3;
            }

            // 小怪现身
            if(!m.isAlive && dist < 250)
            {
                m.isAlive = true;

                if(m.label){
                    m.label->show();
                }
                if(m.monsterHpBg){
                    m.monsterHpBg->show();
                }
                if(m.monsterHpbar){
                    m.monsterHpbar->show();
                }
            }

            // 小怪追踪
            if(m.isAlive && !pla.isPlayerStealth)
            {
                double smooth = 0.06;
                int newX = m.monster_x + dx * smooth;
                int newY = m.monster_y + dy * smooth;

                QRect newRect(newX, newY, 40, 50);
                bool canMove = true;

                // 排斥玩家
                if(newRect.intersects(playerRect))
                    canMove = false;

                //排斥其他怪物：离其他怪物过近就不移动，防止互相重叠
                for (auto &other : level3Monsters)
                {
                    if(&other == &m||other.isDead || !other.isAlive||!other.label)
                        continue;

                    QRect otherRect(other.monster_x, other.monster_y, 40, 50);
                    if (newRect.intersects(otherRect))
                    {
                        canMove = false;
                        break;
                    }
                }

                if(canMove)
                {
                    m.monster_x = newX;
                    m.monster_y = newY;
                    if(m.label){
                        m.label->move(m.monster_x, m.monster_y);
                    }
                }
                if(m.monsterHpbar&&m.monsterHpBg){
                    updateMonsterHP(m);
                }
            }

            QRect fightRect(m.monster_x-20,m.monster_y-10,80,90);

            if(pla.isDefending||pla.isPlayerStealth||pla.isAttacked) continue;


            if(m.isAlive&&fightRect.intersects(player->geometry()))
            {

                m.isFighting = true;

                pla.isAttacked=true;
                pla.hp-=5;
                qDebug()<<"玩家被攻击，当前血量："<<pla.hp;
                updatePlayerHPBar();

                if(!pla.isAttacking){
                    int dx = player->x() - m.monster_x;
                    if(dx > 0)
                    {
                        pla.BackDir = 4;
                    }
                    else
                    {
                        pla.BackDir = 3;
                    }

                    pla.attackedBack = true;
                    pla.playerBackedPic = 0;
                }

                // 玩家死亡
                if(pla.hp<=0){
                    pla.hp=0;
                    updatePlayerHPBar();
                    qDebug() << "你死了！";
                    if(monstermoveTimer){
                        monstermoveTimer->stop();
                    }
                    for(int k=pla.sowrdEffectList.size()-1;k>=0;k--){
                        auto & sowrd = pla.sowrdEffectList[k];
                        if(sowrd.label){
                            sowrd.label->hide();
                            delete sowrd.label;
                            sowrd.label = nullptr;
                        }
                    }
                    QMessageBox::information(this, "提示", "你被怪物击败，挑战失败！");
                    clearAll();
                    backHome();
                    return;
                }
            }
        }
        if(pla.isAttacked){
            pla.hurtTime++;
            if(pla.hurtTime>3){
                pla.isAttacked=false;
                pla.hurtTime = 0;
                pla.attackedBack = false;
                pla.playerBackedPic = 0;
                for(int i=1;i<=2;i++){
                    Monster &m = level3Monsters[i];
                    if(m.isDead||!m.label||!m.monsterHpbar) continue;
                    m.isFighting = false;
                }
            }
        }
        if(pla.isAttacking)
        {
            pla.attackTime++;
            //根据玩家朝向，动态生成攻击范围
            QRect attackArea;
            if (pla.playerDir == 4) {     // 向右攻击
                attackArea = player->geometry().adjusted(20, 0, 30, 0);
            } else if (pla.playerDir == 3) { // 向左攻击
                attackArea = player->geometry().adjusted(-30, 0, 20, 0);
            } else if (pla.playerDir == 1) { // 向下攻击
                attackArea = player->geometry().adjusted(0, 20, 0, 30);
            } else if (pla.playerDir == 2) { // 向上攻击
                attackArea = player->geometry().adjusted(0, -30, 0, 20);
            }

            // 遍历所有怪物，检查是否被攻击
            for(int i = 0; i <level3Monsters.size(); i++)
            {
                Monster &m = level3Monsters[i];
                if(m.isDead||!m.label||!m.monsterHpbar) continue;

                if(attackArea.intersects(m.label->geometry())&&!m.beHitOnce)
                {
                    // 扣血 + 触发受伤动画
                    m.monsterHP--;
                    qDebug()<<"第"<<i<<"只小怪血量为"<<m.monsterHP;
                    m.isHurt = true;
                    m.beHitOnce = true;

                    if(m.monsterHP <= 0)
                    {
                        m.isDead = true;

                        if(m.label)
                        {
                            m.label->hide();
                            delete m.label;
                            m.label = nullptr;
                        }
                        if(m.monsterHpbar)
                        {
                            m.monsterHpbar->hide();
                            delete m.monsterHpbar;
                            m.monsterHpbar = nullptr;
                        }
                        if(m.monsterHpBg){
                            m.monsterHpBg->hide();
                            delete m.monsterHpBg;
                            m.monsterHpBg = nullptr;
                        }
                        pla.killcount++;
                    }
                }
            }
            // 攻击动作持续一段时间结束
            if(pla.attackTime > 3 )
            {
                pla.isAttacking = false;
                pla.attackTime=0;
                for(int i = 0; i <level3Monsters.size(); i++){
                    Monster &m = level3Monsters[i];
                    if(m.isDead||!m.label||!m.monsterHpbar) continue;
                    m.beHitOnce = false;
                }
                for(int k=pla.sowrdEffectList.size()-1;k>=0;k--){
                    auto & sowrd = pla.sowrdEffectList[k];
                    if(sowrd.label){
                        sowrd.label->hide();
                        delete sowrd.label;
                        sowrd.label = nullptr;
                    }
                }
                pla.sowrdEffectList.clear();
            }
        }

        if(pla.killcount == 3)
        {
            pla.isSuccessed = true;
            //清理当前剑光特效
            for(int k=pla.sowrdEffectList.size()-1;k>=0;k--){
                auto & sowrd = pla.sowrdEffectList[k];
                if(sowrd.label){
                    sowrd.label->hide();
                    delete sowrd.label;
                    sowrd.label = nullptr;
                }
            }
            pla.sowrdEffectList.clear();

            if(pla.isSuccessed){
                pla.isAttacking = false;
                levelBGM->stop();
                winBGM->setPosition(0);
                winBGM->play();
                qDebug() << "打怪胜利，恭喜玩家全部关卡挑战成功！";
                QMessageBox::question(this,"通关顺利","恭喜你通过所有关卡！\n点击确定返回主界面");
                clearAll();
                backHome();
            }
        }
        // 防御屏障跟随玩家实时移动
        if(pla.isDefending && defenseLabel && player)
        {
            defenseLabel->move(player->x()-15, player->y()-10);
        }
        moveMonsterFireBullets();
    }
    updateAllSowrdEffect();
    update();
}

//回到主页面
void Widget::backHome(){

    ui->level_1->show();
    ui->level_2->show();
    ui->level_3->show();
    ui->role_btn->show();
    ui->back->hide();
    ui->start_btn->hide();
    ui->mainBg->show();
    ui->role_btn->setEnabled(true);
    winBGM->stop();
    playHomeBGM();

    this->update();
}

//更新剑光特效
void Widget::updateAllSowrdEffect()
{
    if(!player) return;
    if(pla.curRole==2||pla.curRole==3) return;
    int playerX = player->x();
    int playerY = player->y();
    int dir = pla.playerDir;

    QList<SowrdEffect> tempEffect;

    // 遍历所有剑光特效
    for(int i = 0; i < pla.sowrdEffectList.size(); i++)
    {
        SowrdEffect &ef = pla.sowrdEffectList[i];

        if(!ef.alive||!ef.label) {
            if(ef.label){
                ef.label->hide();
                delete ef.label;
                ef.label=nullptr;
            }
            continue;
        }

        // 剑光位置跟随玩家
        if(dir == 4) {
            ef.x = playerX + 90;
            ef.y = playerY;
        } else if(dir == 3) {
            ef.x = playerX - 40;
            ef.y = playerY;
        } else if(dir == 1) {
            ef.x = playerX;
            ef.y = playerY + 30;
        } else if(dir == 2) {
            ef.x = playerX;
            ef.y = playerY - 40;
        }
        else{
            ef.x = playerX;
            ef.y = playerY;
        }
        ef.label->move(ef.x, ef.y);

        // 播放多段剑光动画
        if(ef.curSowrdPic < ef.sowrdPics.size()){
            ef.label->setPixmap(ef.sowrdPics[ef.curSowrdPic]);
            ef.curSowrdPic++;
        }

        // 动画播放完清理特效
        if(ef.curSowrdPic >= ef.sowrdPics.size())
        {
            ef.alive = false;
            ef.label->hide();
            delete ef.label;
            ef.label=nullptr;
            continue;
        }

        if(pla.sowrdEffectList[i].alive)
        {
            tempEffect.append(pla.sowrdEffectList[i]);
        }
    }

    pla.sowrdEffectList = tempEffect;
}

//键盘按下事件
void Widget::keyPressEvent(QKeyEvent *event){

    if(!player) return;
    //上下左右移动
    switch(event->key()){
    case Qt::Key_W:
    case Qt::Key_Up:
        pla.up=true;
        break;
    case Qt::Key_S:
    case Qt::Key_Down:
        pla.down=true;
        break;
    case Qt::Key_A:
    case Qt::Key_Left:
        pla.left=true;
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        pla.right=true;
        break;
    case Qt::Key_J:
        if(curLevel == 1){
            return;
        }
        if (event->isAutoRepeat()) return;
        if(!pla.isAttacking&&pla.atkCd<=0){
            sowrdBGM->setPosition(0);
            sowrdBGM->play();
            pla.playerDir=3;
            pla.isAttacking = true;
            pla.attackTime=0;
            pla.playerAttackPic = 0;
            createSowrdEffect();
        }
        break;
    case Qt::Key_L:
        if(curLevel == 1){
            return;
        }
        if (event->isAutoRepeat()) return;
        if(!pla.isAttacking&&pla.atkCd<=0){
            sowrdBGM->setPosition(0);
            sowrdBGM->play();
            pla.playerDir=4;
            pla.isAttacking = true;
            pla.attackTime=0;
            pla.playerAttackPic = 0;
            createSowrdEffect();
        }
        break;
    case Qt::Key_K:
        if(curLevel != 1&& !pla.isDefending && !pla.defenseCoolDown){
            pla.isDefending = true;
            pla.defenseCoolDown = true;

            if(defenseLabel){
                defenseLabel->hide();
                delete defenseLabel;
                defenseLabel=nullptr;
            }
            // 创建防御屏障，跟随玩家位置
            defenseLabel = new QLabel(this);
            defenseLabel->setGeometry(player->x()-15, player->y()-10, 80, 180);
            defenseLabel->setPixmap(QPixmap(":/images/effect/defense.png"));
            defenseLabel->setScaledContents(true);
            defenseLabel->raise();
            defenseLabel->show();

            //使用AI----------
            QTimer::singleShot(pla.defenseLastTime, this, [=](){
                pla.isDefending = false;
                if(defenseLabel){
                    defenseLabel->hide();
                    delete defenseLabel;
                    defenseLabel=nullptr;
                }
            });

            // 冷却结束，解锁防御
            QTimer::singleShot(pla.defenseCdTime, this, [=](){
                pla.defenseCoolDown = false;
            });
            //-----------------
        }
        break;
    case Qt::Key_I:
        if(curLevel == 3 && !pla.isPlayerStealth && pla.stealthUseCount > 0 && !pla.stealthCooldown)
        {
            // 开启隐身
            pla.isPlayerStealth = true;
            pla.stealthCooldown = true;
            pla.stealthUseCount--;

            //使用AI------------
            // 玩家变透明
            player->setStyleSheet("background-color:rgba(255,255,255,0.2);");

            // 隐身持续 2000毫秒 后自动解除
            QTimer::singleShot(pla.stealthLastTime, this, [=](){
                pla.isPlayerStealth = false;
                player->setStyleSheet("");
            });

            // 冷却 5000毫秒 后才能再用
            QTimer::singleShot(pla.stealthCdTime, this, [=](){
                pla.stealthCooldown = false;
            });
            //------------------
        }
        break;
    default:
        return;
    }

    QWidget::keyPressEvent(event);
}

void Widget::keyReleaseEvent(QKeyEvent *event){

    switch(event->key()){
    case Qt::Key_W:
    case Qt::Key_Up:
        pla.up=false;
        break;
    case Qt::Key_S:
    case Qt::Key_Down:
        pla.down=false;
        break;
    case Qt::Key_A:
    case Qt::Key_Left:
        pla.left=false;
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        pla.right=false;
        break;
    default:
        break;
    }
}

//生成剑光特效
void Widget::createSowrdEffect()
{
    if(!player||pla.allSowrdEffectPics.isEmpty()||pla.curRole==2||pla.curRole==3){
        return;
    }

    SowrdEffect ef;
    ef.sowrdPics = pla.allSowrdEffectPics;
    ef.curSowrdPic = 0;
    ef.alive = true;

    // 获取玩家实时位置和方向
    int playerX = player->x();
    int playerY = player->y();
    int dir = pla.playerDir;

    // 根据玩家方向，决定剑光的初始位置
    if(dir == 4) // 向右
    {
        ef.x = playerX + 90;
        ef.y = playerY;
    }
    else if(dir == 3) // 向左
    {
        ef.x = playerX - 40;
        ef.y = playerY;
    }
    else if(dir == 1) // 向下
    {
        ef.x = playerX;
        ef.y = playerY + 30;
    }
    else if(dir == 2) // 向上
    {
        ef.x = playerX;
        ef.y = playerY - 40;
    }
    else{
        ef.x = playerX;
        ef.y = playerY;
    }

    // 创建特效标签
    ef.label = new QLabel(this);
    ef.label->setFixedSize(30, 40);
    ef.label->setScaledContents(true);
    ef.label->move(ef.x, ef.y);
    ef.label->raise();
    ef.label->show();

    pla.sowrdEffectList.append(ef);
}

void Widget::on_back_clicked()
{
    winBGM->stop();

    int back_ans=QMessageBox::question(this,"返回键","是否确认返回游戏主界面？",
                          QMessageBox::Yes|QMessageBox::No,QMessageBox::No);
    if(back_ans==QMessageBox::Yes){

        playHomeBGM();

        if(intro){
            intro->hide();
        }

        //清理
        clearAll();

        //初始化
        pla.playerPicID=0;
        pla.playerDir=0;
        pla.ismoving=false;
        pla.playerPicdown.clear();
        pla.playerPicup.clear();
        pla.playerPicleft.clear();
        pla.playerPicright.clear();
        pla.playerFightLeft.clear();
        pla.playerFightRight.clear();

        //按钮
        ui->level_1->show();
        ui->level_2->show();
        ui->level_3->show();
        ui->role_btn->show();
        ui->start_btn->hide();
        ui->back->hide();
        ui->mainBg->show();
        beginGame = false;
        ui->role_btn->setEnabled(true);

        this->update();
    }
}