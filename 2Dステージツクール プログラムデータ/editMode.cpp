#include "DxLib.h"
#include "stdlib.h"
#include <stdio.h>
#include <math.h>
#include "definitions.h"
static int a, b, c, d, e;
static int editCursorScrollPerTime = 2, editCursorScrollStart = 10;//キーを押しっぱなしでカーソルが移動する時間の設定
static int editCursorMoveSound = 0, editCursorMoveSoundWait = 0;//editCursorX,YとeditMapX,Yの合計が前フレームと異なるのなら音を出す設定,waitは次に音を鳴らすまでの時間待つため
static int editMapTipNumber = 0;//選択しているマップチップの値。これをステージに代入していく
static int editMapTipSelect = 0;//選択しているマップチップの番号
static int editCursorCount = 0;//カーソルのアニメーション用
static int editOperation = NO;//NO操作説明表示なし,YES操作説明表示
//音読み込み
static int soundEditCursorMove = LoadSoundMem("音声/editCursorMove.mp3");//カーソル移動したときの効果音

void editModeInitialize(SaveData_t *Data) {
	ChangeVolumeSoundMem(255 * 30 / 100, soundEditCursorMove);//音の大きさを調整している。
	Data->LayerNow = 0;
	Data->TimeMax = 0; Data->CoinExist = 0; Data->CoinNeed = 0;
	Data->CoinMax = 99999;
	Data->PlayerStartX = 1; Data->PlayerStartY = 1;
	for (a = 0; a < EDIT_LAYER_MAX; a++)
	{
		for (b = 0; b < MAP_Y_MAX; b++)
		{
			Data->MapData[a][b][0] = bBlock;
			Data->MapData[a][b][MAP_Y_MAX - 1] = bBlock;
		}
		for (b = 0; b < MAP_X_MAX; b++)
		{
			Data->MapData[a][0][b] = bBlock;
			Data->MapData[a][MAP_X_MAX - 1][b] = bBlock;
		}
	}
	for (a = 0; a < EDIT_LAYER_MAX; a++)
	{
		for (b = 1; b < MAP_Y_MAX - 1; b++)
		{
			for (c = 1; c < MAP_X_MAX - 1; c++)
			{
				Data->MapData[a][b][c] = bAir;
			}
		}
	}
	for (a = 0; a < EDIT_LAYER_MAX; a++)
	{
		Data->CursorX[a] = 0;
		Data->CursorY[a] = 47;
		Data->MapX[a] = 1;
		Data->MapY[a] = -MAP_Y_MAX + 49;
		Data->MapXmax[a] = MAP_X_MAX;
		Data->MapYmax[a] = MAP_Y_MAX;
		Data->CoinExist = 0;
	}
	Data->MapData[Data->LayerNow][-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow]][-Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow]] = bAir;
	Data->PlayerStartX = -Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow];
	Data->PlayerStartY = -Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow];
	for (a = 0; a < EDIT_4DIRECTION_CANON_MAX; a++)
	{
		Data->CanonAimNumber[a] = a;
		Data->Canon4Number[a] = a;
	}
}
void editModeExecute(SaveData_t* Data, const int(&Key)[256]) {
	SetDrawScreen(DX_SCREEN_BACK);
	if ((Key[KEY_INPUT_E] == 1 || (Key[KEY_INPUT_E] % editCursorScrollPerTime == 0 && Key[KEY_INPUT_E] >= editCursorScrollStart)) && Key[KEY_INPUT_D] == 0)
	{
		if (Data->CursorY[Data->LayerNow] == 0 && Data->MapY[Data->LayerNow] <= -2)
		{
			Data->MapY[Data->LayerNow]++;
		}
		else if (Data->CursorY[Data->LayerNow] <= 47 && Data->CursorY[Data->LayerNow] >= 1)
		{
			Data->CursorY[Data->LayerNow]--;
		}
	}
	else if ((Key[KEY_INPUT_D] == 1 || (Key[KEY_INPUT_D] % editCursorScrollPerTime == 0 && Key[KEY_INPUT_D] >= editCursorScrollStart)) && Key[KEY_INPUT_E] == 0)
	{
		if (Data->CursorY[Data->LayerNow] == 47 && -Data->MapY[Data->LayerNow] + 50 <= Data->MapYmax[Data->LayerNow])
		{
			Data->MapY[Data->LayerNow]--;
		}
		else if (Data->CursorY[Data->LayerNow] <= 46 && Data->CursorY[Data->LayerNow] >= 0)
		{
			Data->CursorY[Data->LayerNow]++;
		}
	}
	if ((Key[KEY_INPUT_S] == 1 || (Key[KEY_INPUT_S] % editCursorScrollPerTime == 0 && Key[KEY_INPUT_S] >= editCursorScrollStart)) && Key[KEY_INPUT_F] == 0)
	{
		if (Data->CursorX[Data->LayerNow] == 0 && Data->MapX[Data->LayerNow] <= -2)
		{
			Data->MapX[Data->LayerNow]++;
		}
		else if (Data->CursorX[Data->LayerNow] <= 63 && Data->CursorX[Data->LayerNow] >= 1)
		{
			Data->CursorX[Data->LayerNow]--;
		}
	}
	else if ((Key[KEY_INPUT_F] == 1 || (Key[KEY_INPUT_F] % editCursorScrollPerTime == 0 && Key[KEY_INPUT_F] >= editCursorScrollStart)) && Key[KEY_INPUT_S] == 0)
	{
		if (Data->CursorX[Data->LayerNow] == 63 && -Data->MapX[Data->LayerNow] + 66 <= Data->MapXmax[Data->LayerNow])
		{
			Data->MapX[Data->LayerNow]--;
		}
		else if (Data->CursorX[Data->LayerNow] <= 62 && Data->CursorX[Data->LayerNow] >= 0)
		{
			Data->CursorX[Data->LayerNow]++;
		}
	}
	//カーソルを移動させたときの音を鳴らす設定
	if (editCursorMoveSound != Data->CursorX[Data->LayerNow] * 10000 + Data->CursorY[Data->LayerNow] + Data->MapX[Data->LayerNow] * 10000 + Data->MapY[Data->LayerNow] && editCursorMoveSoundWait >= editCursorScrollPerTime)
	{
		PlaySoundMem(soundEditCursorMove, DX_PLAYTYPE_BACK);
		editCursorMoveSoundWait = 0;
	}
	editCursorMoveSound = Data->CursorX[Data->LayerNow] * 10000 + Data->CursorY[Data->LayerNow] + Data->MapX[Data->LayerNow] * 10000 + Data->MapY[Data->LayerNow];
	editCursorMoveSoundWait++;
	if ((Key[KEY_INPUT_Q] == 1 || (Key[KEY_INPUT_Q] % (editCursorScrollPerTime * 5) == 0 && Key[KEY_INPUT_Q] >= editCursorScrollStart)) && Key[KEY_INPUT_A] == 0)
	{
		editCursorCount = 0;
		//非常に効率の悪い書き方ですが、後で設定を変更する可能性があるのでこのような書き方になってます
		switch (editMapTipNumber) {
		case bBlock:editMapTipNumber = bAir;break;
		case bAir:editMapTipNumber = bStart;break;
		case bStart:editMapTipNumber = bJump;break;
		case bJump:editMapTipNumber = bCoin;break;
		case bCoin:editMapTipNumber = bHeart;break;
		case bHeart:editMapTipNumber = bGoal;break;
		case bGoal:editMapTipNumber = bConveyorRightL;break;
		case bConveyorRightL:editMapTipNumber = bConveyorRightM;break;
		case bConveyorRightM:editMapTipNumber = bConveyorRightS;break;
		case bConveyorRightS:editMapTipNumber = bConveyorLeftL;break;
		case bConveyorLeftL:editMapTipNumber = bConveyorLeftM;break;
		case bConveyorLeftM:editMapTipNumber = bConveyorLeftS;break;
		case bConveyorLeftS:editMapTipNumber = bConveyorUpL;break;
		case bConveyorUpL:editMapTipNumber = bConveyorUpM;break;
		case bConveyorUpM:editMapTipNumber = bConveyorUpS;break;
		case bConveyorUpS:editMapTipNumber = bDieBlock;break;
		case bDieBlock:editMapTipNumber = bDamageBlock;break;
		case bDamageBlock:editMapTipNumber = bCanon4DirectionDown;break;
		case bCanon4DirectionDown:editMapTipNumber = bCanon4DirectionRight;break;
		case bCanon4DirectionRight:editMapTipNumber = bCanon4DirectionLeft;break;
		case bCanon4DirectionLeft:editMapTipNumber = bCanon4DirectionUp;break;
		case bCanon4DirectionUp:editMapTipNumber = bCanonAim;break;
		case bCanonAim:editMapTipNumber = bIceBlock;break;
		case bIceBlock:editMapTipNumber = bBridge;break;
		case bBridge:editMapTipNumber = bBlock;break;
			//case 2:editMapTipNumber = 1;
			//case 1:editMapTipNumber = bAir;
		}
	}
	else if ((Key[KEY_INPUT_A] == 1 || (Key[KEY_INPUT_A] % (editCursorScrollPerTime * 5) == 0 && Key[KEY_INPUT_A] >= editCursorScrollStart)) && Key[KEY_INPUT_Q] == 0)
	{
		editCursorCount = 0;
		//非常に効率の悪い書き方ですが、後で設定を変更する可能性があるのでこのような書き方になってます
		switch (editMapTipNumber) {
		case bBlock:editMapTipNumber = bBridge;break;
		case bAir:editMapTipNumber = bBlock;break;
		case bStart:editMapTipNumber = bAir;break;
		case bJump:editMapTipNumber = bStart;break;
		case bCoin:editMapTipNumber = bJump;break;
		case bHeart:editMapTipNumber = bCoin;break;
		case bGoal:editMapTipNumber = bHeart;break;
		case bConveyorRightL:editMapTipNumber = bGoal;break;
		case bConveyorRightM:editMapTipNumber = bConveyorRightL;break;
		case bConveyorRightS:editMapTipNumber = bConveyorRightM;break;
		case bConveyorLeftL:editMapTipNumber = bConveyorRightS;break;
		case bConveyorLeftM:editMapTipNumber = bConveyorLeftL;break;
		case bConveyorLeftS:editMapTipNumber = bConveyorLeftM;break;
		case bConveyorUpL:editMapTipNumber = bConveyorLeftS;break;
		case bConveyorUpM:editMapTipNumber = bConveyorUpL;break;
		case bConveyorUpS:editMapTipNumber = bConveyorUpM;break;
		case bDieBlock:editMapTipNumber = bConveyorUpS;break;
		case bDamageBlock:editMapTipNumber = bDieBlock;break;
		case bCanon4DirectionDown:editMapTipNumber = bDamageBlock;break;
		case bCanon4DirectionRight:editMapTipNumber = bCanon4DirectionDown;break;
		case bCanon4DirectionLeft:editMapTipNumber = bCanon4DirectionRight;break;
		case bCanon4DirectionUp:editMapTipNumber = bCanon4DirectionLeft;break;
		case bCanonAim:editMapTipNumber = bCanon4DirectionUp;break;
		case bIceBlock:editMapTipNumber = bCanonAim;break;
		case bBridge:editMapTipNumber = bIceBlock;break;
			//case 2:editMapTipNumber = bBridge;
			//case 1:editMapTipNumber = 2;
		}
	}
	if (Key[KEY_INPUT_RETURN] >= 1)//エンターキー
	{
		Data->isThisStageCleared = false;
		if (editMapTipNumber == bStart)
		{
			if (Data->MapData[Data->LayerNow][-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow]][-Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow]] == bCoin)
			{
				Data->CoinExist--;
			}
			//4方向キャノンのマップチップを削除するとき
			if (Data->MapData[Data->LayerNow][-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow]][-Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow]] >= bCanon4DirectionUp && Data->MapData[Data->LayerNow][-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow]][-Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow]] <= bCanon4DirectionRight)
			{
				for (a = 0;a < Data->Canon4Max;a++)
				{
					if (-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow] == Data->Canon4Y[a] && -Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow] == Data->Canon4X[a])
					{
						for (b = a;b < Data->Canon4Max;b++)
						{
							Data->Canon4Direction[b] = Data->Canon4Direction[b + 1];
							Data->Canon4X[b] = Data->Canon4X[b + 1];
							Data->Canon4Y[b] = Data->Canon4Y[b + 1];
							Data->Canon4BulletSpeed[b] = Data->Canon4BulletSpeed[b + 1];
							Data->Canon4BulletLiveTime[b] = Data->Canon4BulletLiveTime[b + 1];
							Data->Canon4BulletRestTime[b] = Data->Canon4BulletRestTime[b + 1];
						}
						Data->Canon4Max--;
					}
				}
			}
			//自機狙いキャノンのマップチップを削除するとき
			if (Data->MapData[Data->LayerNow][-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow]][-Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow]] == bCanonAim)
			{
				for (a = 0;a < Data->CanonAimMax;a++)
				{
					if (-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow] == Data->CanonAimY[a] && -Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow] == Data->CanonAimX[a])
					{
						for (b = a;b < Data->CanonAimMax;b++)
						{
							Data->CanonAimDirection[b] = Data->CanonAimDirection[b + 1];
							Data->CanonAimX[b] = Data->CanonAimX[b + 1];
							Data->CanonAimY[b] = Data->CanonAimY[b + 1];
							Data->CanonAimBulletSpeed[b] = Data->CanonAimBulletSpeed[b + 1];
							Data->CanonAimBulletLiveTime[b] = Data->CanonAimBulletLiveTime[b + 1];
							Data->CanonAimBulletRestTime[b] = Data->CanonAimBulletRestTime[b + 1];
						}
						Data->CanonAimMax--;
					}
				}
			}
			Data->MapData[Data->LayerNow][-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow]][-Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow]] = bAir;
			Data->PlayerStartX = -Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow];
			Data->PlayerStartY = -Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow];
		}
		else if (Data->PlayerStartX != -Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow] || Data->PlayerStartY != -Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow])
		{
			if (editMapTipNumber == bCoin)
			{
				Data->CoinExist++;
			}
			if (Data->MapData[Data->LayerNow][-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow]][-Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow]] == bCoin)
			{
				Data->CoinExist--;
			}
			//4方向キャノンのマップチップを削除するとき
			if (Data->MapData[Data->LayerNow][-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow]][-Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow]] >= bCanon4DirectionUp && Data->MapData[Data->LayerNow][-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow]][-Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow]] <= bCanon4DirectionDown)
			{
				for (a = 0;a < Data->Canon4Max;a++)
				{
					if (-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow] == Data->Canon4Y[a] && -Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow] == Data->Canon4X[a])
					{
						for (b = a;b < Data->Canon4Max;b++)
						{
							Data->Canon4Direction[b] = Data->Canon4Direction[b + 1];
							Data->Canon4X[b] = Data->Canon4X[b + 1];
							Data->Canon4Y[b] = Data->Canon4Y[b + 1];
							Data->Canon4BulletSpeed[b] = Data->Canon4BulletSpeed[b + 1];
							Data->Canon4BulletLiveTime[b] = Data->Canon4BulletLiveTime[b + 1];
							Data->Canon4BulletRestTime[b] = Data->Canon4BulletRestTime[b + 1];
						}
						Data->Canon4Max--;
					}
				}
			}
			//自機狙いキャノンのマップチップを削除するとき
			if (Data->MapData[Data->LayerNow][-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow]][-Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow]] == bCanonAim)
			{
				for (a = 0;a < Data->CanonAimMax;a++)
				{
					if (-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow] == Data->CanonAimY[a] && -Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow] == Data->CanonAimX[a])
					{
						for (b = a;b < Data->CanonAimMax;b++)
						{
							Data->CanonAimDirection[b] = Data->CanonAimDirection[b + 1];
							Data->CanonAimX[b] = Data->CanonAimX[b + 1];
							Data->CanonAimY[b] = Data->CanonAimY[b + 1];
							Data->CanonAimBulletSpeed[b] = Data->CanonAimBulletSpeed[b + 1];
							Data->CanonAimBulletLiveTime[b] = Data->CanonAimBulletLiveTime[b + 1];
							Data->CanonAimBulletRestTime[b] = Data->CanonAimBulletRestTime[b + 1];
						}
						Data->CanonAimMax--;
					}
				}
			}
			//4方向キャノンのマップチップを配置するとき
			if (editMapTipNumber >= bCanon4DirectionUp && editMapTipNumber <= bCanon4DirectionDown)
			{
				if (Data->Canon4Max < EDIT_4DIRECTION_CANON_MAX)
				{
					Data->Canon4Direction[Data->Canon4Max] = editMapTipNumber;
					Data->Canon4X[Data->Canon4Max] = -Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow];
					Data->Canon4Y[Data->Canon4Max] = -Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow];
					Data->Canon4BulletSpeed[Data->Canon4Max] = Data->CanonBulletSpeedNow;
					Data->Canon4BulletLiveTime[Data->Canon4Max] = Data->CanonBulletLiveTimeNow;
					Data->Canon4BulletRestTime[Data->Canon4Max] = Data->CanonBulletRestTimeNow;
					Data->Canon4Max++;
					Data->MapData[Data->LayerNow][-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow]][-Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow]] = editMapTipNumber;
				}
			}
			//自機狙いキャノンのマップチップを配置するとき
			else if (editMapTipNumber == bCanonAim)
			{
				if (Data->CanonAimMax < EDIT_AIM_CANON_MAX)
				{
					Data->CanonAimDirection[Data->CanonAimMax] = editMapTipNumber;
					Data->CanonAimX[Data->CanonAimMax] = -Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow];
					Data->CanonAimY[Data->CanonAimMax] = -Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow];
					Data->CanonAimBulletSpeed[Data->CanonAimMax] = Data->CanonBulletSpeedNow;
					Data->CanonAimBulletLiveTime[Data->CanonAimMax] = Data->CanonBulletLiveTimeNow;
					Data->CanonAimBulletRestTime[Data->CanonAimMax] = Data->CanonBulletRestTimeNow;
					Data->CanonAimMax++;
					Data->MapData[Data->LayerNow][-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow]][-Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow]] = editMapTipNumber;
				}
			}
			else
			{
				Data->MapData[Data->LayerNow][-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow]][-Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow]] = editMapTipNumber;
			}
		}
	}
	if (Key[KEY_INPUT_BACK] >= 1)//BackSpaceキー
	{
		if (Data->PlayerStartX != -Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow] || Data->PlayerStartY != -Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow])
		{
			if (Data->MapData[Data->LayerNow][-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow]][-Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow]] != bAir)
			{
				Data->isThisStageCleared = false;
			}
			if (Data->MapData[Data->LayerNow][-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow]][-Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow]] == bCoin)
			{
				Data->CoinExist--;
			}
			//4方向キャノンのマップチップを削除するとき
			if (Data->MapData[Data->LayerNow][-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow]][-Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow]] >= bCanon4DirectionUp && Data->MapData[Data->LayerNow][-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow]][-Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow]] <= bCanon4DirectionDown)
			{
				for (a = 0;a < Data->Canon4Max;a++)
				{
					if (-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow] == Data->Canon4Y[a] && -Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow] == Data->Canon4X[a])
					{
						for (b = a;b < Data->Canon4Max;b++)
						{
							Data->Canon4Direction[b] = Data->Canon4Direction[b + 1];
							Data->Canon4X[b] = Data->Canon4X[b + 1];
							Data->Canon4Y[b] = Data->Canon4Y[b + 1];
							Data->Canon4BulletSpeed[b] = Data->Canon4BulletSpeed[b + 1];
							Data->Canon4BulletLiveTime[b] = Data->Canon4BulletLiveTime[b + 1];
							Data->Canon4BulletRestTime[b] = Data->Canon4BulletRestTime[b + 1];
						}
						Data->Canon4Max--;
					}
				}
			}
			//自機狙いキャノンのマップチップを削除するとき
			if (Data->MapData[Data->LayerNow][-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow]][-Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow]] == bCanonAim)
			{
				for (a = 0;a < Data->CanonAimMax;a++)
				{
					if (-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow] == Data->CanonAimY[a] && -Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow] == Data->CanonAimX[a])
					{
						for (b = a;b < Data->CanonAimMax;b++)
						{
							Data->CanonAimDirection[b] = Data->CanonAimDirection[b + 1];
							Data->CanonAimX[b] = Data->CanonAimX[b + 1];
							Data->CanonAimY[b] = Data->CanonAimY[b + 1];
							Data->CanonAimBulletSpeed[b] = Data->CanonAimBulletSpeed[b + 1];
							Data->CanonAimBulletLiveTime[b] = Data->CanonAimBulletLiveTime[b + 1];
							Data->CanonAimBulletRestTime[b] = Data->CanonAimBulletRestTime[b + 1];
						}
						Data->CanonAimMax--;
					}
				}
			}
			Data->MapData[Data->LayerNow][-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow]][-Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow]] = bAir;
		}
	}
	if (Key[KEY_INPUT_LSHIFT] >= 1 || Key[KEY_INPUT_RSHIFT] >= 1)//Shiftキーを押したときのスポイト機能
	{
		if (Data->PlayerStartY == -Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow] && -Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow] == Data->PlayerStartX)
		{
			editMapTipNumber = bStart;
		}
		else
		{

			for (a = 0;a < Data->Canon4Max;a++)
			{
				if (-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow] == Data->Canon4Y[a] && -Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow] == Data->Canon4X[a])
				{
					Data->CanonBulletSpeedNow = Data->Canon4BulletSpeed[a];
					Data->CanonBulletLiveTimeNow = Data->Canon4BulletLiveTime[a];
					Data->CanonBulletRestTimeNow = Data->Canon4BulletRestTime[a];
					break;
				}
			}
			for (a = 0;a < Data->CanonAimMax;a++)
			{
				if (-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow] == Data->CanonAimY[a] && -Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow] == Data->CanonAimX[a])
				{
					Data->CanonBulletSpeedNow = Data->CanonAimBulletSpeed[a];
					Data->CanonBulletLiveTimeNow = Data->CanonAimBulletLiveTime[a];
					Data->CanonBulletRestTimeNow = Data->CanonAimBulletRestTime[a];
					break;
				}
			}
			editMapTipNumber = Data->MapData[Data->LayerNow][-Data->MapY[Data->LayerNow] + Data->CursorY[Data->LayerNow]][-Data->MapX[Data->LayerNow] + Data->CursorX[Data->LayerNow]];
		}
	}

	if ((Key[KEY_INPUT_T] == 1 || (Key[KEY_INPUT_T] % editCursorScrollPerTime == 0 && Key[KEY_INPUT_T] >= editCursorScrollStart)) && Key[KEY_INPUT_G] == 0)
	{
		Data->isThisStageCleared = false;
		if (Key[KEY_INPUT_LCONTROL] >= 1 || Key[KEY_INPUT_RCONTROL] >= 1)
		{
			Data->CoinNeed += 100;
		}
		else
		{
			Data->CoinNeed++;
		}
		Data->CoinNeed = min(Data->CoinNeed, Data->CoinExist);
	}
	else if ((Key[KEY_INPUT_G] == 1 || (Key[KEY_INPUT_G] % editCursorScrollPerTime == 0 && Key[KEY_INPUT_G] >= editCursorScrollStart)) && Key[KEY_INPUT_T] == 0)
	{
		if (Key[KEY_INPUT_LCONTROL] >= 1 || Key[KEY_INPUT_RCONTROL] >= 1)
		{
			Data->CoinNeed -= 100;
		}
		else
		{
			Data->CoinNeed--;
		}
		Data->CoinNeed = max(Data->CoinNeed, 0);
	}
	if ((Key[KEY_INPUT_Y] == 1 || (Key[KEY_INPUT_Y] % editCursorScrollPerTime == 0 && Key[KEY_INPUT_Y] >= editCursorScrollStart)) && Key[KEY_INPUT_H] == 0)
	{
		Data->isThisStageCleared = false;
		if (Key[KEY_INPUT_LCONTROL] >= 1 || Key[KEY_INPUT_RCONTROL] >= 1)
		{
			Data->TimeMax += 100;
		}
		else
		{
			Data->TimeMax++;
		}
		Data->TimeMax = min(Data->TimeMax, EDIT_TIME_MAX);
	}
	else if ((Key[KEY_INPUT_H] == 1 || (Key[KEY_INPUT_H] % editCursorScrollPerTime == 0 && Key[KEY_INPUT_H] >= editCursorScrollStart)) && Key[KEY_INPUT_Y] == 0)
	{
		Data->isThisStageCleared = false;
		if (Key[KEY_INPUT_LCONTROL] >= 1 || Key[KEY_INPUT_RCONTROL] >= 1)
		{
			Data->TimeMax -= 100;
		}
		else
		{
			Data->TimeMax--;
		}
		Data->TimeMax = max(Data->TimeMax, 0);
	}

	//ステージの初期化
	if (Key[KEY_INPUT_DELETE] == 1 && (Key[KEY_INPUT_LCONTROL] >= 1 || Key[KEY_INPUT_RCONTROL] >= 1))
	{
		editModeInitialize(Data);
	}
	if (Key[KEY_INPUT_SPACE] == 1)
	{
		if (editOperation == NO)
		{
			editOperation = YES;
		}
		else
		{
			editOperation = NO;
		}
	}
	//キャノンのオプション
	if (editMapTipNumber >= bCanonAim && editMapTipNumber <= bCanon4DirectionDown)
	{
		if (Key[KEY_INPUT_U] == 1 || (Key[KEY_INPUT_U] % editCursorScrollPerTime == 0 && Key[KEY_INPUT_U] >= editCursorScrollStart))
		{
			if (Key[KEY_INPUT_LCONTROL] >= 1 || Key[KEY_INPUT_RCONTROL] >= 1)
			{
				Data->CanonBulletSpeedNow += 10;
			}
			else
			{
				Data->CanonBulletSpeedNow++;
			}
			Data->CanonBulletSpeedNow = min(Data->CanonBulletSpeedNow, EDIT_BULLET_SPEED_MAX);
		}
		else if (Key[KEY_INPUT_J] == 1 || (Key[KEY_INPUT_J] % editCursorScrollPerTime == 0 && Key[KEY_INPUT_J] >= editCursorScrollStart))
		{
			if (Key[KEY_INPUT_LCONTROL] >= 1 || Key[KEY_INPUT_RCONTROL] >= 1)
			{
				Data->CanonBulletSpeedNow -= 10;
			}
			else
			{
				Data->CanonBulletSpeedNow--;
			}
			Data->CanonBulletSpeedNow = max(Data->CanonBulletSpeedNow, 1);
		}
		if (Key[KEY_INPUT_I] == 1 || (Key[KEY_INPUT_I] % editCursorScrollPerTime == 0 && Key[KEY_INPUT_I] >= editCursorScrollStart))
		{
			if (Key[KEY_INPUT_LCONTROL] >= 1 || Key[KEY_INPUT_RCONTROL] >= 1)
			{
				Data->CanonBulletLiveTimeNow += 300;
			}
			else
			{
				Data->CanonBulletLiveTimeNow++;
			}
			Data->CanonBulletLiveTimeNow = min(Data->CanonBulletLiveTimeNow, EDIT_BULLET_LIVE_TIME_MAX);
		}
		else if (Key[KEY_INPUT_K] == 1 || (Key[KEY_INPUT_K] % editCursorScrollPerTime == 0 && Key[KEY_INPUT_K] >= editCursorScrollStart))
		{
			if (Key[KEY_INPUT_LCONTROL] >= 1 || Key[KEY_INPUT_RCONTROL] >= 1)
			{
				Data->CanonBulletLiveTimeNow -= 300;
			}
			else
			{
				Data->CanonBulletLiveTimeNow--;
			}
			Data->CanonBulletLiveTimeNow = max(Data->CanonBulletLiveTimeNow, 0);
		}
		if (Key[KEY_INPUT_O] == 1 || (Key[KEY_INPUT_O] % editCursorScrollPerTime == 0 && Key[KEY_INPUT_O] >= editCursorScrollStart))
		{
			if (Key[KEY_INPUT_LCONTROL] >= 1 || Key[KEY_INPUT_RCONTROL] >= 1)
			{
				Data->CanonBulletRestTimeNow += 300;
			}
			else
			{
				Data->CanonBulletRestTimeNow++;
			}
			Data->CanonBulletRestTimeNow = min(Data->CanonBulletRestTimeNow, EDIT_BULLET_REST_TIME_MAX);
		}
		else if (Key[KEY_INPUT_L] == 1 || (Key[KEY_INPUT_L] % editCursorScrollPerTime == 0 && Key[KEY_INPUT_L] >= editCursorScrollStart))
		{
			if (Key[KEY_INPUT_LCONTROL] >= 1 || Key[KEY_INPUT_RCONTROL] >= 1)
			{
				Data->CanonBulletRestTimeNow -= 300;
			}
			else
			{
				Data->CanonBulletRestTimeNow--;
			}
			Data->CanonBulletRestTimeNow = max(Data->CanonBulletRestTimeNow, 0);
		}
	}
}
void editModeDraw(const SaveData_t& Data, const Images_t& images) {
	//画像読み込み
	DrawBox(0, 0, 640, 480, GetColor(255, 255, 255), TRUE);    // 背景の白を描画
	for (a = -Data.MapY[Data.LayerNow]; a < -Data.MapY[Data.LayerNow] + 48; a++)//マップチップ用の画像
	{
		for (b = -Data.MapX[Data.LayerNow]; b < -Data.MapX[Data.LayerNow] + 64; b++)
		{
			if (Data.MapData[Data.LayerNow][a][b] != bAir)
			{
				DrawGraph(b * BLOCK_SIZE + Data.MapX[Data.LayerNow] * BLOCK_SIZE, a * BLOCK_SIZE + Data.MapY[Data.LayerNow] * BLOCK_SIZE, images.GmapTip[Data.MapData[Data.LayerNow][a][b]], TRUE);
			}
		}
	}
	editCursorCount++;
	if (editCursorCount <= 60 || editCursorCount % 20 < 10)
	{
		if (editMapTipNumber == bStart)
		{
			DrawGraph(Data.CursorX[Data.LayerNow] * BLOCK_SIZE, Data.CursorY[Data.LayerNow] * BLOCK_SIZE, images.GplayerTip[bBlock], TRUE);
		}
		else if (editMapTipNumber != bAir)
		{
			DrawGraph(Data.CursorX[Data.LayerNow] * BLOCK_SIZE, Data.CursorY[Data.LayerNow] * BLOCK_SIZE, images.GmapTip[editMapTipNumber], TRUE);
		}
	}
	DrawGraph(Data.CursorX[Data.LayerNow] * BLOCK_SIZE, Data.CursorY[Data.LayerNow] * BLOCK_SIZE, images.GeditTip[(editCursorCount / 40) % 4], TRUE);
	DrawBox(Data.CursorX[Data.LayerNow] * BLOCK_SIZE - 1, Data.CursorY[Data.LayerNow] * BLOCK_SIZE - 1, Data.CursorX[Data.LayerNow] * BLOCK_SIZE + 11, Data.CursorY[Data.LayerNow] * BLOCK_SIZE + 11, GetColor(255, 255, 255), FALSE);    // 四角形を描画
	DrawBox(Data.CursorX[Data.LayerNow] * BLOCK_SIZE - 3, Data.CursorY[Data.LayerNow] * BLOCK_SIZE - 3, Data.CursorX[Data.LayerNow] * BLOCK_SIZE + 13, Data.CursorY[Data.LayerNow] * BLOCK_SIZE + 13, GetColor(0, 0, 0), FALSE);    // 四角形を描画
	DrawGraph((Data.PlayerStartX + Data.MapX[Data.LayerNow]) * BLOCK_SIZE, (Data.PlayerStartY + Data.MapY[Data.LayerNow]) * BLOCK_SIZE, images.GplayerTip[bBlock], TRUE);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);		//ブレンドモードをα(200/255)に設定
	DrawBox(500, 470, 610, 480, GetColor(255, 255, 255), TRUE);
	if (editOperation == NO)
	{
		DrawBox(0, 0, 640, 10, GetColor(255, 255, 255), TRUE);
		DrawBox(0, 20, 50, 40, GetColor(255, 255, 255), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);		//ブレンドモードをオフ
		if (Data.isThisStageCleared)
		{
			DrawFormatString(385, 0, GetColor(255, 0, 0), "ステージクリアされています。");
		}
		else
		{
			DrawFormatString(385, 0, GetColor(255, 0, 0), "ステージクリアされていません。");
		}
	}
	else
	{
		DrawBox(0, 60, 210, 370, GetColor(255, 255, 255), TRUE);
		DrawBox(0, 0, 640, 40, GetColor(255, 255, 255), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);		//ブレンドモードをオフ
		DrawFormatString(0, 60, GetColor(255, 0, 0), "<2Dステージツクール操作説明>\n~ステージ作成モード~\n[Q], [A]　マップチップ変更\n[E], [S], [D], [F]　カーソル移動\n[T], [G]　コインアイテム必要数変更\n(コインアイテムを配置しないと変更出来ない)\n[Y], [H]　制限時間変更\n(ctrl押しながらで100ずつ変更)\n[Z]　セーブウィンドウを開く\n(ファイル名の末尾に.datと付けて保存)\n[X]　ロードウィンドウを開く\n[shift]　スポイト機能\n[Enter]　マップチップ配置\n[back space]　マップチップを削除\n[Tab]プレイモードに変更\n[ESC]　ゲームを強制終了\n[Ctrl] + [delete]　マップ全消し\n\n~プレイモード~\n[S], [F]または[←], [→]　左右移動\n[E]または[↑]　ジャンプ\n[ENTER]　リトライ\n[Tab]　ステージ作成モードに変更\n[esc]　ゲームを強制終了\n\nスペースキーで操作説明終了");
		DrawFormatString(385, 0, GetColor(255, 0, 0), "←選択しているマップチップの説明");
		DrawFormatString(580, 0, GetColor(255, 0, 0), "FPS→");
		DrawFormatString(0, 10, GetColor(255, 0, 0), "↑現在のマップチップカーソルの座標");
		DrawFormatString(70, 20, GetColor(255, 0, 0), "←ステージ上にあるコインアイテム/集めるコインアイテムの必要数");
		DrawFormatString(70, 30, GetColor(255, 0, 0), "←制限時間　0に設定すると制限時間が無制限となる");
	}
	DrawGraph(0, 20, images.GmapTip[bCoin], TRUE);
	//文字描写
	DrawFormatString(0, 0, GetColor(255, 0, 0), "X=%d,Y=%d", -Data.MapX[Data.LayerNow] + Data.CursorX[Data.LayerNow], MAP_Y_MAX - 1 + Data.MapY[Data.LayerNow] - Data.CursorY[Data.LayerNow]);
	DrawFormatString(500, 470, GetColor(255, 0, 0), "スペースキーで操作説明");
	DrawFormatString(10, 20, GetColor(255, 0, 0), "%d/%d", Data.CoinExist, Data.CoinNeed);
	DrawFormatString(0, 30, GetColor(255, 0, 0), "Time:%d", Data.TimeMax);
	if (editMapTipNumber >= bCanonAim && editMapTipNumber <= bCanon4DirectionDown)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);		//ブレンドモードをα(128/255)に設定
		DrawBox(0, 380, 190, 475, GetColor(255, 255, 255), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);		//ブレンドモードをオフ
		DrawFormatString(0, 380, GetColor(255, 0, 0), "~キャノンのオプション~\n[U],[J]　弾の速さ→%d\n(プレイヤーの最大速度は50)\n[I],[K]　弾の残る時間→%d\n(0に設定すると黒壁に当たるまで\n消えない・氷を溶かす設定になる)\n[O],[L]　弾を次に撃つまでの待機時間→%d\n(時間は60カウントで1秒となる。)", Data.CanonBulletSpeedNow, Data.CanonBulletLiveTimeNow, Data.CanonBulletRestTimeNow);
	}
	switch (editMapTipNumber) {
	case bAir:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "空気であり何も配置しない。すなわち虚無。");
		break;
	case bBlock:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "四角い壁。速く走れば1マス抜きでも渡れる。");
		break;
		//case 1:
		//	DrawFormatString(80, 0, GetColor(255, 0, 0), "右上の坂。");
		//	break;
		//case 2:
		//	DrawFormatString(80, 0, GetColor(255, 0, 0), "左上の坂。");
		//	break;
	case bBridge:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "すり抜け足場。下からすり抜けられて、上からはすり抜けられない。");
		break;
	case bIceBlock:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "氷の壁。動摩擦係数が0なので永遠に止まりません。");
		break;
	case bCanonAim:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "自機狙いキャノンあと%d個まで置くことが出来る。", EDIT_AIM_CANON_MAX - Data.CanonAimMax);
		break;
	case bCanon4DirectionUp:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "上向きキャノン。あと%d個まで置くことが出来る。", EDIT_4DIRECTION_CANON_MAX - Data.Canon4Max);
		break;
	case bCanon4DirectionLeft:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "左向きキャノン。あと%d個まで置くことが出来る。", EDIT_4DIRECTION_CANON_MAX - Data.Canon4Max);
		break;
	case bCanon4DirectionRight:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "右向きキャノン。あと%d個まで置くことが出来る。", EDIT_4DIRECTION_CANON_MAX - Data.Canon4Max);
		break;
	case bCanon4DirectionDown:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "下向きキャノン。あと%d個まで置くことが出来る。", EDIT_4DIRECTION_CANON_MAX - Data.Canon4Max);
		break;
	case bDamageBlock:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "ダメージ壁。触れるとライフが1減る。");
		break;
	case bDieBlock:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "即死壁。触れるとどんなにライフがあっても死ぬ。");
		break;
	case bConveyorUpS:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "上向きコンベア弱。触れてる間は上向きに固定移動する。");
		break;
	case bConveyorUpM:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "上向きコンベア中。縦に連続して配置しよう。");
		break;
	case bConveyorUpL:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "上向きコンベア強。まるで上向きに風でも吹いているような感じ。");
		break;
	case bConveyorLeftS:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "左向きコンベア弱。触れてる間は左向きに固定移動する。");
		break;
	case bConveyorLeftM:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "左向きコンベア中。横に連続して配置しよう。");
		break;
	case bConveyorLeftL:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "左向きコンベア強。まるで左向きに風でも吹いているような感じ。");
		break;
	case bConveyorRightS:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "右向きコンベア弱。触れてる間は右向きに固定移動する。");
		break;
	case bConveyorRightM:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "右向きコンベア中。横に連続して配置しよう。");
		break;
	case bConveyorRightL:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "右向きコンベア強。まるで右向きに風でも吹いているような感じ。");
		break;
	case bGoal:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "ゴール。触れるとゲームクリア。");
		break;
	case bHeart:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "ハートアイテム。取るとライフが1つ増える。");
		break;
	case bCoin:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "コインアイテム。これを一定数取るとゲームクリア。");
		break;
	case bJump:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "ジャンプアイテム。取ると最大空中ジャンプ回数が増える。");
		break;
	case bStart:
		DrawFormatString(80, 0, GetColor(255, 0, 0), "スタート地点。ステージに1つしか置けない。");
		break;
	}
}