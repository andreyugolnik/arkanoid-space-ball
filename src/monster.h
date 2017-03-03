/**********************************************\
*
*  Copyright (C) 2006 by Andrey A. Ugolnik
*  http://www.ugolnik.info
*  andrey@ugolnik.info
*
\**********************************************/

#pragma once

#include <SDL.h>
#include <vector>

class CMonster final
{
    friend class CGame;
public:
    CMonster();
    ~CMonster();

    enum _MONST1 { MONST_PATROL, MONST_COPTER, MONST_EYE, MONST_BLACKHOLE, MONST_FIGHTER, MONST_HAND, MONST_TURBINE, MONST_UFO, MONST_STONE1, MONST_STONE2, MONST_END };
    enum _MONST2 { M_WRECKAGE1, M_WRECKAGE2 };
    bool IsAcrossBall(SDL_Rect rc, bool bRemoveMonster);
    bool IsAcrossBall2(SDL_Rect rc);
    bool IsAcrossPaddle(int nX, int nY, int nWidth, int nHeight);
    bool IsAcrossPaddle2(int nX, int nY, int nWidth, int nHeight);
    bool IsAcross(int nX, int nY, int nWidth, int nHeight, bool bRemoveMonster);
    void Move(bool bBackWall, int nPaddleY, int nPaddleHeight);
    void AddMonster(int x, int y, int nType);
    void AddMonster2();
    void Draw();
    void Draw2();
    void RemoveAll();

private:
    void ChangeMonsterAngle(size_t idx);
    void RemoveByPos(size_t idx);

private:
    int m_nSndPatrol;
    bool m_bBackWall;
    struct sMonster
    {
        int nType;
        float x;
        float y;
        int w, h, nFrame, nFramesCount, nAngle;
        float   fSpeed; // wreckage related
        SDL_Surface* img;
        int nCaptureMode;           // MONST_HAND: 0 - none, 1 - ball captured, 2 - ball released
        int nCapturedBallIndex; // MONST_HAND: captured ball index
    };
    std::vector<sMonster>m_vecMonster2;
    std::vector<sMonster>m_vecMonster;
};
