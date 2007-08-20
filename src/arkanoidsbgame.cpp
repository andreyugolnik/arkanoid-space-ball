// Krakout2Game.cpp: implementation of the CArkanoidSBGame class.
//
//////////////////////////////////////////////////////////////////////

#include "arkanoidsb.h"
#include "arkanoidsbgame.h"

// paddle size can be small (50px), default (80px), expanded (110px), big (140px)
#define PADDLE_HEIGHT	(50 + m_nRacketSize * 30)
#define SCORE_TO_ADDITIONAL_BALL	5000
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CArkanoidSBGame::CArkanoidSBGame() {
	m_nCurrentLevel		= 0;
	m_nLevelImage		= -1;
	m_nBonusLevelType	= -1;
	m_nSndWall			= -1;
	m_dwUnregisterdTime	= 0;
	m_nGameState		= -1;
	m_pchGetReeadyInfo	= 0;
	m_bIsCustomLevels	= false;
	m_vecLevelBricks.reserve(BRICKS_WIDTH * BRICKS_HEIGHT);
}

CArkanoidSBGame::~CArkanoidSBGame() {
	delete[]	m_pchGetReeadyInfo;
}

void CArkanoidSBGame::InitNewGame(bool bIsCustomLevels) {
	m_bIsCustomLevels	= bIsCustomLevels;
	m_bTutorialPause	= false;
	m_nCurrentLevel		= 0;
	m_nLevelPrev		= -1;
	m_nScore			= 0;
	m_nLives			= 3;
	m_nScoreToAdditionalBall	= SCORE_TO_ADDITIONAL_BALL;
	g_Bonus.RemoveAll(true);
	//EnableMusic();

	InitLevel(m_nCurrentLevel);
	//g_pMainFrame->m_pImix->SamplePlay(m_nSampleGetReady);
}

void CArkanoidSBGame::DoGameActive() {
	if(m_bTutorialPause == true) {
		return;
	}
	if(IsKeyPressed(SDLK_p) && IsKeyStateChanged(SDLK_p)) {
		SetPause();
	}

	SDL_Rect	rc;
	Uint32	dwTime	= SDL_GetTicks();
	static Uint32	dwSecondTime	= 0;
	if(dwSecondTime + 1000 < dwTime) {
		dwSecondTime	= dwTime;
		if(m_nBonusLevelType == -1) {
			static int	nCountCopter	= 30;
			if(m_nCurrentLevel % 4 != 0) {	// each 4 level only meteors
				if(--nCountCopter == 0) {
					nCountCopter	= 50 + g_Rnd.Get(30);
					g_Monster.AddMonster(50, (int)m_nRacketY + (PADDLE_HEIGHT - 48) / 2, CMonster::MONST_PATROL);
				}
			}
			if(++m_nAddNewMonsterCount == 10) {
				m_nAddNewMonsterCount	= 0;
				if(m_nCurrentLevel % 4 == 0) {	// each 4 level only meteors
					g_Monster.AddMonster(100 + g_Rnd.Get(200), 100 + g_Rnd.Get(320), CMonster::MONST_STONE1 + g_Rnd.Get(2));
				}
				else {
					g_Monster.AddMonster(100 + g_Rnd.Get(200), 100 + g_Rnd.Get(320), CMonster::MONST_COPTER + g_Rnd.Get(CMonster::MONST_END - 1));
				}
			//for(int a = 0; a < CMonster::MONST_END; a++) {
			//	g_Monster.AddMonster(100 + g_Rnd.Get(200), 100 + g_Rnd.Get(320), a);
			//}
			}
			if(m_bSlowDown == true && --m_nSlowDownCount == 0) {
				m_bSlowDown	= false;
			}
			if(m_bCanMovePaddle == false && --m_nCanMovePaddleCount == 0) {
				m_bCanMovePaddle	= true;
			}
			if(m_bPaddleIsInvert == true &&--m_nPaddleInvertCount == 0){
				m_bPaddleIsInvert	= false;
			}
			if(m_bSelfDestructStarted == true && m_nCountSelfDestruct > 0) {
				m_nCountSelfDestruct--;
			}
			if(m_bBackWall == true && --m_nCountBackWall == 0) {
				StopSound(m_nSndWall);
				m_bBackWall	= false;
			}
		}
		else if(m_nBonusLevelType == 1) {
			m_nBonusLevelTicks--;
			g_Monster.AddMonster2();
		}
	}

	if(m_bCanMovePaddle == true) {
		const double	fMultipler	= 1.1;// * g_fSpeedCorrection;
		if(m_bPaddleIsInvert == false) {
			m_nRacketY	+= g_nMouseDY * fMultipler;
			if(m_nRacketType == RT_ENGINE)	m_nRacketX	+= g_nMouseDX * fMultipler;
		}
		else {
			m_nRacketY	-= g_nMouseDY * fMultipler;
			if(m_nRacketType == RT_ENGINE)	m_nRacketX	-= g_nMouseDX * fMultipler;
		}
	}
	m_nRacketY	= max(m_nRacketY, WALL_Y1);
	m_nRacketY	= min(m_nRacketY, WALL_Y2 - PADDLE_HEIGHT);
	m_nRacketX	= max(m_nRacketX, WALL_X1 + 50);
	m_nRacketX	= min((m_nRacketType == RT_ENGINE ? m_nRacketX : RACKET_X), RACKET_X + 5);
	static Uint32	dwRacketTime	= 0;
	if(m_nRacketX > RACKET_X) {
		if(dwRacketTime + 15 < SDL_GetTicks()) {
			dwRacketTime	= SDL_GetTicks();
			m_nRacketX--;
		}
	}

	if(m_nBonusLevelType == -1) {
 		if(m_bIsCustomLevels == true) {
// 			if(IsKeyPressed(SDLK_n) && IsKeyStateChanged(SDLK_n)) {
// 				m_vecLevelBricks.clear();
// 				//InitLevel(++m_nCurrentLevel);
// 			}
 		}
		if(m_nBrickCount == 0) {
			g_CoolString.Add("you have destroyed all bricks!");
			g_TutorialDlg.AddDialog(WALL_X2, WALL_Y1, 0, 4);
			if(g_Exploision.GetCount() == 0 && (g_bAutoBonusMode == true || g_Bonus.GetBonusesOnScreen() == 0)/* && g_CoolString.GetCount() == 0*/) {
				AddGetReeadyInfo("You have destroyed all bricks!");
				InitLevel(++m_nCurrentLevel);
			}
		}
		// max number of bricks before self destruction
		else if(m_nBrickCount <= 5 && m_bSelfDestructStarted == false && m_nCountSelfDestruct != -1) {
			g_CoolString.Add("timer activated");
			g_TutorialDlg.AddDialog(WALL_X2, WALL_Y1, 0, 1);
			m_bSelfDestructStarted	= true;
			m_nCountSelfDestruct	= 20;
		}
		if(g_Ball.GetBallsCount() == 0) {
			AddGetReeadyInfo("You lost balls");
			RemoveOneLives();
		}
		if(g_bMouseLB == true) {
			g_bMouseLB	= false;
			DoShoot();
			g_Ball.StartBall((int)m_nRacketY, (int)m_nRacketY + PADDLE_HEIGHT);
		}
		if(g_bAutoBonusMode == false && g_bMouseRB == true) {
			g_bMouseRB	= false;
			ProcessBonus(g_Bonus.GetBonusFromStack());
		}
		// correct it after paddle, because it should move as before
		if(m_bSlowDown == true) {
			g_fSpeedCorrection	*= .4;
		}

		rc.x	= (int)m_nRacketX;
		rc.y	= (int)m_nRacketY;
		rc.h	= PADDLE_HEIGHT;
		//if(IsKeyPressed(SDLK_n) && IsKeyStateChanged(SDLK_n) || IsKeyPressed(SDLK_m)) {
		if(m_nBrickCount > 0) {	// don't move balls if no bricks on screen
			int	nPx	= (int)m_nRacketX;
			m_nGetReadyBallsLose	+= g_Ball.Move(m_bBackWall, rc, m_nRacketType, nPx);
			m_nRacketX	= nPx;
		}
		g_Bullet.Move();
		g_Monster.Move(m_bBackWall, (int)m_nRacketY, PADDLE_HEIGHT);
		MoveBrickBullets();
		g_EnergyHole.Move();
		g_Bonus.Move(m_nRacketType);

		int	nBonusType;
		if((nBonusType = g_Bonus.IsAcross((int)m_nRacketX, (int)m_nRacketY, 12, PADDLE_HEIGHT)) != -1) {
			if(g_bAutoBonusMode == true)
				ProcessBonus(nBonusType);
			else
				g_Bonus.AddToStack(nBonusType);
		}

		if(true == g_Monster.IsAcrossPaddle((int)m_nRacketX, (int)m_nRacketY, 12, PADDLE_HEIGHT)) {
			AddScore(15);
		}

		for(int i = 0; i < m_vecBrickBullets.size(); i++) {
			if(m_vecBrickBullets[i].fX + 10 > m_nRacketX && m_vecBrickBullets[i].fX < m_nRacketX + 12 && m_vecBrickBullets[i].fY + 10 > m_nRacketY && m_vecBrickBullets[i].fY < m_nRacketY + PADDLE_HEIGHT) {
				g_Exploision.AddExploision((int)m_vecBrickBullets[i].fX - 12, (int)m_vecBrickBullets[i].fY - 10, 0);
				if(m_vecBrickBullets[i].nType == 0) {
					m_bPaddleIsInvert			= true;
					m_nPaddleInvertCount		+= 3;
				}
				else {
					m_bCanMovePaddle			= false;
					m_nCanMovePaddleCount	= 3;
				}
				swap(m_vecBrickBullets[i], m_vecBrickBullets.back());
				m_vecBrickBullets.resize(m_vecBrickBullets.size() - 1);
				i--;
			}
		}

		int	nPos;
		g_Bullet.GetPositionAndSize(rc, nPos, true);
		while(g_Bullet.GetPositionAndSize(rc, nPos, false) == true) {
			if(true == g_Monster.IsAcross(rc.x, rc.y, rc.w, rc.h, true)) {
				AddScore(25);
			}
			for(int i = 0; i < m_vecBrickBullets.size(); i++) {
				if(m_vecBrickBullets[i].fX + 10 > rc.x && m_vecBrickBullets[i].fX < rc.x + rc.w && m_vecBrickBullets[i].fY + 10 > rc.y && m_vecBrickBullets[i].fY < rc.y + rc.h) {
					g_Exploision.AddExploision((int)m_vecBrickBullets[i].fX - 12, (int)m_vecBrickBullets[i].fY - 10, 0);
					swap(m_vecBrickBullets[i], m_vecBrickBullets.back());
					m_vecBrickBullets.resize(m_vecBrickBullets.size() - 1);
					i--;
				}
			}
		}

		nPos	= 0;
		while(g_Ball.GetPositionAndDiameter(rc, nPos) == true) {
			if(rc.h != CBall::TYPE_BLUE) {
				for(int i = 0; i < m_vecBrickBullets.size(); i++) {
					if(g_Ball.IsThisBallOverObject(nPos - 1, (int)m_vecBrickBullets[i].fX, (int)m_vecBrickBullets[i].fY, 10, 10) > 0) {
						g_Ball.ChangeBallAngle(nPos - 1, false);
						AddScore(15);
						g_Exploision.AddExploision((int)m_vecBrickBullets[i].fX - 12, (int)m_vecBrickBullets[i].fY - 10, 0);
						swap(m_vecBrickBullets[i], m_vecBrickBullets.back());
						m_vecBrickBullets.resize(m_vecBrickBullets.size() - 1);
					}
				}
				if(true == g_Monster.IsAcrossBall(rc, true)) {
					g_Ball.ChangeBallAngle(nPos - 1, true);
				}
			}
		}
	}
	else if(m_nBonusLevelType == 0) {
		rc.x	= (int)m_nRacketX;
		rc.y	= (int)m_nRacketY;
		rc.h	= PADDLE_HEIGHT;
		int	nPx	= (int)m_nRacketX;
		m_nGetReadyBallsLose	+= g_Ball.Move(m_bBackWall, rc, m_nRacketType, nPx);
		m_nRacketX	= nPx;
		if(m_nBrickCount == 0 && g_Exploision.GetCount() == 0 && g_CoolString.GetCount() == 0) {
			AddGetReeadyInfo("All cannons destroyed, bonus 3000 score!");
			AddScore(3000);
			InitLevel(++m_nCurrentLevel);
		}
		if(m_nGetReadyBallsLose == 5) {
			AddGetReeadyInfo("You lost 5 balls, no bonus score");
			PlaySound(99);//g_pMainFrame->m_pImix->SamplePlay(m_nSampleGetReady);
			InitLevel(++m_nCurrentLevel);
		}
	}
	else if(m_nBonusLevelType == 1) {
		if(g_bMouseLB == true) {
			g_bMouseLB	= false;
			g_Ball.StartBall((int)m_nRacketY, (int)m_nRacketY + PADDLE_HEIGHT);
		}
		rc.x	= (int)m_nRacketX;
		rc.y	= (int)m_nRacketY;
		rc.h	= PADDLE_HEIGHT;
		int	nPx	= (int)m_nRacketX;
		m_nGetReadyBallsLose	+= g_Ball.Move(m_bBackWall, rc, m_nRacketType, nPx);
		m_nRacketX	= nPx;
		g_Monster.Move(m_bBackWall, (int)m_nRacketY, PADDLE_HEIGHT);
		if(true == g_Monster.IsAcrossPaddle2((int)m_nRacketX, (int)m_nRacketY, 12, PADDLE_HEIGHT) || g_Ball.GetBallsCount() == 0) {
			AddGetReeadyInfo("Your paddle destroyed, no bonus score");
			PlaySound(99);//g_pMainFrame->m_pImix->SamplePlay(m_nSampleGetReady);
			InitLevel(++m_nCurrentLevel);
		}
		// check intercept monsters and ball
		int	nPos	= 0;
		while(g_Ball.GetPositionAndDiameter(rc, nPos) == true) {
			if(true == g_Monster.IsAcrossBall2(rc)) {
				g_Ball.ChangeBallAngle(nPos - 1, true);
			}
		}
		if(m_nBonusLevelTicks == 0) {
			AddGetReeadyInfo("You are strong, bonus 3000 score!");
			AddScore(3000);
			InitLevel(++m_nCurrentLevel);
		}
	}
}

bool CArkanoidSBGame::DrawScreen() {
	bool	bIsExit			= false;

	DrawBackground();

	// we should add combos if timeout
	if(m_dwCombosTime + 300 < SDL_GetTicks()) {
		if(m_nCombosBricks > 5) {
			g_CoolString.Add("combos %d", m_nCombosBricks);
			AddScore(m_nCombosBricks * 20);
		}
		m_nCombosBricks	= 0;
	}

	DrawStatistic();
	if(m_nBonusLevelType == -1) {
		g_EnergyHole.Draw();
		//g_Monster.Draw2();
		DrawBricks();
		g_Exploision.Draw(true);
		DrawBrickBullets();
		g_Bullet.Draw();
		DrawPaddle();
		g_Ball.Draw(m_nRacketType);
		g_Monster.Draw();
		if(m_bBackWall == true) {
			SDL_Rect	rc;
			static Uint32	dwTime	= 0;
			static int		nFrame	= 0;
			if(dwTime + 50 < SDL_GetTicks()) {
				dwTime	= SDL_GetTicks();
				nFrame++;
				nFrame	%= 10;
			}
			rc.x	= nFrame * 40;	rc.y	= 0;
			rc.w	= 40;				rc.h	= 350;
			Blit(600, 90, m_pGameWall, &rc);
		}
		g_Bonus.Draw();
	}
	else if(m_nBonusLevelType == 0) {
		g_Exploision.Draw(true);
		DrawBricks();
		DrawPaddle();
		g_Ball.Draw(m_nRacketType);
	}
	else if(m_nBonusLevelType == 1) {
		g_Exploision.Draw(true);
		DrawPaddle();
		g_Ball.Draw(m_nRacketType);
		g_Monster.Draw2();
	}
	g_Exploision.Draw(false);
	g_CoolString.Draw();

	switch(m_nGameState) {
// 	case GS_GAME_RESTORE:
// 		DrawRestoreGame();
// 		break;

	case GS_GAME_ACTIVE:
		//if(IsKeyPressed(SDLK_g) && IsKeyStateChanged(SDLK_g))	m_GenLev.Generate(++m_nCurrentLevel, false);
#ifndef __linux__
		// unregistered version
		if(g_bIsRegistered == false) {
			if(m_dwUnregisterdTime < SDL_GetTicks()) {
				m_dwUnregisterdTime	= SDL_GetTicks() + 1000;
				if(m_bTutorialPause == false && g_nUnregisterdCount > 0) g_nUnregisterdCount--;
			}
			if(g_nUnregisterdCount == 20 * 60 || g_nUnregisterdCount == 10 * 60 ||
				g_nUnregisterdCount ==  3 * 60 || g_nUnregisterdCount ==  1 * 60) {
				EnableCursor(true);
				m_nGameState			= GS_GAME_REMINDER1;
			}
			else if(g_nUnregisterdCount == 0) {
				EnableCursor(true);
				m_nGameState			= GS_GAME_REMINDER2;
			}
		}
#endif
		if(IsKeyPressed(SDLK_ESCAPE) && IsKeyStateChanged(SDLK_ESCAPE)) {
			SendEsc();
		}
#ifdef _DEBUG
		if(IsKeyPressed(SDLK_w) && IsKeyStateChanged(SDLK_w)) {
			ProcessBonus(CBonus::TYPE_WALL);
		}
		if(IsKeyPressed(SDLK_r) && IsKeyStateChanged(SDLK_r)) {
			static int	nType	= 0;
			ProcessBonus(CBonus::TYPE_PADDLE_MAGNET + nType);
			nType++;	nType	%= 7;
		}
		if(g_dwModState & KMOD_SHIFT && IsKeyPressed(SDLK_s) && IsKeyStateChanged(SDLK_s)) {
			ProcessBonus(CBonus::TYPE_BALL_EXPAND);
		}
		if(!(g_dwModState & KMOD_SHIFT) && IsKeyPressed(SDLK_s) && IsKeyStateChanged(SDLK_s)) {
			ProcessBonus(CBonus::TYPE_BALL_SHRINK);
		}
		if(IsKeyPressed(SDLK_b) && IsKeyStateChanged(SDLK_b)) {
			g_Ball.AddBall(m_nRacketX - 20, (int)m_nRacketY + (PADDLE_HEIGHT - 20) / 2);
		}
		if(IsKeyPressed(SDLK_o) && IsKeyStateChanged(SDLK_o)) {
			g_Ball.SetAllBallsToBlue();
		}
#endif
		DoGameActive();
		m_bTutorialPause	= g_TutorialDlg.ShowDialog();
		break;

	case GS_GET_READY:
		if(true == DrawGetReady() || (IsKeyPressed(SDLK_ESCAPE) && IsKeyStateChanged(SDLK_ESCAPE)) || g_bMouseRB == true) {
			SendEsc();
		}
		break;

	case GS_GAME_PAUSED:
		DrawPause();
		break;

	case GS_GAME_AREYOUSURE_EXIT:
		DrawAreYouSure();
		break;

	case GS_GAME_OVER:
		bIsExit	= DoGameOver();
		break;

#ifndef __linux__
		case GS_GAME_REMINDER1:
		switch(g_ReminderDlg.Draw(0)) {
		case 1:
			g_MainMenu.SetMenuType(CMainMenu::MT_REG_KEY);
			g_nGameMode		= APPS_MAINMENU;
			break;
		case 2:
			EnableCursor(false);
			m_nGameState	= GS_GAME_ACTIVE;
			break;
		}
		break;
	case GS_GAME_REMINDER2:
		switch(g_ReminderDlg.Draw(1)) {
		case 1:
			g_MainMenu.SetMenuType(CMainMenu::MT_REG_KEY);
			g_nGameMode		= APPS_MAINMENU;
			break;
		case 2:
			g_nGameMode		= APPS_SHOULDGETNAME;
			break;
		}
		break;
#endif
	}

#ifndef __linux__
	if(g_bIsRegistered == false && m_bTutorialPause == false &&
		(m_nGameState != GS_GAME_REMINDER1 && m_nGameState != GS_GAME_REMINDER2)) {
		g_Font.DrawString(0, WALL_Y1 + 30, "Unregistered version", CMyString::FONT_ALIGN_CENTER);
	}
#endif
	
	return	bIsExit;
}

bool CArkanoidSBGame::DoGameOver() {
#define	GO_ITEM_X	((SCREEN_WIDTH - 124) / 2)
#define	GO_ITEM_Y	((SCREEN_HEIGHT - 15) / 2)

	FadeScreen();

	char	achBuf[50];
	g_Font2.DrawString(0, (SCREEN_HEIGHT - 15) / 2 - 10, "GAME OVER", CMyString::FONT_ALIGN_CENTER);
	sprintf(achBuf, "Your achieve %d level, and gain %d score points", m_nCurrentLevel + 1, m_nScore);
	g_Font.DrawString(0, (SCREEN_HEIGHT - 15) / 2 + 20, achBuf, CMyString::FONT_ALIGN_CENTER);

	bool	bIsOver	= g_MainMenu.DrawMenuButton(GO_ITEM_X, GO_ITEM_Y + 40, CMainMenu::B_OK);
	if(g_bMouseRB == true || (g_bMouseLB == true && true == bIsOver) || (IsKeyPressed(SDLK_ESCAPE) && IsKeyStateChanged(SDLK_ESCAPE))) {
		SendEsc();
		return true;
	}

	return false;
}

void CArkanoidSBGame::DrawBricks() {
	Uint32			dwTime	= SDL_GetTicks();
	static Uint32	dwTimeCountToDie	= 0;
	bool				bCountDown		= false;
	if(dwTimeCountToDie + 300 < dwTime) {
		dwTimeCountToDie	= dwTime;
		bCountDown			= true;
	}

	m_nBrickCount	= 0;
	static Uint32	dwBrickAnimTime	= 0;
	bool				bBrickAnimate		= false;
	if(dwBrickAnimTime + 15 < dwTime) {
		dwBrickAnimTime	= dwTime;
		bBrickAnimate		= true;
	}
	static Uint32	dwBrickAnimTime2	= 0;
	bool				bBrickAnimate2		= false;
	if(dwBrickAnimTime2 + 50 < dwTime) {
		dwBrickAnimTime2	= dwTime;
		bBrickAnimate2		= true;
	}

	static Uint32	dwTimeDirChange	= 0;
	bool				bChangeDir			= false;
	if(dwTimeDirChange + 100 < dwTime) {
		dwTimeDirChange	= dwTime;
		bChangeDir		= true;
	}

	bool	bSecondOut	= false;
	static Uint32	dwTimeSecond	= 0;
	if(dwTimeSecond + 1000 < dwTime) {
		dwTimeSecond	= dwTime;
		bSecondOut		= true;
	}

	for(int i = 0; i < m_vecLevelBricks.size(); i++) {
		int	nX	= (int)m_vecLevelBricks[i].fX;
		int	nY	= (int)m_vecLevelBricks[i].fY;
		DrawBrick(i, bBrickAnimate, bBrickAnimate2);
		// remove brick from list
		if(m_vecLevelBricks[i].byType == BOX_NONE) {
			swap(m_vecLevelBricks[i--], m_vecLevelBricks.back());
			m_vecLevelBricks.resize(m_vecLevelBricks.size() - 1);
			continue;
		}

		// movable bricks
		if(m_vecLevelBricks[i].byType >= BOX_MOV_0 && m_vecLevelBricks[i].byType < BOX_SHOOT_0) {
			double	fSpeed	= g_fSpeedCorrection * .5;
			do {
				int	nY	= int(m_vecLevelBricks[i].fY);
				if(m_vecLevelBricks[i].bDir == true) {
					if(nY + BRICK_H < WALL_Y2 && IsEmptyBrickPos(i, nX, nY + BRICK_H) == true) {
						m_vecLevelBricks[i].fY	+= 1;
					}
					else {
						fSpeed	= 0;
						if(m_vecLevelBricks[i].nDirChangeCount == 0)	m_vecLevelBricks[i].nDirChangeCount	= 10;
					}
				}
				else {
					if(nY > WALL_Y1 && IsEmptyBrickPos(i, nX, nY - BRICK_H) == true) {
						m_vecLevelBricks[i].fY	-= 1;
					}
					else {
						fSpeed	= 0;
						if(m_vecLevelBricks[i].nDirChangeCount == 0)	m_vecLevelBricks[i].nDirChangeCount	= 10;
					}
				}
				fSpeed	-= 1;
			} while(fSpeed > 0.0f);

			// only 0-type brick can reverse
			if(m_vecLevelBricks[i].byType == BOX_MOV_0 && bChangeDir == true && m_vecLevelBricks[i].nDirChangeCount > 0) {
				m_vecLevelBricks[i].nDirChangeCount--;
				if(m_vecLevelBricks[i].nDirChangeCount == 0) {
					m_vecLevelBricks[i].bDir	= !m_vecLevelBricks[i].bDir;
				}
			}
		}
		//shooting bricks
		else if(m_nGameState == GS_GAME_ACTIVE && m_vecLevelBricks[i].byType >= BOX_SHOOT_0 && m_vecLevelBricks[i].byType < BOX_BOMB_0) {
			if(true == bSecondOut && m_vecLevelBricks[i].nCountToShoot > 0) {
				m_vecLevelBricks[i].nCountToShoot--;
				if(m_vecLevelBricks[i].nCountToShoot == 0) {
					if(m_nBonusLevelType == -1) {
						_BRICK_BULLET	bb;
						bb.fX			= m_vecLevelBricks[i].fX + (BRICK_W - 20) / 2;	// 20 - diameter of bullet
						bb.fY			= m_vecLevelBricks[i].fY + (BRICK_H - 20) / 2;
						bb.nFrame	= 0;
						if(m_vecLevelBricks[i].byType == BOX_SHOOT_0) {	// do shoot on paddle
							g_TutorialDlg.AddDialog((int)bb.fX + 10, (int)bb.fY + 10, 0, 0);
							m_vecLevelBricks[i].nCountToShoot	= 30 + g_Rnd.Get(20);
							bb.nType		= 0;
							bb.nAngle	= CalcBrickBulletsAngle(i, (int)m_nRacketX, (int)m_nRacketY + PADDLE_HEIGHT / 2);
						}
						else {		// do shoot on ball
							g_TutorialDlg.AddDialog((int)bb.fX + 10, (int)bb.fY + 10, 0, 3);
							m_vecLevelBricks[i].nCountToShoot	= 20 + g_Rnd.Get(30);
							bb.nType		= 1;
							bb.nAngle	= 90;
							SDL_Rect	rc;
							int	nPos = 0;
							while(g_Ball.GetPositionAndDiameter(rc, nPos) == true) {
								if(rc.h != CBall::TYPE_BLUE) {
									bb.nAngle	= CalcBrickBulletsAngle(i, rc.x + rc.w / 2, rc.y + rc.w / 2);
									break;
								}
							}
						}
						m_vecBrickBullets.push_back(bb);
					}
					else if(m_nBonusLevelType == 0) {
						m_vecLevelBricks[i].nCountToShoot	= 5;
						g_Ball.AddBall(nX + BRICK_W, nY + 5, 25 + g_Rnd.Get(130)); // angle from 25 to 155
					}
				}
			}
		}
		else if(m_vecLevelBricks[i].byType >= BOX_BOMB_0) {
			if(true == bCountDown && m_vecLevelBricks[i].nCountToDie > 0) {
				m_vecLevelBricks[i].nCountToDie--;
				if(m_vecLevelBricks[i].nCountToDie == 0) {
					DoBomb(i);
				}
			}
		}

		if(m_nBonusLevelType == -1) {
			// set it to bomb
			if(m_bSelfDestructStarted == true && m_nCountSelfDestruct == 0) {
				g_CoolString.Add("bricks armed");
				m_vecLevelBricks[i].byType		= BOX_BOMB_0 + g_Rnd.Get(BOX_END - BOX_BOMB_0);
				m_vecLevelBricks[i].nCountToDie	= 5;
			}
			if(m_nGameState == GS_GAME_ACTIVE) {
				//CalculateLaserX(nX, nY, BRICK_W, BRICK_H);
				bool	bRemove;
				int	nType;
				if(g_Bullet.IsAcross(nX, nY, BRICK_W, BRICK_H, bRemove, nType) == true) {
					if(nType != CBullet::TYPE_MISSILE) {
						DoImpact(i, bRemove);
					}
					else {
						DoBomb(i);
					}
				}
			}
			// calculate bricks count
			if(m_vecLevelBricks[i].byType < BOX_IM_0 || m_vecLevelBricks[i].byType >= BOX_BOMB_0) {
				m_nBrickCount++;
			}
		}
		else if(m_nBonusLevelType == 0) {
			m_nBrickCount++;
		}
	}
	// disable self destruct while we already turn all bricks to bombs
	if(m_bSelfDestructStarted == true && m_nCountSelfDestruct == 0) {
		m_nCountSelfDestruct	= -1;
		m_bSelfDestructStarted	= false;
	}
/*	static int	nBr	= -1;
	if(m_nBrickCount != nBr) {
		nBr	= m_nBrickCount;
		printf("Bricks %d\n", m_nBrickCount);
	}*/
}

void CArkanoidSBGame::DrawBrick(int nIndex, bool bNextFrame, bool bNextFrameAnim) {
	SDL_Rect	src;
	int	nType	= m_vecLevelBricks[nIndex].byType;
	int	nX		= (int)m_vecLevelBricks[nIndex].fX;
	int	nY		= (int)m_vecLevelBricks[nIndex].fY;
	static int		nFrame	= 0;
	static Uint32	dwTime	= 0;
	static bool		bDir		= true;
	if(dwTime + 80 < SDL_GetTicks()) {
		dwTime	= SDL_GetTicks();
		nFrame++;	nFrame	%= 5;
		if(!nFrame)	bDir	= !bDir;
	}

	if(nType < BOX_40) {
		src.y	= ((nType - 1) / 10) * BRICK_H;
	}
	else if(nType < BOX_DBL_0) {
		if(bDir || nType == BOX_49)
			src.y	= ((nType - 1) / 10 + nFrame) * BRICK_H;
		else
			src.y	= ((nType - 1) / 10 + 4 - nFrame) * BRICK_H;
	}
	else {
		src.y	= ((nType - 1) / 10 + 4) * BRICK_H;
	}
	src.x	= ((nType - 1) % 10) * BRICK_W;
	src.w	= BRICK_W;
	src.h	= BRICK_H;
	Blit(nX, nY, m_pBricks, &src);
	if(m_vecLevelBricks[nIndex].nAnimateType != -1) {
		src.x	= m_vecLevelBricks[nIndex].nAnimateType * 20;
		src.y	= m_vecLevelBricks[nIndex].nFrame * 30;
		if(bNextFrameAnim == true) {
			if(++m_vecLevelBricks[nIndex].nFrame == 10) {
				m_vecLevelBricks[nIndex].nAnimateType	= -1;
			}
		}
		Blit(nX, nY, m_pExploision, &src);
	}
}

void CArkanoidSBGame::DoBomb(int nIndex) {
	int	nX			= (int)m_vecLevelBricks[nIndex].fX;
	int	nY			= (int)m_vecLevelBricks[nIndex].fY;
	Uint8	byType	= m_vecLevelBricks[nIndex].byType;
	g_TutorialDlg.AddDialog(nX + BRICK_W / 2, nY + BRICK_H / 2, 0, 6);

	for(int i = 0; i < m_vecLevelBricks.size(); i++) {
		int	x	= (int)m_vecLevelBricks[i].fX;
		int	y	= (int)m_vecLevelBricks[i].fY;
		if(abs(nX - x) <= BRICK_W && abs(nY - y) <= BRICK_H) {
			if(m_vecLevelBricks[i].byType >= BOX_BOMB_0 && m_vecLevelBricks[i].nCountToDie != 0) {
				m_vecLevelBricks[i].nCountToDie	= 1;
				m_vecLevelBricks[i].byType			= byType;
			}
			else {
				AddScore(5);
				ChangeBrick(i, BOX_NONE, true);
				g_Exploision.AddExploision(x - 12, y - 5, 0);
			}
		}
	}
}

void CArkanoidSBGame::InitLevel(int nLevel, bool bRestore) {
	int	i;
	m_vecLevelBricks.clear();
	g_CoolString.Clear();
	//g_TutorialDlg.Reset();
	m_nCurrentLevel			= nLevel;
	m_nGameState			= GS_GET_READY;
	m_bSelfDestructStarted	= false;
	m_nCountSelfDestruct	= 0;
	EnableCursor(true);

	char	achBuf[PATH_MAX];
	FILE	*pFile;
	sprintf(achBuf, "%ssave", g_achUserProfile);

	static bool	bInit	= true;
	if(bInit == true) {
		bInit	= false;
		m_GenLev.LoadPatterns();
	}
#ifndef __linux__
	if(g_nUnregisterdCount > 30 * 60)	g_nUnregisterdCount	= 30 * 60;
	if(g_nUnregisterdCount == 0)		g_nUnregisterdCount	= 3;
#endif
	
	if(bRestore == true) {
		bool	bLoaded	= false;
		_SAVE	str;
		if(pFile = fopen(achBuf, "rb")) {
			if(sizeof(_SAVE) == fread(&str, 1, sizeof(_SAVE), pFile)) {
				EncodeDecode(&str, sizeof(_SAVE));
				if(strncmp(str.achMagic, "WEGR", 4) == 0) {
					bLoaded	= true;
					m_nCurrentLevel				= str.nLevel;
					m_nScore					= str.nScore;
					m_nLives					= str.nLives;
					m_nScoreToAdditionalBall	= str.nScoreToAdditionalBall;
					m_nBonusLevelType			= str.nBonusLevelType;
					_BRICK	brick;
					for(i = 0; i < str.nBricksCount; i++) {
						fread(&brick, sizeof(_BRICK), 1, pFile);
						EncodeDecode(&brick, sizeof(_BRICK));
						m_vecLevelBricks.push_back(brick);
					}
					if(g_bAutoBonusMode == false) {
						int	nBonusType;
						for(i = 0; i < str.nBonusesCount; i++) {
							fread(&nBonusType, sizeof(int), 1, pFile);
							EncodeDecode(&nBonusType, sizeof(int));
							g_Bonus.AddToStack(nBonusType);
						}
					}
				}
			}
			fclose(pFile);
		}
		if(bLoaded == false) {
			InitNewGame(m_bIsCustomLevels);
			return;
		}

		m_nLevelPrev		= -2;	// inform, that we restore game
		m_bTutorialPause	= false;
	}
	else {
		// set bonus level each 5 level
		if(m_bIsCustomLevels == false && m_nCurrentLevel % 5 == 4)
			m_nBonusLevelType	= g_Rnd.Get(2);	// at this moment only 2 types available
		else
			m_nBonusLevelType	= -1;			// disable bonus level
	}
	
	if(m_nCurrentLevel > 0)	PlaySound(1);
	ResetAll();

	if(m_nBonusLevelType == -1) {
		if(bRestore == false)	m_GenLev.Generate(m_nCurrentLevel, m_bIsCustomLevels);
		g_EnergyHole.RemoveAll();
		switch(m_nCurrentLevel % 5) {
		case 0:
			g_EnergyHole.AddEnergyHole();
			break;
		case 1:
			g_EnergyHole.AddEnergyHole();
			g_EnergyHole.AddEnergyHole();
			break;
		case 3:
			g_EnergyHole.AddEnergyHole();
			if(g_Rnd.Get(10) % 2 == 0)	g_EnergyHole.AddEnergyHole();
			break;
		}
	}
	else if(m_nBonusLevelType == 0) {
		// TODO: place bricks without intersect one by other and symmetrically
		_BRICK	brick;
		m_vecLevelBricks.clear();
		for(i = 0; i < 5; i++) {
			brick.fX				= BRICK_X + g_Rnd.Get(BRICKS_WIDTH / 4) * BRICK_W * 4;
			brick.fY				= BRICK_Y + g_Rnd.Get(BRICKS_HEIGHT / 4) * BRICK_H * 4;
			brick.byType			= BOX_SHOOT_0 + g_Rnd.Get(2);
			brick.nCountToDie		= 10;
			brick.nFrame			= 0;
			brick.nAnimateType		= -1;
			brick.nCountToShoot		= 2 + i;
			m_vecLevelBricks.push_back(brick);
		}
		m_nGetReadyBallsLose	= 0;	// used for counting lost balls
	}
	else if(m_nBonusLevelType == 1) {
		m_vecLevelBricks.clear();
		m_nBonusLevelTicks	= 3 * 60;	// 3 minutes
		// TODO: add paddle power
	}
	
	// store current game state for later restoring
	if(bRestore == false) {
		if(m_nCurrentLevel > 0 && (pFile = fopen(achBuf, "wb"))) {
			_SAVE	str;
			memcpy(str.achMagic, "WEGR", 4);
			str.nLevel					= m_nCurrentLevel;
			str.nScore					= m_nScore;
			str.nLives					= m_nLives;
			str.nScoreToAdditionalBall	= m_nScoreToAdditionalBall;
			str.nBonusLevelType			= m_nBonusLevelType;
			str.nBonusLevelTicks		= m_nBonusLevelTicks;
			str.nGetReadyBallsLose		= m_nGetReadyBallsLose;
			str.nBricksCount			= m_vecLevelBricks.size();
			str.nBonusesCount			= g_Bonus.GetCountInStack();
			str.nBonusLevelType			= m_nBonusLevelType;
			EncodeDecode(&str, sizeof(_SAVE));
			fwrite(&str, sizeof(_SAVE), 1, pFile);
			_BRICK	brick;
			for(i = 0; i < m_vecLevelBricks.size(); i++) {
				memcpy(&brick, &m_vecLevelBricks[i], sizeof(_BRICK));
				EncodeDecode(&brick, sizeof(_BRICK));
				fwrite(&brick, sizeof(_BRICK), 1, pFile);
			}
			int	nBonusType;
			for(i = 0; i < g_Bonus.GetCountInStack(); i++) {
				nBonusType	= g_Bonus.GetTypeInStack(i);
				EncodeDecode(&nBonusType, sizeof(int));
				fwrite(&nBonusType, sizeof(int), 1, pFile);
			}
			fclose(pFile);
		}
	}

	printf("level %d, is bonus level %s\n", m_nCurrentLevel + 1, m_nBonusLevelType != -1 ? "yes" : "no");
}

void CArkanoidSBGame::DrawPaddle() {
	SDL_Rect	src;

	static Uint32	dwEngine	= 0;
	static int		nEngine	= 0;
	src.x	= m_nRacketSize * 12;
	src.y	= m_bPaddleIsInvert == true ? 140 : 0;
	src.w	= 12;
	src.h	= PADDLE_HEIGHT;
	Blit((int)m_nRacketX, (int)m_nRacketY, m_pRacket, &src);
	g_Font.SetRect((int)m_nRacketX, 0, 14, SCREEN_HEIGHT);
	src.x	= 0;	src.h	= 50;
	switch(m_nRacketType) {
	case RT_LASER:
		src.y	= 380; src.w	= 32;
		Blit((int)m_nRacketX - 15, (int)m_nRacketY + (PADDLE_HEIGHT - 50) / 2, m_pRacket, &src);
		break;
	case RT_MISSILE:
		//g_Font.DrawNumber(m_nPaddleMissileCount, 0, (int)m_nRacketY - 15, 2);
	case RT_PLASMA:
		src.y	= 330; src.w	= 36;
		Blit((int)m_nRacketX - 17, (int)m_nRacketY + (PADDLE_HEIGHT - 50) / 2, m_pRacket, &src);
		break;
	case RT_MAGNET:
		break;
	case RT_ENGINE:
		src.y	= 280; src.w	= 21;
		Blit((int)m_nRacketX, (int)m_nRacketY + (PADDLE_HEIGHT - 50) / 2, m_pRacket, &src);
		src.x	= 48;	src.y	= 30 * nEngine;
		src.w	= 30;	src.h	= 30;
		Blit((int)m_nRacketX + 12, (int)m_nRacketY + (PADDLE_HEIGHT - 30) / 2, m_pRacket, &src);
		if(dwEngine + 50 < SDL_GetTicks()) {
			dwEngine	= SDL_GetTicks();
			nEngine++;	nEngine	%= 3;
		}
		break;
	}
	if(m_bPaddleIsInvert == true) {
		g_Font.DrawNumber(m_nPaddleInvertCount, 0, (int)m_nRacketY + PADDLE_HEIGHT, 2);
	}
	g_Font.SetRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	if(m_bCanMovePaddle == false) {
		src.x	= 0;	src.h	= 50;
		src.y	= 430; src.w	= 36;
		Blit((int)m_nRacketX - 12, (int)m_nRacketY + (PADDLE_HEIGHT - 50) / 2, m_pRacket, &src);
		g_Font.DrawNumber(m_nCanMovePaddleCount, (int)m_nRacketX + 18, (int)m_nRacketY + (PADDLE_HEIGHT - 15) / 2);
	}
}

void CArkanoidSBGame::DrawStatistic() {
	char	achBuff[20];
	g_Font2.SetRect(97, 0, 56, SCREEN_HEIGHT);
	g_Font2.DrawNumber(m_nCurrentLevel + 1, 99, 19, CMyString::FONT_ALIGN_CENTER);

	g_Font2.SetRect(174, 0, 141, SCREEN_HEIGHT);
	g_Font2.DrawNumber(m_nScore, 177, 19, CMyString::FONT_ALIGN_CENTER);

	g_Font2.SetRect(338, 0, 56, SCREEN_HEIGHT);
	g_Font2.DrawNumber(m_nLives >= 0 ? m_nLives : 0, 342, 19, CMyString::FONT_ALIGN_CENTER);
	g_Font2.SetRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	int	nX	= 598;
	static int		nFrame	= 0;
	static int		nFrame2	= 0;
	static Uint32	dwTime	= 0;
	SDL_Rect	rc;
	if(dwTime + 60 < SDL_GetTicks()) {
		dwTime	= SDL_GetTicks();
		nFrame++;	nFrame	%= 15;
		nFrame2++;	nFrame2	%= 10;
	}
	rc.x	= nFrame * 36;
	rc.w	= 36;	rc.h	= 36;
	if(m_bBackWall == true) {
		rc.y	= 12 * 36;
		Blit(nX, 10, m_pBonuses, &rc);
		g_Font.DrawNumber(m_nCountBackWall, nX, 31);
		nX	-= 36;
	}
	if(m_bSlowDown == true) {
		rc.y	= 13 * 36;
		Blit(nX, 10, m_pBonuses, &rc);
		g_Font.DrawNumber(m_nSlowDownCount, nX, 31);
		nX	-= 36;
	}
	if(m_nRacketType == RT_PLASMA) {
		rc.y	= 6 * 36;
		Blit(nX, 10, m_pBonuses, &rc);
		g_Font.DrawNumber(m_nPaddleMissileCount, nX, 31);
		nX	-= 36;
	}
	if(m_nRacketType == RT_MISSILE) {
		rc.y	= 7 * 36;
		Blit(nX, 10, m_pBonuses, &rc);
		g_Font.DrawNumber(m_nPaddleMissileCount, nX, 31);
		nX	-= 36;
	}
	if(m_nRacketType == RT_LASER) {
		rc.y	= 8 * 36;
		Blit(nX, 10, m_pBonuses, &rc);
		g_Font.DrawNumber(m_nPaddleMissileCount, nX, 31);
		nX	-= 36;
	}
	int	nType	= g_Ball.GetType();
	if(nType != CBall::TYPE_WHITE) {
		if(nType == CBall::TYPE_RED)	rc.y	= 1 * 36;
		else									rc.y	= 2 * 36;
		Blit(nX, 10, m_pBonuses, &rc);
		g_Font.DrawNumber(g_Ball.GetTypeCount(), nX, 31);
		nX	-= 36;
	}
	if(m_nBonusLevelType == 0) {
		nX	= SCREEN_WIDTH - 5 - 24;
		for(int i = 0; i < 5; i++) {
			rc.x	= 0;	rc.y	= 0;	rc.w	= rc.h	= 22;
			Blit(nX - i * 24 - 1, 18 - 1, g_pTransp, &rc);
			if(i < m_nGetReadyBallsLose) {
				rc.x	= nFrame2 * 20;
				rc.y	= 128 + 12;	rc.w	= rc.h	= 20;
				Blit(nX - i * 24, 18, m_pBall, &rc);
			}
		}
	}
	else if(m_nBonusLevelType == 1) {
		sprintf(achBuff, "%d:%02d", m_nBonusLevelTicks / 60, m_nBonusLevelTicks % 60);
		g_Font.DrawString(5, 10, achBuff, 1);
		//sprintf(achBuff, "power: %d%%", 100 / m_nGetReadyBallsLose);
		//g_Font.DrawString(5, 10, achBuff, 1);
	}
}

void CArkanoidSBGame::DoShoot() {
	int	i, nLaserY	= (int)m_nRacketY + (PADDLE_HEIGHT - 2) / 2;

	if((m_nRacketType == RT_LASER || m_nRacketType == RT_MISSILE || m_nRacketType == RT_PLASMA)) {
		//static Uint32	dwFirePressed	= 0;
		//if(dwFirePressed + 250 < SDL_GetTicks()) {
		//	dwFirePressed	= SDL_GetTicks();
			switch(m_nRacketType) {
			case RT_LASER:
				for(i = 0; i < m_vecLevelBricks.size(); i++) {
					int	nY	= (int)m_vecLevelBricks[i].fY;
					if(nY <= nLaserY + 2 && nY + BRICK_H >= nLaserY) {
					//if(abs(nY - nLaserY) <= abs(4 - nH)) {
						m_nLaserX	= max(m_nLaserX, (int)m_vecLevelBricks[i].fX + BRICK_W);
					}
				}
				g_Bullet.AddBullets((int)m_nRacketY + (PADDLE_HEIGHT - 20) / 2, CBullet::TYPE_LASER);
				PlaySound(8);
				break;
			case RT_MISSILE:
				g_Bullet.AddBullets((int)m_nRacketY + (PADDLE_HEIGHT - 20) / 2, CBullet::TYPE_MISSILE);
				PlaySound(12);
				break;
			case RT_PLASMA:
				g_Bullet.AddBullets((int)m_nRacketY + (PADDLE_HEIGHT - 20) / 2, CBullet::TYPE_PLASMA);
				PlaySound(13);
				break;
			}
			if(--m_nPaddleMissileCount == 0)
				m_nRacketType	= RT_NORMAL;
		//}
	}

	m_nLaserX	= WALL_X1;
}

void CArkanoidSBGame::GetGameData(int &nScore, int &nLevel) {
	nScore		= m_nScore;
	nLevel		= m_nCurrentLevel;
}

void CArkanoidSBGame::ProcessBonus(int nBonusType) {
	if(nBonusType == -1)	return;
	int	x;

	PlaySound(9);
	m_nGetReadyBonuses++;

	switch(nBonusType) {
	case CBonus::TYPE_BALL_SPLIT:
		g_Ball.SplitBalls();
		g_CoolString.Add("split balls");
		break;
	case CBonus::TYPE_BALL_FIRE:
		g_Ball.SetAllBallsToFire();
		g_CoolString.Add("fireball");
		break;
	case CBonus::TYPE_BALL_BLUE:
		g_Ball.SetAllBallsToBlue();
		g_CoolString.Add("shadow ball");
		break;
	case CBonus::TYPE_BALL_SHRINK:
		g_Ball.ChangeBallSize(-1);
		g_CoolString.Add("shrink balls");
		break;
	case CBonus::TYPE_BALL_EXPAND:
		g_Ball.ChangeBallSize(1);
		g_CoolString.Add("expand balls");
		break;
	case CBonus::TYPE_PADDLE_LASER:
		while(true == g_Ball.StartBall((int)m_nRacketY, (int)m_nRacketY + PADDLE_HEIGHT))	;
		if(RT_LASER != m_nRacketType)
			m_nPaddleMissileCount	= 0;
		m_nPaddleMissileCount	+= 40;
		m_nRacketType			= RT_LASER;
		g_CoolString.Add("laser");
		break;
	case CBonus::TYPE_PADDLE_FIRE:
		while(true == g_Ball.StartBall((int)m_nRacketY, (int)m_nRacketY + PADDLE_HEIGHT))	;
		if(RT_PLASMA != m_nRacketType)
			m_nPaddleMissileCount	= 0;
		m_nPaddleMissileCount	+= 1;
		m_nRacketType			= RT_PLASMA;
		g_CoolString.Add("plasmagun");
		break;
	case CBonus::TYPE_PADDLE_MISSILE:
		while(true == g_Ball.StartBall((int)m_nRacketY, (int)m_nRacketY + PADDLE_HEIGHT))	;
		if(RT_MISSILE != m_nRacketType)
			m_nPaddleMissileCount	= 0;
		m_nPaddleMissileCount	+= 10;
		m_nRacketType			= RT_MISSILE;
		g_CoolString.Add("missile");
		break;
	case CBonus::TYPE_PADDLE_SHRINK:
		if(m_nRacketSize > 0)	m_nRacketSize--;
		g_CoolString.Add("shrink paddle");
		break;
	case CBonus::TYPE_PADDLE_EXPAND:
		if(m_nRacketSize < 3)	m_nRacketSize++;
		g_CoolString.Add("expand paddle");
		break;
	case CBonus::TYPE_PADDLE_ENGINE:
		while(true == g_Ball.StartBall((int)m_nRacketY, (int)m_nRacketY + PADDLE_HEIGHT))	;
		m_nPaddleMissileCount	= 0;
		m_nRacketType				= RT_ENGINE;
		g_CoolString.Add("flying paddle");
		break;
	case CBonus::TYPE_PADDLE_MAGNET:
		m_nPaddleMissileCount	= 0;
		m_nRacketType				= RT_MAGNET;
		g_CoolString.Add("magnetic paddle");
		break;
	case CBonus::TYPE_WALL:
		m_bBackWall			= true;
		m_nCountBackWall	+= 30;
		g_CoolString.Add("force field");
		if(m_nSndWall == -1)	m_nSndWall	= PlaySound(0, -1);
		break;
	case CBonus::TYPE_DESTROY:
		for(x = 0; x < m_vecLevelBricks.size(); x++) {
			Uint8	byType	= m_vecLevelBricks[x].byType;
			if(byType < BOX_BOMB_0) {
				if(byType >= BOX_IM_0) {
					ChangeBrick(x, BOX_0 + (byType - BOX_IM_0));
				}
				else if(byType >= BOX_FOU_0) {
					ChangeBrick(x, BOX_0 + (byType - BOX_FOU_0));
				}
				else if(byType >= BOX_TRD_0) {
					ChangeBrick(x, BOX_0 + (byType - BOX_TRD_0));
				}
				else if(byType >= BOX_DBL_0) {
					ChangeBrick(x, BOX_0 + (byType - BOX_DBL_0));
				}
			}
		}
		g_Monster.IsAcross(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, true);
		g_CoolString.Add("bricks destructor");
		break;
	case CBonus::TYPE_FREEZE:
		m_bSlowDown			= true;
		m_nSlowDownCount	= 20;
		g_CoolString.Add("slow down");
		break;
	}
}

void CArkanoidSBGame::DoImpact(int nIndex, bool bRemoveAll) {
	m_nCombosBricks++;
	m_dwCombosTime	= SDL_GetTicks();

	Uint8	byType	= m_vecLevelBricks[nIndex].byType;

	if(byType >= BOX_BOMB_0) {
		AddScore(5);
		DoBomb(nIndex);
	}
	else if(byType >= BOX_IM_0) {	// and BOX_SHOOT_ and BOX_MOV_ too
		if(byType < BOX_MOV_0)	g_TutorialDlg.AddDialog((int)m_vecLevelBricks[nIndex].fX + BRICK_W / 2, (int)m_vecLevelBricks[nIndex].fY + BRICK_H / 2, 0, 8);
		m_vecLevelBricks[nIndex].nCountToDie--;
		if(bRemoveAll == true || m_vecLevelBricks[nIndex].nCountToDie == 0) {
			AddScore(30);
			ChangeBrick(nIndex, BOX_NONE, bRemoveAll);
		}
		else {
			m_vecLevelBricks[nIndex].nAnimateType	= 0;
			m_vecLevelBricks[nIndex].nFrame			= 0;
		}
	}
	else if(byType >= BOX_FOU_0) {
		AddScore(5);
		ChangeBrick(nIndex, BOX_TRD_0 + (byType - BOX_FOU_0), bRemoveAll);
	}
	else if(byType >= BOX_TRD_0) {
		AddScore(5);
		ChangeBrick(nIndex, BOX_DBL_0 + (byType - BOX_TRD_0), bRemoveAll);
	}
	else if(byType >= BOX_DBL_0) {
		AddScore(5);
		ChangeBrick(nIndex, BOX_0 + (byType - BOX_DBL_0), bRemoveAll);
	}
	else {
		AddScore(m_bSelfDestructStarted == false ? 5 : 100);
		ChangeBrick(nIndex, BOX_NONE, true);
	}
	if(byType < BOX_BOMB_0) {
		PlaySound(7);
	}
}

void CArkanoidSBGame::ChangeBrick(int nIndex, Uint8 byToBrickType, bool bRemoveAll) {
	if(bRemoveAll == true) {
		byToBrickType	= BOX_NONE;
	}
	else {
		m_vecLevelBricks[nIndex].nAnimateType	= 1;
		m_vecLevelBricks[nIndex].nFrame			= 0;
	}

	// add bonus. only for non bonus level
	if(byToBrickType == BOX_NONE) {
		m_nGetReadyBricks++;
		int	nX	= (int)m_vecLevelBricks[nIndex].fX;
		int	nY	= (int)m_vecLevelBricks[nIndex].fY;
		g_Exploision.AddExploision(nX - 12, nY - 5, 3);
		if(m_nBonusLevelType == -1) {
			static Uint32	dwTime	= 0;
			// brick 47 adds bonus always
			if(m_vecLevelBricks[nIndex].byType == BOX_47 || dwTime + 5000 < SDL_GetTicks()) {
				if(m_vecLevelBricks[nIndex].byType == BOX_47)
					g_TutorialDlg.AddDialog(nX + BRICK_W / 2, nY + BRICK_H / 2, 0, 7);
				dwTime	= SDL_GetTicks();
				int	nType	= g_Rnd.Get(CBonus::TYPE_LAST_BONUS);
#ifndef __linux__
				if(g_bIsRegistered == false) {
					while(nType == CBonus::TYPE_BALL_FIRE || nType == CBonus::TYPE_PADDLE_MISSILE || nType == CBonus::TYPE_PADDLE_ENGINE) {
						nType++;	nType	%= CBonus::TYPE_LAST_BONUS;
					}
				}
#endif
				g_Bonus.AddBonus(nX, nY, nType);
				PlaySound(10);
			}
		}
	}

	m_vecLevelBricks[nIndex].byType	= byToBrickType;
}

void CArkanoidSBGame::AddScore(int nScore) {
	m_nGetReadyScore	+= nScore;
	m_nScore				+= nScore;
	m_nScoreToAdditionalBall	-= nScore;
	if(m_nScoreToAdditionalBall <= 0) {
		m_nScoreToAdditionalBall	+= SCORE_TO_ADDITIONAL_BALL;
		m_nLives++;
	}
}

/*!
    \fn CArkanoidSBGame::ResetAll()
 */
void CArkanoidSBGame::ResetAll() {
	m_dwCombosTime				= 0;
	m_nCombosBricks			= 0;

	m_nCountBackWall			= 0;
	m_bBackWall					= false;
	StopSound(m_nSndWall);
	//m_bSelfDestructStarted	= false;
	//m_nCountSelfDestruct		= 0;
	m_bCanMovePaddle			= true;
	m_nCanMovePaddleCount	= 0;
	m_bSlowDown					= false;
	m_nSlowDownCount			= 0;

	m_nAddNewMonsterCount	= 0;

	m_bPaddleIsInvert			= false;
	m_nPaddleInvertCount		= 0;
	m_nBrickCount				= -1;
	switch(m_nBonusLevelType) {
	case -1:	m_nRacketSize	= 1;	break;
	case 0:		m_nRacketSize	= 2;	break;
	case 1:		m_nRacketSize	= 0;	break;
	}
	m_nRacketType				= RT_NORMAL;
	m_nPaddleMissileCount	= 0;
	m_nRacketY					= WALL_Y1 + ((WALL_Y2 - WALL_Y1) - PADDLE_HEIGHT) / 2;
	m_nRacketX					= RACKET_X;

	m_vecBrickBullets.clear();
	m_vecBrickBullets.reserve(10);
	g_Bonus.RemoveAll(false);
	g_Exploision.RemoveAll();
	g_Bullet.RemoveAll();
	g_Monster.RemoveAll();
	g_Ball.RemoveAll();
	if(m_nBonusLevelType == -1 || m_nBonusLevelType == 1) {
		g_Ball.AddBall((int)m_nRacketX - 20, (int)m_nRacketY + (PADDLE_HEIGHT - 20) / 2);
	}
}


/*!
    \fn CArkanoidSBGame::DrawBrickBullets()
 */
void CArkanoidSBGame::DrawBrickBullets() {
	SDL_Rect			rc;
	static Uint32	dwTime	= 0;
	bool				bUpdate	= false;
	if(dwTime + 50 < SDL_GetTicks()) {
		dwTime	= SDL_GetTicks();
		bUpdate	= true;
	}
	rc.w	= 20;
	rc.h	= 20;
	for(int i = 0; i < m_vecBrickBullets.size(); i++) {
		rc.x	= m_vecBrickBullets[i].nFrame * 20;
		rc.y	= m_vecBrickBullets[i].nType * 20;
		Blit((int)m_vecBrickBullets[i].fX, (int)m_vecBrickBullets[i].fY, m_pBricksMovBullets, &rc);
		if(bUpdate == true) {
			m_vecBrickBullets[i].nFrame++;
			m_vecBrickBullets[i].nFrame	%= 5;
		}
	}
}


/*!
    \fn CArkanoidSBGame::MoveBrickBullets()
 */
void CArkanoidSBGame::MoveBrickBullets() {
	for(int i = 0; i < m_vecBrickBullets.size(); i++) {
		m_vecBrickBullets[i].fX	+= (g_fSpeedCorrection * g_fSin[m_vecBrickBullets[i].nAngle]);
		m_vecBrickBullets[i].fY	-= (g_fSpeedCorrection * g_fCos[m_vecBrickBullets[i].nAngle]);
		if(m_vecBrickBullets[i].fX < WALL_X1 || m_vecBrickBullets[i].fX > (m_bBackWall == true ? WALL_X2 - 20 : SCREEN_WIDTH) ||
				m_vecBrickBullets[i].fY < WALL_Y1 || m_vecBrickBullets[i].fY + 20 > WALL_Y2) {
			if(m_vecBrickBullets[i].fX < SCREEN_WIDTH)
				g_Exploision.AddExploision((int)m_vecBrickBullets[i].fX - 12, (int)m_vecBrickBullets[i].fY - 10, 0);
			swap(m_vecBrickBullets[i--], m_vecBrickBullets.back());
			m_vecBrickBullets.resize(m_vecBrickBullets.size() - 1);
		}
	}
}


/*!
    \fn CArkanoidSBGame::CalcBrickBulletsAngle(int nIndex, int nX, int nY)
 */
int CArkanoidSBGame::CalcBrickBulletsAngle(int nIndex, int nX, int nY) {
	int		nAngle	= 0;
	int		nCatet1	= nX - ((int)m_vecLevelBricks[nIndex].fX + BRICK_W / 2);
	int		nCatet2	= nY - ((int)m_vecLevelBricks[nIndex].fY + BRICK_H / 2);
	double	fDist		= sqrt(nCatet1 * nCatet1 + nCatet2 * nCatet2);
	nAngle				= (int)(57.3f * asin(nCatet2 / fDist));
	if(nCatet1 > 0)	nAngle	= 90 + nAngle;
	else					nAngle	= 270 - nAngle;
	return	((360 + nAngle) % 360);
}


/*!
    \fn CArkanoidSBGame::IsEmptyBrickPos(const int nX, const int nX)
 */
bool CArkanoidSBGame::IsEmptyBrickPos(const int nSkipPos, const int nX, const int nY) {
	for(int i = 0; i < m_vecLevelBricks.size(); i++) {
		if(i == nSkipPos)	continue;	// skip checking itself
		int	nBx	= int(m_vecLevelBricks[i].fX);
		int	nBy	= int(m_vecLevelBricks[i].fY);
		if(abs(nX - nBx) == 0 && abs(nY - nBy) == 0) {
			return	false;
		}
	}

	return	true;
}


/*!
    \fn CArkanoidSBGame::DrawGetReady()
 */
bool CArkanoidSBGame::DrawGetReady() {
	char	achBuf[50];

	FadeScreen();

	if(m_nBonusLevelType == -1) {
		sprintf(achBuf, "LEVEL %d\nGET READY!", m_nCurrentLevel + 1);
		if(m_pchGetReeadyInfo != 0 && m_nLevelPrev != -2) {
			g_Font3.DrawString(0, 218, m_pchGetReeadyInfo, 2);
		}
	}
	else if(m_nBonusLevelType == 0) {
		sprintf(achBuf, "BONUS LEVEL %d\nGET READY!", (m_nCurrentLevel + 1) / 5);
		g_Font3.DrawString(0, 218, "You should destroy all cannons, don't lost more than 5 balls!", 2);
	}
	else if(m_nBonusLevelType == 1) {
		sprintf(achBuf, "BONUS LEVEL %d\nGET READY!", (m_nCurrentLevel + 1) / 5);
		g_Font3.DrawString(0, 218, "You should destroy all wreckage!", 2);
	}
	g_Font2.DrawString(0, 170, achBuf, CMyString::FONT_ALIGN_CENTER);

	// do not show that info while we restore game
	if(m_nLevelPrev != -2) {
		g_Font.DrawString(200, 240+2, "Bricks destroyed");
		g_Font2.DrawNumber(m_nGetReadyBricks, 200, 240, CMyString::FONT_ALIGN_RIGHT);
	
		g_Font.DrawString(200, 260+2, "Monsters destroyed");
		g_Font2.DrawNumber(m_nGetReadyMonsters, 200, 260, CMyString::FONT_ALIGN_RIGHT);
	
		g_Font.DrawString(200, 280+2, "Gained score");
		g_Font2.DrawNumber(m_nGetReadyScore, 200, 280, CMyString::FONT_ALIGN_RIGHT);
	
		g_Font.DrawString(200, 300+2, "Bonuses collected");
		g_Font2.DrawNumber(m_nGetReadyBonuses, 200, 300, CMyString::FONT_ALIGN_RIGHT);
	
		g_Font.DrawString(200, 320+2, "Balls lost");
		g_Font2.DrawNumber(m_nGetReadyBallsLose, 200, 320, CMyString::FONT_ALIGN_RIGHT);
	}
	
	bool	bIsOver	= g_MainMenu.DrawMenuButton((SCREEN_WIDTH - 124) / 2, 350, CMainMenu::B_OK);
	if(g_bMouseLB == true && true == bIsOver) {
		g_bMouseLB	= false;
		return true;
	}
	return false;
}


/*!
    \fn CArkanoidSBGame::AddGetReeadyInfo(const char *pchString)
 */
void CArkanoidSBGame::AddGetReeadyInfo(const char *pchString) {
	delete[]	m_pchGetReeadyInfo;
	m_pchGetReeadyInfo	= new char[strlen(pchString) + 5];
	sprintf(m_pchGetReeadyInfo, "[ %s ]", pchString);
}


/*!
    \fn CArkanoidSBGame::RemoveOneLives
 */
void CArkanoidSBGame::RemoveOneLives() {
	m_nLives--;
	EnableCursor(true);
	StopSound(m_nSndWall);
	if(m_nLives >= 0) {
		//PlaySound(99);//g_pMainFrame->m_pImix->SamplePlay(m_nSampleGetReady);
		m_nGameState			= GS_GET_READY;
	}
	else {
		g_CoolString.Add("game over");
		//PlaySound(99);//g_pMainFrame->m_pImix->SamplePlay(m_nSampleGameOver);
		m_nGameState	= GS_GAME_OVER;
	}
}


/*!
    \fn CArkanoidSBGame::LoadBackground()
 */
void CArkanoidSBGame::LoadBackground() {
	if(m_nLevelImage != m_nCurrentLevel) {
		m_nLevelImage	= m_nCurrentLevel;
		char	achBuf[20];
		if(m_nBonusLevelType == -1) {
			sprintf(achBuf, "gamebg%d.jpg", m_nCurrentLevel % 5);
		}
		else {
			strcpy(achBuf, "gamebg5.jpg");
		}
		SDL_FreeSurface(g_pGameBG);
		g_pGameBG	= LoadImage(achBuf);
	}
}


/*!
    \fn CArkanoidSBGame::FreeBackground()
 */
void CArkanoidSBGame::FreeBackground() {
	m_nLevelImage	= -1;
	SDL_FreeSurface(g_pGameBG);
	g_pGameBG	= 0;
}


/*!
    \fn CArkanoidSBGame::DrawBackground()
 */
void CArkanoidSBGame::DrawBackground() {
	// loading new background
	LoadBackground();

	// draw background
	static Uint32	dwBGtime	= 0;
	static int		nFrame	= 0,	nPumpX	= 0;
	static bool		bPumpDir	= true;
	Blit(0, 0, g_pGameBG, 0);
	if(dwBGtime + 30 < SDL_GetTicks()) {
		dwBGtime	= SDL_GetTicks();
		nFrame++;
		nFrame	%= (240 - 20);
		if(bPumpDir == true)	nPumpX++;
		else						nPumpX--;
		if(nPumpX == 0 || nPumpX == 33)	bPumpDir	= !bPumpDir;
	}
	SDL_Rect	rc;
	rc.x	= nFrame;
	rc.y	= 0;	rc.w	= 10;	rc.h	= 14;
	Blit(5, 128, m_pGameBGanims, &rc);
	Blit(5, 408, m_pGameBGanims, &rc);

	rc.x	= int(nFrame * 3);
	rc.y	= 14;	rc.w	= 15;	rc.h	= 9;
	Blit(81, 56, m_pGameBGanims, &rc);
	rc.h	= 10;
	Blit(544, 465, m_pGameBGanims, &rc);

	rc.x	= 337;	rc.y	= 0;
	rc.w	= 23;		rc.h	= 14;
	Blit(380 + nPumpX, 73, m_pGameBGanims, &rc);
}


/*!
    \fn CArkanoidSBGame::DrawAreYouSure()
 */
void CArkanoidSBGame::DrawAreYouSure() {
#define	MENU_ITEM_X	((SCREEN_WIDTH - 248) / 2)
#define	MANU_ITEM_Y	((SCREEN_HEIGHT - 15) / 2)

	FadeScreen();
	g_Font2.DrawString(0, MANU_ITEM_Y - 10, "ARE YOU SURE TO LEAVE", CMyString::FONT_ALIGN_CENTER);
	g_Font2.DrawString(0, MANU_ITEM_Y + 10, "THIS BOARD", CMyString::FONT_ALIGN_CENTER);

	int nItem	= -1;
	if(true == g_MainMenu.DrawMenuButton(MENU_ITEM_X, MANU_ITEM_Y + 40, CMainMenu::B_OK))	nItem	= 0;
	if(true == g_MainMenu.DrawMenuButton(MENU_ITEM_X + 124, MANU_ITEM_Y + 40, CMainMenu::B_CANCEL))	nItem	= 1;
	if(true == g_MainMenu.DrawMenuButton(MENU_ITEM_X, MANU_ITEM_Y + 79, CMainMenu::B_OPTIONS))	nItem	= 2;

	bool	bExit	= false;
	if(g_bMouseLB == true) {
		g_bMouseLB	= false;
		switch(nItem) {
		case 0:
			EnableCursor(true);
			m_nGameState	= GS_GAME_OVER;
			break;
		case 1:
			bExit	= true;
			break;
		case 2:
			EnableCursor(true);
			m_nGameState	= GS_GAME_PAUSED;
			g_nGameMode		= APPS_MAINMENU;
			g_MainMenu.SetMenuType(CMainMenu::MT_OPTIONS, true);
			break;
		}
	}
	if((IsKeyPressed(SDLK_ESCAPE) && IsKeyStateChanged(SDLK_ESCAPE)) || bExit == true || g_bMouseRB == true) {
		SendEsc();
	}
}

void CArkanoidSBGame::DrawPause() {
#define	PAUSE_ITEM_X	((SCREEN_WIDTH - 124) / 2)
#define	PAUSE_ITEM_Y	((SCREEN_HEIGHT - 15) / 2)

	FadeScreen();

	g_Font2.DrawString(0, (SCREEN_HEIGHT - 15) / 2 - 10, "GAME PAUSED", CMyString::FONT_ALIGN_CENTER);
	g_Font.DrawString(0, (SCREEN_HEIGHT - 15) / 2 + 20, "Press <P> key to unpause.", CMyString::FONT_ALIGN_CENTER);

	bool	bIsOver	= g_MainMenu.DrawMenuButton(PAUSE_ITEM_X, PAUSE_ITEM_Y + 40, CMainMenu::B_OK);
	if(g_bMouseRB == true || (g_bMouseLB == true && true == bIsOver) ||
		  (IsKeyPressed(SDLK_ESCAPE) && IsKeyStateChanged(SDLK_ESCAPE)) || (IsKeyPressed(SDLK_p) && IsKeyStateChanged(SDLK_p))) {
		SendEsc();
	}
}

void CArkanoidSBGame::SetPause() {
	if(m_nGameState == GS_GAME_ACTIVE) {
		EnableCursor(true);
		m_nGameState	= GS_GAME_PAUSED;
	}
}


/*!
    \fn CArkanoidSBGame::RestoreGame()
 */
void CArkanoidSBGame::RestoreGame() {
	InitLevel(0, true);

	//m_nGameState	= GS_GAME_RESTORE;
}


/*!
    \fn CArkanoidSBGame::DrawRestoreGame()
 *//*
void CArkanoidSBGame::DrawRestoreGame() {
	FadeScreen();
// #define MAX_SAVE_NAME   30
// 	struct _SAVED_GAME {
// 		BOOL    bIsValid;
// 		int             nLevel;
// 		int             nScore;
// 		int             nLives;
// 		char    achName[MAX_SAVE_NAME + 1];
// 		struct CLevel::_BRICK   astrLevel[LEVEL_WIDTH][LEVEL_HEIGHT];
// 	} m_astrSavedGames[10];

	int nItem	= -1;
	if(true == g_MainMenu.DrawMenuButton(SCREEN_WIDTH / 2 - 124 - 10, SCREEN_HEIGHT - 50, CMainMenu::B_OK))	nItem	= 0;
	if(true == g_MainMenu.DrawMenuButton(SCREEN_WIDTH / 2 + 10, SCREEN_HEIGHT - 50, CMainMenu::B_CANCEL))	nItem	= 1;

	if(g_bMouseRB == true || (g_bMouseLB == true && nItem == 0) || (IsKeyPressed(SDLK_ESCAPE) && IsKeyStateChanged(SDLK_ESCAPE))) {
		g_bMouseRB	= false;
		g_bMouseLB	= false;
		InitNewGame(m_bIsCustomLevels);
	}
	if(g_bMouseLB == true && nItem == 1) {
		g_bMouseLB	= false;
		g_MainMenu.SetMenuType(CMainMenu::MT_MAIN);
		g_nGameMode	= APPS_MAINMENU;
	}
}
*/


/*!
    \fn CArkanoidSBGame::SendEsc()
 */
void CArkanoidSBGame::SendEsc() {
	switch(m_nGameState) {
	case GS_GAME_ACTIVE:
		EnableCursor(true);
		m_nGameState	= GS_GAME_AREYOUSURE_EXIT;
		break;
	case GS_GET_READY:
			if(m_nLevelPrev != m_nCurrentLevel) {
				m_nLevelPrev			= m_nCurrentLevel;
				m_nGetReadyScore		= 0;
				m_nGetReadyMonsters		= 0;
				m_nGetReadyBonuses		= 0;
				m_nGetReadyBricks		= 0;
				m_nGetReadyBallsLose	= 0;
			}
		//if(m_nCurrentLevel != 1)
			PlaySound(99);//g_pMainFrame->m_pImix->SamplePlay(m_nSampleGetReady);
			m_nGameState	= GS_GAME_ACTIVE;
			g_bMouseRB		= false;
			ResetAll();
		//g_CoolString.Add("LET'S GO!");
			EnableCursor(false);
		break;
	case GS_GAME_OVER:
		m_nBonusLevelType	= -1;
		StopSound(m_nSndWall);
		g_bMouseRB	= false;
		g_bMouseLB	= false;
		break;
	case GS_GAME_AREYOUSURE_EXIT:
		g_bMouseRB	= false;
		EnableCursor(false);
		m_nGameState	= GS_GAME_ACTIVE;
		break;
	case GS_GAME_PAUSED:
		g_bMouseRB	= false;
		g_bMouseLB	= false;
		EnableCursor(false);
		m_nGameState	= GS_GAME_ACTIVE;
		break;
	}
}
