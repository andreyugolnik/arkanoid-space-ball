/**********************************************\
*
*  Copyright (C) 2006 by Andrey A. Ugolnik
*  http://www.ugolnik.info
*  andrey@ugolnik.info
*
\**********************************************/

#pragma once

#include "generatelevel.h"

#include <SDL.h>
#include <vector>

class CGame final
{
    friend class CGenerateLevel;
    friend class CBall;
    friend class CBullet;
    friend class CMonster;

public:
    CGame(const sOptions& options);
    ~CGame();

    enum { GS_GAME_ACTIVE, /*GS_GAME_RESTORE, */GS_GAME_PAUSED, GS_GET_READY, GS_GAME_OVER, GS_GAME_AREYOUSURE_EXIT, GS_GAME_REMINDER1, GS_GAME_REMINDER2 };
    enum _RACKET_TYPE { RT_NORMAL, RT_LASER, RT_MISSILE, RT_PLASMA, RT_MAGNET, RT_ENGINE };
    void SetPause();
    bool DrawReminder(int nReminderType);
    void AddScore(int nScore);
    void DoImpact(int nIndex, bool bRemoveAll);
    //void DoImpact2(int nBx, int nBy, int nBrick, bool bRemoveAll);
    void ProcessBonus(int nBonusType);
    int getScore() const
    {
        return m_nScore;
    }
    int getLevel() const
    {
        return m_nCurrentLevel;
    }
    bool DrawScreen();
    void InitNewGame(bool bIsCustomLevels);
    void Unload();
    bool DrawGetReady();
    void AddGetReeadyInfo(const char* pchString);
    void RemoveOneLives();
    void DrawBackground();
    void RestoreGame();
    void SendEsc();
    void LoadBackground();
    void FreeBackground();

private:
    void DrawPause();
    void _Cheater();
    void ChangeBrick(int nIndex, Uint8 byToBrickType, bool bRemoveAll = false);
    void DoShoot();
    void DrawStatistic();
    bool DoGameOver();
    void DoGameActive();
    void DrawPaddle();
    void InitLevel(int nLevel, bool bRestore = false);
    void DoBomb(int nIndex);
    void DrawBrick(int nIndex, bool bNextFrame, bool bNextFrameAnim);
    void DrawBricks();
    void ResetAll();
    void DrawBrickBullets();
    void MoveBrickBullets();
    int CalcBrickBulletsAngle(int nIndex, int nX, int nY);
    bool IsEmptyBrickPos(const int nSkipPos, const int nX, const int nY);
    void DrawAreYouSure();
    //void DrawRestoreGame();

private:
    const sOptions& m_options;
    int m_nLaserX;
    std::vector<_BRICK>m_vecLevelBricks;
    int m_nLevelPrev;   // store previous level number: -1 - usualy, -2 - inform, that we restore game
    struct _STARS
    {
        float   fX, fY, fSpeed;
        int nType, nFrame;
    };
    std::vector<_STARS>m_vecStars;
    int m_nBonusLevelType;
    int m_nBonusLevelTicks;
    int m_nSndWall; // store channel number for stop
    CGenerateLevel m_GenLev;
    Uint32 m_dwUnregisterdTime;
    int m_nScoreToAdditionalBall;
    Uint8 m_byChar;
    struct _BRICK_BULLET
    {
        int     nType;
        int     nAngle;
        int     nFrame;
        float   fX, fY;
    };
    std::vector<_BRICK_BULLET>m_vecBrickBullets;
    int m_nTotalBonusesInLevel;
    int m_nCanMovePaddleCount;
    bool m_bPaddleIsInvert;
    int m_nPaddleInvertCount;
    bool m_bCanMovePaddle;
    int m_nCountSelfDestruct;
    bool m_bSelfDestructStarted;
    int m_nCountBackWall;
    bool m_bBackWall;
    int m_nAddNewMonsterCount;
    int m_nSlowDownCount;
    bool m_bSlowDown;
    int m_nClockFrame;
    int m_nLives;
    int m_nGameState;
    float m_nRacketX;
    float m_nRacketY;
    int m_nRacketSize;
    int m_nRacketType;
    int m_nPaddleMissileCount;
    int m_nScore;
    int m_nBrickCount;
    int m_nCurrentLevel;
    int m_nLevelImage;
    int m_nGetReadyScore;
    int m_nGetReadyMonsters;
    int m_nGetReadyBonuses;
    int m_nGetReadyBricks;
    int m_nGetReadyBallsLose;
    Uint32 m_dwCombosTime;
    int m_nCombosBricks;
    char* m_pchGetReeadyInfo;
    bool m_bTutorialPause;
    bool m_bIsCustomLevels;
};
