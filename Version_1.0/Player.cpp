#include "Player.h"

Player::Player() {

    //初始化所有玩家数据
    curRole = 0;
    playerPicID = 0;
    playerDir = 0;
    ismoving = false;
    up = false;
    down = false;
    left = false;
    right = false;

    playermoveCount=0;

    isAttacking = false;
    playerAttackPic = 0;
    attackTime = 0;
    killcount = 0;
    isAttacked = false;
    isSuccessed = false;
    atkCd = 0;
    hurtTime=0;

    maxHP = 100;
    hp = maxHP;

    isDefending = false;
    defenseCoolDown = false;
    defenseLastTime = 2000;
    defenseCdTime = 5000;

    isPlayerStealth = false;
    stealthCooldown = false;
    stealthUseCount = 3;
    stealthLastTime = 2000;
    stealthCdTime = 5000;
}


