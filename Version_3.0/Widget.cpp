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
    //规则介绍页面
    rule = new Rule(this);
    rule->hide();
    //场景介绍页面
    beginGame=false;
    intro = new Intro(this);
    intro->hide();
    connect(intro,&Intro::startGame,this,&Widget::onstartGame);

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
    level1.loadMonsterPics();
    level1.init(this);
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

//场景一怪物特效，主定时器超时调用
void Widget::moveMonsterBullets()
{
    if(curLevel != 1) return;
    if(!player) return;
    if(pla.hp<=0) return;

    QList<MonsterBullet>& bulletlist = level1.getbulletList();
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
            QList<Level1Rand> safeZones = level1.getRandsafeZones();
            for(const Level1Rand& safeZone:safeZones){
                if (safeZone.isAlive && safeZone.label != nullptr)
                {
                    if (player->geometry().intersects(safeZone.label->geometry()))
                    {
                        insafe = true;
                        break;
                    }
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
    level1.ShootCircleBullet(this);
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
    level2.loadMonsterPics();
    level2.init(this);
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

//场景三设置
void Widget::on_level_3_clicked()
{
    //场景三怪物设置
    clearCurrentMonsters();
    level3.loadMonsterPics();
    level3.init(this);
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

//场景三怪物特效，主定时器超时调用
void Widget::moveMonsterFireBullets(){

    if(curLevel != 3) return;
    if(!player) return;
    if(pla.hp<=0) return;

    QRect playerRect(player->x(), player->y(),player->width(), player->height());

    QList<FireBullet>& firebulletlist = level3.getfirebulletList();
    for(int i = firebulletlist.size()-1; i >= 0; i--)
    {
        FireBullet &fb = firebulletlist[i];

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
            firebulletlist.removeAt(i);
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
                    firebulletlist.removeAt(i);
                    continue;
                }

                // 扣血
                pla.hp -= 5;
                if(pla.hp < 0) pla.hp = 0;
                updatePlayerHPBar();

                fb.label->hide();
                delete fb.label;
                fb.label = nullptr;
                firebulletlist.removeAt(i);

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

    level3.ShootFireBullet(this,px,py);
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

    if(curLevel==1){
        level1.clear();
    }
    else if(curLevel==2){
        level2.clear();
    }
    else if(curLevel==3){
        level3.clear();
    }

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
    QList<MonsterBullet>& bulletlist = level1.getbulletList();
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
    QList<Level1Rand> &traps = level1.getRandTraps();
    QList<Level1Rand> &safeZones = level1.getRandsafeZones();
    for(Level1Rand& trap:traps){
        if (trap.label)
        {
            trap.label->hide();
            delete trap.label;
            trap.label = nullptr;
        }
        trap.isAlive = false;
    }

    for(Level1Rand& safeZone:safeZones){
        if (safeZone.label)
        {
            safeZone.label->hide();
            delete safeZone.label;
            safeZone.label = nullptr;
        }
        safeZone.isAlive = false;
    }

    QList<FireBullet>& firebulletlist = level3.getfirebulletList();
    for(int i = 0; i < firebulletlist.size(); i++)
    {
        if(firebulletlist[i].label){
            firebulletlist[i].label->hide();
            delete firebulletlist[i].label;
            firebulletlist[i].label=nullptr;
        }
    }
    firebulletlist.clear();

    pla.isPlayerStealth = false;
    pla.stealthCooldown = false;
    pla.stealthUseCount = 3;
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

//清空当前所有怪物
void Widget::clearCurrentMonsters()
{
    if (monstermoveTimer) {
        monstermoveTimer->stop();
    }

    // 根据当前关卡，清空对应怪物列表
    if (curLevel == 1) {
        for (Monster &m : level1.getMonsters()) {
            if (m.label) {
                m.label->hide();
                m.label->setParent(nullptr);
                delete m.label;
                m.label = nullptr;
            }
        }
        level1.getMonsters().clear();
    }
    else if (curLevel == 2) {
        for (Monster &m : level2.getMonsters()) {
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
        level2.getMonsters().clear();
    }
    else if(curLevel==3){
        for (Monster &m : level3.getMonsters()) {
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
        level3.getMonsters().clear();
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
    // 根据当前关卡获取障碍列表
    QList<QRect> obstacleList;
    QList<QRect> trapList;
    QList<QRect> deadList;

    if (curLevel == 1)
    {
        obstacleList = level1.getObstacles();
        trapList = level1.getTraps();
        deadList = level1.getDeadZones();
    }
    else if (curLevel == 2)
    {
        obstacleList = level2.getObstacles();
    }
    else if (curLevel == 3)
    {
        obstacleList = level3.getObstacles();
        deadList = level3.getDeadZones();
    }
    for(const QRect&obsRect:obstacleList){
        if(playerRect.intersects(obsRect)){
            hasCollided=true;
            break;
        }
    }
    if(hasCollided){
        player->move(originalX,originalY);
        return;
    }
    for (const QRect &trap : trapList) {
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
    for (const QRect &water :deadList) {
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
            if(pla.playerAttackPic >= pla.playerFightLeft.size()){
                pla.playerAttackPic = pla.playerFightLeft.size()-1;
            }
            curPic=pla.playerFightLeft[pla.playerAttackPic];
            pla.playerAttackPic++;
        }
        else if(pla.playerDir==4){
            if(pla.playerAttackPic >= pla.playerFightRight.size()){
                pla.playerAttackPic = pla.playerFightRight.size()-1;
            }
            curPic=pla.playerFightRight[pla.playerAttackPic];
            pla.playerAttackPic++;
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
        curMonsters = &level1.getMonsters();
    else if (curLevel == 2)
        curMonsters = &level2.getMonsters();
    else if(curLevel == 3){
        curMonsters = &level3.getMonsters();
    }

    if (!curMonsters || curMonsters->isEmpty())
        return;

    //动画设置
    for(Monster &m:*curMonsters){
        updateMonsterAnim(m,curLevel);
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
        QList<Level1Rand> &traps = level1.getRandTraps();
        for(int i=traps.size()-1;i>=0;i--){
            Level1Rand &_trap = traps[i];
            _trap.lifetime++;
            if (_trap.lifetime >= 30)
            {
                _trap.lifetime = 0;

                // 清理旧陷阱
                if (_trap.label)
                {
                    _trap.label->hide();
                    delete _trap.label;
                    _trap.label = nullptr;
                }

                // 随机位置生成新陷阱
                _trap.x = 230+rand() % 450;
                _trap.y = 150+rand() % 250;

                _trap.label = new QLabel(this);
                _trap.label->setGeometry(_trap.x, _trap.y, _trap.width, _trap.height);
                _trap.label->setPixmap(QPixmap(":/images/effect/trap.png"));
                _trap.label->setScaledContents(true);
                _trap.label->show();

                _trap.isAlive = true;
            }
        }

        //安全区刷新
        QList<Level1Rand> &safeZones = level1.getRandsafeZones();
        for(int i=safeZones.size()-1;i>=0;i--){
            Level1Rand &_safeZone =  safeZones[i];
            _safeZone.lifetime++;
            if (_safeZone.lifetime >= 50)
            {
                _safeZone.lifetime = 0;

                // 清理旧安全区
                if (_safeZone.label)
                {
                    _safeZone.label->hide();
                    delete _safeZone.label;
                    _safeZone.label = nullptr;
                }

                // 随机位置生成新安全区
                _safeZone.x = 230+rand() % 450;
                _safeZone.y = 150+rand() % 250;

                _safeZone.label = new QLabel(this);
                _safeZone.label->setGeometry(_safeZone.x, _safeZone.y, _safeZone.width, _safeZone.height);
                _safeZone.label->setPixmap(QPixmap(":/images/effect/safezone.png"));
                _safeZone.label->setScaledContents(true);
                _safeZone.label->show();

                _safeZone.isAlive = true;
            }
        }

        //玩家踩陷阱扣血
        QRect playerRect(player->x(), player->y(), 50, 150);
        for(const Level1Rand&trap:traps){
            if (trap.isAlive && trap.label != nullptr)
            {
                if (playerRect.intersects(trap.label->geometry()))
                {
                    pla.hp -= 2;
                    if (pla.hp < 0) pla.hp = 0;
                    updatePlayerHPBar();
                }
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
        for(int i = 0; i < curMonsters->size(); i++)
        {

            if(pla.isSuccessed){
                return;
            }

            Monster &m = (*curMonsters)[i];

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
            for (auto &other : *curMonsters)
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
                    return;
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
            for(int i = 0; i < curMonsters->size(); i++)
            {
                if(haveHitOne) {
                    break;
                }
                Monster &m = (*curMonsters)[i];
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
                for(int i = 0; i < curMonsters->size(); i++){
                    Monster &m = (*curMonsters)[i];
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

            Monster &m = (*curMonsters)[i];

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
                for (auto &other : *curMonsters)
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
                    Monster &m = (*curMonsters)[i];
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
            for(int i = 0; i <curMonsters->size(); i++)
            {
                Monster &m = (*curMonsters)[i];
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
                for(int i = 0; i <curMonsters->size(); i++){
                    Monster &m = (*curMonsters)[i];
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
        }
        break;
    case Qt::Key_I:
        if(curLevel == 3 && !pla.isPlayerStealth && pla.stealthUseCount > 0 && !pla.stealthCooldown)
        {
            // 开启隐身
            pla.isPlayerStealth = true;
            pla.stealthCooldown = true;
            pla.stealthUseCount--;

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