#include "DxLib.h"
#include "stdlib.h"
#include <stdio.h>
#include <math.h>
#include <cmath>//absを使用するため
#include "definitions.h"

static double playerX, playerY;
static double playerSpeed, playerSpeedMax, playerAccerelation = 0.2, playerDeceleration = PLAYER_DECELERATION, playerGravity = 0.2, playerJumpPower = 4, playerFallVelocity = 0, playerFallVelocityMax = 5;
static double playerConveyorL = 0, playerConveyorR = 0, playerConveyorU = 0;//プレイヤーがコンベアによって動く距離を代入する。
static double playerConveyorLR = 0;//playerConveyorL + playerConveyorR
static int playerOnIce = NO;//プレイヤーが氷の壁の上に立っているのならば、playerDecelerationを実行しない。
static int playerJumpTime = 0, playerJumpTimeMax = 30;//JumpTimeはジャンプボタンを押す時間によってジャンプする高さが変わる。
static int playerJumpCount = 0, playerJumpMax = 10;//JumpCountは現在のジャンプした回数。JumpMaxは最大ジャンプ可能数。
static int playerDirection = RIGHT;//プレイヤーの向き。LEFT,RIGHTのみを使う。最後に押された方を記憶しておくためにも使用する。
static int playerWalkG = STAND;//プレイヤーの歩く画像の値。0,5,10,15,20のみを使う。
static int playerWalkCount = 0;//プレイヤーの歩く画像を変えるための設定。
static int playerWalkCountMax = 5;//プレイヤーの歩く画像を変えるまでの値。
static int playerLife = 1;//プレイヤーの体力
static int playerIsDeath = NO;//プレイヤーが死んだときにYESに変える
static int playerInvincibleTime = 0;//プレイヤーの無敵時間。1~playerInvincibleMaxまでが無敵時間
static int playerInvincibleTimeMax = 120;
static int playerIsDamaged = NO;
static int playerCoin = 0, playerCoinMax = 3;//プレイヤーのコイン
static int playerTime = 0, playerTimeMax = 999, playerTimeCount = 0;//playerTimeCountは、60でplayerTimeを1減らす
															 //プレイモードのステージに使用する変数
static char Canon4BulletExist[EDIT_AIM_CANON_MAX] = { 0 };//0で無い,1で有る
static unsigned int CanonBulletTime = 0;//プレイモード時スタートから1フレーム毎に1増やす
static short int Canon4Time[EDIT_4DIRECTION_CANON_MAX] = { 0 };//キャノンのLiveTimeが0の時にRestTime内にあるのかを調べるときに使う
static double Canon4X[EDIT_4DIRECTION_CANON_MAX] = { 0 };
static double Canon4Y[EDIT_4DIRECTION_CANON_MAX] = { 0 };
static double Canon4BulletX[EDIT_4DIRECTION_CANON_MAX] = { 0 };
static double Canon4BulletY[EDIT_4DIRECTION_CANON_MAX] = { 0 };

static char CanonAimBulletExist[EDIT_AIM_CANON_MAX] = { 0 };//0で無い,1で有る
static short int CanonAimTime[EDIT_4DIRECTION_CANON_MAX] = { 0 };//キャノンのLiveTimeが0の時にRestTime内にあるのかを調べるときに使う
static double CanonAimSin[EDIT_AIM_CANON_MAX] = { 0 };
static double CanonAimCos[EDIT_AIM_CANON_MAX] = { 0 };
static double radius;//ラジアンを一時的に保存する
static double CanonAimX[EDIT_4DIRECTION_CANON_MAX] = { 0 };
static double CanonAimY[EDIT_4DIRECTION_CANON_MAX] = { 0 };
static double CanonAimBulletX[EDIT_4DIRECTION_CANON_MAX] = { 0 };
static double CanonAimBulletY[EDIT_4DIRECTION_CANON_MAX] = { 0 };

static int a, b, c, d, e;
static double mapX = 0, mapY = 0;//マップの左端、マップの上端をそれぞれ示している。
static double mapXadjust = 0, mapYadjust = 0;//プレイヤーを中心に、ステージを動かすときに使用する。
static int mapXhit = 0, mapYhit = 0;//マップの当たるブロックのマス目の値。それぞれ±1まであたり判定をする。
static int playerXtouch = NO, playerYtouch = NO;//プレイヤーがブロックに当たったかを判定する。ブロックに当たっていなければ(NOならば)移動する。
char mapData[EDIT_LAYER_MAX][MAP_Y_MAX][MAP_X_MAX] = { 0 };//0■壁,1右上坂,2左上坂,3すり抜け足場,4氷足場,5動く壁静止,6動く壁上,7動く壁左,8動く壁右,9動く壁下,10ダメージ壁■,11即死針,12ジャンプ台小,13ジャンプ台中,14ジャンプ台強,15左コンベア小,16左コンベア中,17左コンベア大,18右コンベア小,19右コンベア中,20右コンベア強,21扉,22ハート,23お金,24青い○,
bool gameClearFlag = false;

//効果音用変数宣言
static int soundPlayerJumpLanding; //プレイヤーが着地したときの効果音
static int soundPlayerJump; //プレイヤーがジャンプしたときの効果音
static int soundPlayerDamaged; //プレイヤーがダメージをくらったときの効果音
static int soundPlayerLifeUp; //プレイヤーがライフアイテムを取ったときの効果音
static int soundPlayerJumpMaxUp; //プレイヤーがジャンプアイテムを取ったときの効果音
static int soundPlayerCoinGet; //プレイヤーがコインアイテムを取ったときの効果音
static int soundPlayerDeath; //プレイヤーが死んだときの効果音
static int soundPlayerStart; //ステージが始まったときの効果音
void playerMoveY(double *dy, const SaveData_t &Data, bool isPlayerMove)//第3引数はプレイヤー移動ならtrue,コンベア移動ならfalse
{
	if (playerX + PLAYER_WIDTH / 2 > mapX + a * BLOCK_SIZE
		&& playerX - PLAYER_WIDTH / 2 < mapX + (a+1) * BLOCK_SIZE
		&& playerY + PLAYER_TALL / 2 + *dy > mapY + b * BLOCK_SIZE
		&& playerY - PLAYER_TALL / 2 + *dy < mapY + b * BLOCK_SIZE + BLOCK_SIZE)
	{
		if (mapData[Data.LayerNow][b][a] == bBlock || mapData[Data.LayerNow][b][a] == bIceBlock || mapData[Data.LayerNow][b][a] == bCanonAim || mapData[Data.LayerNow][b][a] == bCanon4DirectionUp || mapData[Data.LayerNow][b][a] == bCanon4DirectionLeft || mapData[Data.LayerNow][b][a] == bCanon4DirectionRight || mapData[Data.LayerNow][b][a] == bCanon4DirectionDown || mapData[Data.LayerNow][b][a] == bDamageBlock || mapData[Data.LayerNow][b][a] == bDieBlock)
		{
			if (*dy >= 0)
			{
				playerY = mapY + b * BLOCK_SIZE - PLAYER_TALL / 2;
				playerYtouch = YES;
				*dy = 0;
				//下に移動してブロックに当たりました。
				if (playerJumpCount != 0)
				{
					playerJumpCount = 0;
					PlaySoundMem(soundPlayerJumpLanding, DX_PLAYTYPE_BACK);
				}
				if (mapData[Data.LayerNow][b][a] == bIceBlock)
				{
					playerDeceleration = 1;//氷の上で滑る設定
				}
			}
			if (*dy < 0)
			{
				playerY = mapY + (b+1) * BLOCK_SIZE + PLAYER_TALL / 2;
				playerYtouch = YES;
				*dy = 0;
				//上に移動してブロックに当たりました。
			}
		}
		if (mapData[Data.LayerNow][b][a] == 3 && playerY + PLAYER_TALL / 2 <= mapY + b * BLOCK_SIZE)
		{
			if (*dy >= 0)
			{
				playerY = mapY + b * BLOCK_SIZE - PLAYER_TALL / 2;
				playerYtouch = YES;
				*dy = 0;
				//下に移動してブロックに当たりました。
				if (playerJumpCount != 0)
				{
					playerJumpCount = 0;
					PlaySoundMem(soundPlayerJumpLanding, DX_PLAYTYPE_BACK);
				}
			}
		}
		if (mapData[Data.LayerNow][b][a] == bDamageBlock)//ダメージ壁
		{
			playerIsDamaged = YES;
		}
		if (mapData[Data.LayerNow][b][a] == bGoal)//ゴール扉
		{
			gameClearFlag = true;
		}
		if (mapData[Data.LayerNow][b][a] == bHeart)//ライフ
		{
			playerLife++;
			mapData[Data.LayerNow][b][a] = -1;
			PlaySoundMem(soundPlayerLifeUp, DX_PLAYTYPE_BACK);
		}
		if (mapData[Data.LayerNow][b][a] == bCoin)//コイン
		{
			playerCoin++;
			mapData[Data.LayerNow][b][a] = -1;
			PlaySoundMem(soundPlayerCoinGet, DX_PLAYTYPE_BACK);
		}
		if (mapData[Data.LayerNow][b][a] == bJump)//ジャンプ
		{
			playerJumpMax++;
			mapData[Data.LayerNow][b][a] = -1;
			PlaySoundMem(soundPlayerJumpMaxUp, DX_PLAYTYPE_BACK);
		}
		if (mapData[Data.LayerNow][b][a] == bDieBlock)//即死針
		{
			playerLife = 0;
		}
		if (isPlayerMove)
		{
			if (mapData[Data.LayerNow][b][a] == bConveyorUpS)//コンベア上
			{
				playerConveyorU = min(playerConveyorU, EDIT_CONVEYOR_UP_S);
			}
			if (mapData[Data.LayerNow][b][a] == bConveyorUpM)//コンベア上
			{
				playerConveyorU = min(playerConveyorU, EDIT_CONVEYOR_UP_M);
			}
			if (mapData[Data.LayerNow][b][a] == bConveyorUpL)//コンベア上
			{
				playerConveyorU = min(playerConveyorU, EDIT_CONVEYOR_UP_L);
			}
			if (mapData[Data.LayerNow][b][a] == bConveyorLeftS)//コンベア左
			{
				playerConveyorL = min(playerConveyorL, -EDIT_CONVEYOR_LR_S);
			}
			if (mapData[Data.LayerNow][b][a] == bConveyorLeftM)//コンベア左
			{
				playerConveyorL = min(playerConveyorL, -EDIT_CONVEYOR_LR_M);
			}
			if (mapData[Data.LayerNow][b][a] == bConveyorLeftL)//コンベア左
			{
				playerConveyorL = min(playerConveyorL, -EDIT_CONVEYOR_LR_L);
			}
			if (mapData[Data.LayerNow][b][a] == bConveyorRightS)//コンベア右
			{
				playerConveyorR = max(playerConveyorR, EDIT_CONVEYOR_LR_S);
			}
			if (mapData[Data.LayerNow][b][a] == bConveyorRightM)//コンベア右
			{
				playerConveyorR = max(playerConveyorR, EDIT_CONVEYOR_LR_M);
			}
			if (mapData[Data.LayerNow][b][a] == bConveyorRightL)//コンベア右
			{
				playerConveyorR = max(playerConveyorR, EDIT_CONVEYOR_LR_L);
			}
		}
	}
}
void playerMoveX(double *dx, const SaveData_t &Data)
{
	if (playerX + PLAYER_WIDTH / 2 + *dx > mapX + a * BLOCK_SIZE
		&& playerX - PLAYER_WIDTH / 2 + *dx < mapX + (a+1) * BLOCK_SIZE
		&& playerY + PLAYER_TALL / 2 >mapY + b * BLOCK_SIZE
		&& playerY - PLAYER_TALL / 2 < mapY + (b+1) * BLOCK_SIZE)
	{
		if (mapData[Data.LayerNow][b][a] == bBlock || mapData[Data.LayerNow][b][a] == bIceBlock || mapData[Data.LayerNow][b][a] == bCanonAim || mapData[Data.LayerNow][b][a] == bCanon4DirectionUp || mapData[Data.LayerNow][b][a] == bCanon4DirectionLeft || mapData[Data.LayerNow][b][a] == bCanon4DirectionRight || mapData[Data.LayerNow][b][a] == bCanon4DirectionDown || mapData[Data.LayerNow][b][a] == bDamageBlock || mapData[Data.LayerNow][b][a] == bDieBlock)
		{
			if (*dx > 0)
			{
				playerX = mapX + a * BLOCK_SIZE - (PLAYER_WIDTH / 2);
				playerXtouch = YES;
				playerSpeed = 0;
				playerConveyorLR = 0;
				//右に移動してブロックに当たりました。
			}
			else if (*dx < 0)
			{
				playerX = mapX + (a+1) * BLOCK_SIZE + (PLAYER_WIDTH / 2);
				playerXtouch = YES;
				playerSpeed = 0;
				playerConveyorLR = 0;
				//左に移動してブロックに当たりました。
			}
		}
		if (mapData[Data.LayerNow][b][a] == bDamageBlock)
		{
			playerIsDamaged = YES;
		}
		if (mapData[Data.LayerNow][b][a] == bGoal)//ゴール扉
		{
			gameClearFlag = true;
		}
		if (mapData[Data.LayerNow][b][a] == bHeart)//ライフ
		{
			playerLife++;
			mapData[Data.LayerNow][b][a] = -1;
			PlaySoundMem(soundPlayerLifeUp, DX_PLAYTYPE_BACK);
		}
		if (mapData[Data.LayerNow][b][a] == bCoin)//コイン
		{
			playerCoin++;
			mapData[Data.LayerNow][b][a] = -1;
			PlaySoundMem(soundPlayerCoinGet, DX_PLAYTYPE_BACK);
		}
		if (mapData[Data.LayerNow][b][a] == bJump)//ジャンプ
		{
			playerJumpMax++;
			mapData[Data.LayerNow][b][a] = -1;
			PlaySoundMem(soundPlayerJumpMaxUp, DX_PLAYTYPE_BACK);
		}
		if (mapData[Data.LayerNow][b][a] == bDieBlock)//即死針
		{
			playerLife = 0;
		}
	}
}
void playModeInitialize(void) {
	playerX = 0, playerY = 0;//playerの中心を示している。PLAYER_TALL/2,PLAYER_WIDTH/2を足したりして使う。
	playerSpeed = 0, playerSpeedMax = 5, playerAccerelation = 0.2, playerDeceleration = PLAYER_DECELERATION, playerGravity = 0.2, playerJumpPower = 4, playerFallVelocity = 0, playerFallVelocityMax = 5;
	playerConveyorL = 0, playerConveyorR = 0, playerConveyorU = 0;//プレイヤーがコンベアによって動く距離を代入する。
	playerOnIce = NO;//プレイヤーが氷の壁の上に立っているのならば、playerDecelerationを実行しない。
	playerJumpTime = 0, playerJumpTimeMax = 30;//JumpTimeはジャンプボタンを押す時間によってジャンプする高さが変わる。
	playerJumpCount = 0, playerJumpMax = 10;//JumpCountは現在のジャンプした回数。JumpMaxは最大ジャンプ可能数。
	playerDirection = RIGHT;//プレイヤーの向き。LEFT,RIGHTのみを使う。最後に押された方を記憶しておくためにも使用する。
	playerWalkG = STAND;//プレイヤーの歩く画像の値。0,5,10,15,20のみを使う。
	playerWalkCount = 0;//プレイヤーの歩く画像を変えるための設定。
	playerWalkCountMax = 5;//プレイヤーの歩く画像を変えるまでの値。
	playerLife = 1;//プレイヤーの体力
	playerIsDeath = NO;//プレイヤーが死んだときにYESに変える
	playerInvincibleTime = 0;//プレイヤーの無敵時間。1~playerInvincibleMaxまでが無敵時間
	playerInvincibleTimeMax = 120;
	playerIsDamaged = NO;
	playerCoin = 0, playerCoinMax = 3;//プレイヤーのコイン
	playerTime = 0, playerTimeMax = 999, playerTimeCount = 0;//playerTimeCountは、60でplayerTimeを1減らす																	   

	mapX = 0, mapY = 0;//マップの左端、マップの上端をそれぞれ示している。
	mapXadjust = 0, mapYadjust = 0;//プレイヤーを中心に、ステージを動かすときに使用する。
	mapXhit = 0, mapYhit = 0;//マップの当たるブロックのマス目の値。それぞれ±1まであたり判定をする。
	playerXtouch = NO, playerYtouch = NO;//プレイヤーがブロックに当たったかを判定する。ブロックに当たっていなければ(NOならば)移動する。
	//音読み込み
	soundPlayerJumpLanding = LoadSoundMem("音声/playerJumpLanding.mp3"); //プレイヤーが着地したときの効果音
	soundPlayerJump = LoadSoundMem("音声/playerJump.mp3"); //プレイヤーがジャンプしたときの効果音
	soundPlayerDamaged = LoadSoundMem("音声/playerDamaged.mp3"); //プレイヤーがダメージをくらったときの効果音
	soundPlayerLifeUp = LoadSoundMem("音声/playerLifeUp.mp3"); //プレイヤーがライフアイテムを取ったときの効果音
	soundPlayerJumpMaxUp = LoadSoundMem("音声/playerJumpMaxUp.mp3"); //プレイヤーがジャンプアイテムを取ったときの効果音
	soundPlayerCoinGet = LoadSoundMem("音声/playerCoinGet.mp3"); //プレイヤーがコインアイテムを取ったときの効果音
	soundPlayerDeath = LoadSoundMem("音声/playerDeath.mp3"); //プレイヤーが死んだときの効果音
	soundPlayerStart = LoadSoundMem("音声/playerStart.mp3"); //ステージが始まったときの効果音														   
	//音量調整
	ChangeVolumeSoundMem(255 * 50 / 100, soundPlayerJumpLanding);
	ChangeVolumeSoundMem(255 * 50 / 100, soundPlayerJump);
	ChangeVolumeSoundMem(255 * 50 / 100, soundPlayerDamaged);
	ChangeVolumeSoundMem(255 * 50 / 100, soundPlayerLifeUp);
	ChangeVolumeSoundMem(255 * 50 / 100, soundPlayerJumpMaxUp);
	ChangeVolumeSoundMem(255 * 50 / 100, soundPlayerCoinGet);
	ChangeVolumeSoundMem(255 * 50 / 100, soundPlayerDeath);
	ChangeVolumeSoundMem(255 * 50 / 100, soundPlayerStart);
}
void playModeDataInput(const SaveData_t& Data) {
	playerX = Data.PlayerStartX * BLOCK_SIZE + PLAYER_WIDTH / 2 + 2;
	playerY = Data.PlayerStartY * BLOCK_SIZE + PLAYER_TALL / 2;
	mapX = 0;
	mapY = 0;
	for (a = 0; a < Data.Canon4Max; a++)
	{
		Canon4X[a] = Data.Canon4X[a] * BLOCK_SIZE;
		Canon4Y[a] = Data.Canon4Y[a] * BLOCK_SIZE;
	}
	for (a = 0; a < Data.CanonAimMax; a++)
	{
		CanonAimX[a] = Data.CanonAimX[a] * BLOCK_SIZE;
		CanonAimY[a] = Data.CanonAimY[a] * BLOCK_SIZE;
	}
	//スタート地点が右端か下端にいるときに調整するプログラム
	if (-playerX + MAP_X_MAX * BLOCK_SIZE - 640 < 0)
	{
		playerX -= MAP_X_MAX * BLOCK_SIZE - 640;
		mapX -= MAP_X_MAX * BLOCK_SIZE - 640;
	}
	if (-playerY + MAP_Y_MAX * BLOCK_SIZE - 480 < 0)
	{
		playerY -= mapY + MAP_Y_MAX * BLOCK_SIZE - 480;
		mapY -= MAP_Y_MAX * BLOCK_SIZE - 480;
		for (a = 0; a < Data.Canon4Max; a++)
		{
			Canon4Y[a] -= mapY + MAP_Y_MAX * BLOCK_SIZE - 480;
		}
		for (a = 0; a < Data.CanonAimMax; a++)
		{
			CanonAimY[a] -= mapY + MAP_Y_MAX * BLOCK_SIZE - 480;
		}
	}
	playerLife = 1;
	playerJumpMax = 1;
	playerJumpCount = 1;
	playerCoin = 0;
	playerCoinMax = Data.CoinNeed;
	playerIsDeath = NO;
	playerTimeMax = Data.TimeMax;
	playerTime = playerTimeMax;
	playerSpeed = 0;
	playerFallVelocity = 0;
	playerInvincibleTime = 0;
	//キャノン関係の初期化
	for (a = 0; a < EDIT_4DIRECTION_CANON_MAX; a++)
	{
		Canon4BulletExist[a] = 0;
		Canon4Time[a] = 0;
	}
	for (a = 0; a < EDIT_AIM_CANON_MAX; a++)
	{
		CanonAimBulletExist[a] = 0;
		CanonAimTime[a] = 0;
	}
	CanonBulletTime = 0;//プレイモード時スタートから1フレーム毎に1増やす
	PlaySoundMem(soundPlayerStart, DX_PLAYTYPE_BACK);
	for (a = 0; a < EDIT_LAYER_MAX - 1; a++)
	{
		for (b = 0; b < MAP_Y_MAX; b++)
		{
			for (c = 0; c < MAP_X_MAX; c++)
			{
				mapData[a][b][c] = Data.MapData[a][b][c];
			}
		}
	}
	gameClearFlag = false;
}
bool play(const SaveData_t& Data, const int(&Key)[256]) {
	SetDrawScreen(DX_SCREEN_BACK);
	if (playerLife > 0)
	{
		//マップスクロール関係のプログラム
		if (playerX >= 320 && mapX + MAP_X_MAX * BLOCK_SIZE - 640 > 0)
		{
			mapXadjust = playerX - 320;
			playerX = 320;
			mapX -= mapXadjust;
			for (a = 0;a < Data.Canon4Max;a++)
			{
				Canon4BulletX[a] -= mapXadjust;
			}
			for (a = 0;a < Data.CanonAimMax;a++)
			{
				CanonAimBulletX[a] -= mapXadjust;
			}
			if (mapX + MAP_X_MAX * BLOCK_SIZE - 640 < 0)
			{
				playerX += mapX + MAP_X_MAX * BLOCK_SIZE - 640;
				for (a = 0;a < Data.Canon4Max;a++)
				{
					Canon4BulletX[a] -= mapX + MAP_X_MAX * BLOCK_SIZE - 640;
				}
				for (a = 0;a < Data.CanonAimMax;a++)
				{
					CanonAimBulletX[a] -= mapX + MAP_X_MAX * BLOCK_SIZE - 640;
				}
				mapX = -(MAP_X_MAX * BLOCK_SIZE - 640);
			}
		}
		else if (playerX <= 320 && mapX < 0)
		{
			mapXadjust = playerX - 320;
			playerX = 320;
			mapX -= mapXadjust;
			for (a = 0;a < Data.Canon4Max;a++)
			{
				Canon4BulletX[a] -= mapXadjust;
			}
			for (a = 0;a < Data.CanonAimMax;a++)
			{
				CanonAimBulletX[a] -= mapXadjust;
			}
			if (mapX > 0)
			{
				playerX += mapX;
				for (a = 0;a < Data.Canon4Max;a++)
				{
					Canon4BulletX[a] -= mapX;
				}
				for (a = 0;a < Data.CanonAimMax;a++)
				{
					CanonAimBulletX[a] -= mapX;
				}
				mapX = 0;
			}
		}

		if (playerY >= 240 && mapY + MAP_Y_MAX * BLOCK_SIZE - 480 > 0)
		{
			mapYadjust = playerY - 240;
			playerY = 240;
			mapY -= mapYadjust;
			for (a = 0;a < Data.Canon4Max;a++)
			{
				Canon4BulletY[a] -= mapYadjust;
			}
			for (a = 0;a < Data.CanonAimMax;a++)
			{
				CanonAimBulletY[a] -= mapYadjust;
			}
			if (mapY + MAP_Y_MAX * BLOCK_SIZE - 480 < 0)
			{
				playerY += mapY + MAP_Y_MAX * BLOCK_SIZE - 480;
				for (a = 0;a < Data.Canon4Max;a++)
				{
					Canon4BulletY[a] -= mapY + MAP_Y_MAX * BLOCK_SIZE - 480;
				}
				for (a = 0;a < Data.CanonAimMax;a++)
				{
					CanonAimBulletY[a] -= mapY + MAP_Y_MAX * BLOCK_SIZE - 480;
				}
				mapY = -(MAP_Y_MAX * BLOCK_SIZE - 480);
			}
		}
		else if (playerY <= 240 && mapY < 0)
		{
			mapYadjust = playerY - 240;
			playerY = 240;
			mapY -= mapYadjust;
			for (a = 0;a < Data.Canon4Max;a++)
			{
				Canon4BulletY[a] -= mapYadjust;
			}
			for (a = 0;a < Data.CanonAimMax;a++)
			{
				CanonAimBulletY[a] -= mapYadjust;
			}
			if (mapY > 0)
			{
				playerY += mapY;
				for (a = 0;a < Data.Canon4Max;a++)
				{
					Canon4BulletY[a] -= mapY;
				}
				for (a = 0;a < Data.CanonAimMax;a++)
				{
					CanonAimBulletY[a] -= mapY;
				}
				mapY = 0;
			}
		}
		if (playerTimeMax > 0)
		{
			playerTimeCount++;
			if (playerTimeCount >= 60)
			{
				playerTime--;
				playerTimeCount = 0;
			}
			if (playerTime <= 0)
			{
				playerLife = 0;
			}
		}
		//プレイヤーの設定
		if (Key[KEY_INPUT_S] >= 1 || Key[KEY_INPUT_LEFT] >= 1)
		{
			if (playerSpeed > -playerSpeedMax)
			{
				playerSpeed -= playerAccerelation;
			}
			else
			{
				playerSpeed = -playerSpeedMax;
			}
			//減速する時だけいつもよりも早く方向転換できる。
			if (playerSpeed > 0)
			{
				playerSpeed *= playerDeceleration;
				if (playerDeceleration == 1)
				{
					playerSpeed += playerAccerelation / 2;
				}
			}
			playerWalkCount += int(abs(playerSpeed) + 1);//歩く速さで、アニメーションのスピードを変える。
			playerDirection = LEFT;
		}
		else if (Key[KEY_INPUT_F] >= 1 || Key[KEY_INPUT_RIGHT] >= 1)
		{
			if (playerSpeed < playerSpeedMax)
			{
				playerSpeed += playerAccerelation;
			}
			else
			{
				playerSpeed = playerSpeedMax;
			}
			//減速する時だけいつもよりも早く方向転換できる。
			if (playerSpeed < 0)
			{
				playerSpeed *= playerDeceleration;
				if (playerDeceleration == 1)
				{
					playerSpeed -= playerAccerelation / 2;
				}
			}
			playerWalkCount += int(abs(playerSpeed) + 1);//歩く速さで、アニメーションのスピードを変える。
			playerDirection = RIGHT;
		}
		else
		{
			playerSpeed *= playerDeceleration;
			playerWalkCount = 0;
		}
		//ジャンプ関係のプログラム(移動はしない)
		if ((Key[KEY_INPUT_E] == 1 || Key[KEY_INPUT_UP] == 1) && playerJumpCount < playerJumpMax&&playerJumpTime == 0)
		{
			playerJumpCount++;
			playerJumpTime = 0;
			playerFallVelocity = -1 * playerJumpPower;
			PlaySoundMem(soundPlayerJump, DX_PLAYTYPE_BACK);
		}
		else if ((Key[KEY_INPUT_E] >= 1 || Key[KEY_INPUT_UP] >= 1) && playerJumpCount < playerJumpMax&&playerJumpTime >= 1
			&& playerJumpTime < playerJumpTimeMax)
		{
			playerFallVelocity = -1 * playerJumpPower;
			playerJumpTime++;
		}
		else
		{
			playerJumpTime = 0;
			if (playerFallVelocity < playerFallVelocityMax)
			{
				playerFallVelocity += playerGravity;
			}
		}
		//マップの当たり判定。処理を減らすためにプレイヤーの近くのマップのみ当たり判定をする。
		mapXhit = int((playerX - mapX) / BLOCK_SIZE);
		mapYhit = int((playerY - mapY) / BLOCK_SIZE);
		playerXtouch = NO;
		playerYtouch = NO;
		playerIsDamaged = NO;
		playerOnIce = NO;
		playerDeceleration = PLAYER_DECELERATION;//氷の壁の上に乗っている場合にはplayerDecelerationを1にする。
		//下記の判定でブロックに当たっていればplayerXtouchをYESに変える。
		for (b = mapYhit - 1; b <= mapYhit + 1; b++)//当たり判定の移動の関係で下から順番に当たり判定を計算していく。
		{
			for (a = mapXhit - 1; a <= mapXhit + 1; a++)
			{
				playerMoveY(&playerFallVelocity, Data, true);//第3引数はプレイヤー移動ならtrue,コンベア移動ならfalse
				playerMoveX(&playerSpeed, Data);
			}
		}
		if (playerXtouch == NO)
		{
			playerX += playerSpeed;
		}
		if (playerYtouch == NO)
		{
			playerY += playerFallVelocity;
			//ジャンプしないで壁から降りたとき
			if (playerJumpCount == 0)
			{
				playerJumpCount = 1;
			}
		}
		//キャノンの弾の移動と当たり判定
		//4方向キャノン
		for (a = 0; a < Data.Canon4Max; a++)
		{
			//弾の残る時間を0にしたとき、つまりプレイヤーや壁に当たるまで消えないキャノン
			if (Data.Canon4BulletLiveTime[a] == 0)
			{
				//弾が存在しないとき
				if (Canon4BulletExist[a] == NO)
				{
					Canon4Time[a]++;
					if (Canon4Time[a] >= Data.Canon4BulletRestTime[a])
					{
						Canon4BulletExist[a] = YES;
						Canon4BulletX[a] = Data.Canon4X[a] + mapX + 5;
						Canon4BulletY[a] = Data.Canon4Y[a] + mapY + 5;
						Canon4Time[a] = 0;
					}
				}
				//弾が存在するとき
				else
				{
					d = Data.Canon4BulletSpeed[a];
					while (d > 0)
					{
						if (d > 50)
						{
							e = 50;
							d -= 50;
						}
						else
						{
							e = d;
							d = 0;
						}
						if ((Canon4BulletX[a] - playerX)*(Canon4BulletX[a] - playerX) < BULLET_RADIUS*BULLET_RADIUS
							&& (Canon4BulletY[a] - playerY)*(Canon4BulletY[a] - playerY) < BULLET_RADIUS*BULLET_RADIUS
							&&Canon4BulletExist[a] == YES)
						{
							playerLife--;
							if (playerLife >= 1)
							{
								PlaySoundMem(soundPlayerDamaged, DX_PLAYTYPE_BACK);
							}
							Canon4BulletExist[a] = NO;
						}
						if (Data.Canon4Direction[a] == bCanon4DirectionUp)
						{
							Canon4BulletY[a] -= (double)e / 10;
						}
						else if (Data.Canon4Direction[a] == bCanon4DirectionLeft)
						{
							Canon4BulletX[a] -= (double)e / 10;
						}
						else if (Data.Canon4Direction[a] == bCanon4DirectionRight)
						{
							Canon4BulletX[a] += (double)e / 10;
						}
						else if (Data.Canon4Direction[a] == bCanon4DirectionDown)
						{
							Canon4BulletY[a] += (double)e / 10;
						}
						//現在の弾のマップチップの位置を代入
						b = int((Canon4BulletX[a] - mapX) / BLOCK_SIZE);
						c = int((Canon4BulletY[a] - mapY) / BLOCK_SIZE);
						if (mapData[Data.LayerNow][c][b] == 0)
						{
							Canon4BulletExist[a] = NO;
							d = 0;
						}
						else if (mapData[Data.LayerNow][c][b] == bIceBlock)//氷ブロック
						{
							Canon4BulletExist[a] = NO;
							mapData[Data.LayerNow][c][b] = -1;
							d = 0;
						}
					}
				}
			}
			//キャノンの弾を発射する瞬間
			else if (CanonBulletTime % (Data.Canon4BulletLiveTime[a] + Data.Canon4BulletRestTime[a]) == 0)
			{
				Canon4BulletExist[a] = YES;
				Canon4BulletX[a] = Canon4X[a] + mapX + 5;
				Canon4BulletY[a] = Canon4Y[a] + mapY + 5;
			}
			//キャノンの弾がステージ上に存在しているとき
			else if (int(CanonBulletTime % (Data.Canon4BulletLiveTime[a] + Data.Canon4BulletRestTime[a])) < Data.Canon4BulletLiveTime[a])
			{
				d = Data.Canon4BulletSpeed[a];
				while (d > 0)
				{
					if (d > 50)
					{
						e = 50;
						d -= 50;
					}
					else
					{
						e = d;
						d = 0;
					}
					if ((Canon4BulletX[a] - playerX)*(Canon4BulletX[a] - playerX) < BULLET_RADIUS*BULLET_RADIUS
						&& (Canon4BulletY[a] - playerY)*(Canon4BulletY[a] - playerY) < BULLET_RADIUS*BULLET_RADIUS
						&&Canon4BulletExist[a] == YES)
					{
						playerLife--;
						if (playerLife >= 1)
						{
							PlaySoundMem(soundPlayerDamaged, DX_PLAYTYPE_BACK);
						}
						Canon4BulletExist[a] = NO;
					}
					if (Data.Canon4Direction[a] == bCanon4DirectionUp)
					{
						Canon4BulletY[a] -= (double)e / 10;
					}
					else if (Data.Canon4Direction[a] == bCanon4DirectionLeft)
					{
						Canon4BulletX[a] -= (double)e / 10;
					}
					else if (Data.Canon4Direction[a] == bCanon4DirectionRight)
					{
						Canon4BulletX[a] += (double)e / 10;
					}
					else if (Data.Canon4Direction[a] == bCanon4DirectionDown)
					{
						Canon4BulletY[a] += (double)e / 10;
					}
				}
			}
			//キャノンの弾がステージ上に存在しなくなり弾を発射しない期間
			else if (CanonBulletTime % (Data.Canon4BulletLiveTime[a] + Data.Canon4BulletRestTime[a]) == Data.Canon4BulletLiveTime[a])
			{
				Canon4BulletExist[a] = NO;
			}
		}
		//自機狙いキャノン
		for (a = 0; a < Data.CanonAimMax; a++)
		{
			//弾の残る時間を0にしたとき、つまりプレイヤーや壁に当たるまで消えないキャノン
			if (Data.CanonAimBulletLiveTime[a] == 0)
			{
				//弾が存在しないとき
				if (CanonAimBulletExist[a] == NO)
				{
					CanonAimTime[a]++;
					if (CanonAimTime[a] >= Data.CanonAimBulletRestTime[a])
					{
						CanonAimBulletExist[a] = YES;
						CanonAimBulletX[a] = CanonAimX[a] + mapX + 5;
						CanonAimBulletY[a] = CanonAimY[a] + mapY + 5;
						radius = atan2((-CanonAimBulletY[a] + playerY), (-CanonAimBulletX[a] + playerX));//ラジアンの値を出す計算式
						CanonAimSin[a] = sin(radius);
						CanonAimCos[a] = cos(radius);
						CanonAimTime[a] = 0;
					}
				}
				//弾が存在するとき
				else
				{
					d = Data.CanonAimBulletSpeed[a];
					while (d > 0)
					{
						if (d > 50)
						{
							e = 50;
							d -= 50;
						}
						else
						{
							e = d;
							d = 0;
						}
						if ((CanonAimBulletX[a] - playerX)*(CanonAimBulletX[a] - playerX) < BULLET_RADIUS*BULLET_RADIUS
							&& (CanonAimBulletY[a] - playerY)*(CanonAimBulletY[a] - playerY) < BULLET_RADIUS*BULLET_RADIUS
							&&CanonAimBulletExist[a] == YES)
						{
							playerLife--;
							if (playerLife >= 1)
							{
								PlaySoundMem(soundPlayerDamaged, DX_PLAYTYPE_BACK);
							}
							CanonAimBulletExist[a] = NO;
						}
						CanonAimBulletY[a] += CanonAimSin[a] * (double)e / 10;
						CanonAimBulletX[a] += CanonAimCos[a] * (double)e / 10;
						//現在の弾のマップチップの位置を代入
						b = int((CanonAimBulletX[a] - mapX) / BLOCK_SIZE);
						c = int((CanonAimBulletY[a] - mapY) / BLOCK_SIZE);
						if (mapData[Data.LayerNow][c][b] == 0)
						{
							CanonAimBulletExist[a] = NO;
							d = 0;
						}
						else if (mapData[Data.LayerNow][c][b] == bIceBlock)//氷ブロック
						{
							CanonAimBulletExist[a] = NO;
							mapData[Data.LayerNow][c][b] = -1;
							d = 0;
						}
					}
				}

			}
			//キャノンの弾を発射する瞬間
			else if (CanonBulletTime % (Data.CanonAimBulletLiveTime[a] + Data.CanonAimBulletRestTime[a]) == 0)
			{
				CanonAimBulletExist[a] = YES;
				CanonAimBulletX[a] = CanonAimX[a] + mapX + 5;
				CanonAimBulletY[a] = CanonAimY[a] + mapY + 5;
				radius = atan2((-CanonAimBulletY[a] + playerY), (-CanonAimBulletX[a] + playerX));//ラジアンの値を出す計算式
				CanonAimSin[a] = sin(radius);
				CanonAimCos[a] = cos(radius);
			}
			//キャノンの弾がステージ上に存在しているとき
			else if (int(CanonBulletTime % (Data.CanonAimBulletLiveTime[a] + Data.CanonAimBulletRestTime[a])) < Data.CanonAimBulletLiveTime[a])
			{
				d = Data.CanonAimBulletSpeed[a];
				while (d > 0)
				{
					if (d > 50)
					{
						e = 50;
						d -= 50;
					}
					else
					{
						e = d;
						d = 0;
					}
					if ((CanonAimBulletX[a] - playerX)*(CanonAimBulletX[a] - playerX) < BULLET_RADIUS*BULLET_RADIUS
						&& (CanonAimBulletY[a] - playerY)*(CanonAimBulletY[a] - playerY) < BULLET_RADIUS*BULLET_RADIUS
						&&CanonAimBulletExist[a] == YES)
					{
						playerLife--;
						if (playerLife >= 1)
						{
							PlaySoundMem(soundPlayerDamaged, DX_PLAYTYPE_BACK);
						}
						CanonAimBulletExist[a] = NO;
					}
					CanonAimBulletY[a] += CanonAimSin[a] * (double)e / 10;
					CanonAimBulletX[a] += CanonAimCos[a] * (double)e / 10;
				}
			}
			//キャノンの弾がステージ上に存在しなくなり弾を発射しない期間
			else if (CanonBulletTime % (Data.CanonAimBulletLiveTime[a] + Data.CanonAimBulletRestTime[a]) == Data.CanonAimBulletLiveTime[a])
			{
				CanonAimBulletExist[a] = NO;
			}
		}
		CanonBulletTime++;//プレイモード時スタートから1フレーム毎に1増やす
		playerConveyorLR = playerConveyorL + playerConveyorR;
		//コンベアによる移動の処理
		for (b = mapYhit - 1; b <= mapYhit + 1; b++)//坂の当たり判定の移動の関係で下から順番に当たり判定を計算していく。
		{
			for (a = mapXhit - 1; a <= mapXhit + 1; a++)
			{
				if (playerConveyorU != 0)
				{
					playerMoveY(&playerConveyorU, Data, false);//第3引数はプレイヤー移動ならtrue,コンベア移動ならfalse
				}
				if (playerConveyorLR != 0)
				{
					playerMoveX(&playerConveyorLR, Data);
				}
			}
		}
		//ダメージ壁の処理
		if (playerIsDamaged == YES&&playerInvincibleTime == 0)
		{
			playerInvincibleTime = 1;
			playerLife--;
			if (playerLife >= 1)
			{
				PlaySoundMem(soundPlayerDamaged, DX_PLAYTYPE_BACK);
			}
		}
		if (playerLife <= 0 && playerIsDeath == NO)
		{
			playerIsDeath = YES;
			PlaySoundMem(soundPlayerDeath, DX_PLAYTYPE_BACK);
		}
		if (playerInvincibleTime != 0) { playerInvincibleTime++; }
		if (playerInvincibleTime > playerInvincibleTimeMax)
		{
			playerInvincibleTime = 0;
		}
		if (playerXtouch == NO)
		{
			playerX += playerConveyorL + playerConveyorR;
		}
		if (playerYtouch == NO)
		{
			playerY += playerConveyorU;
			//ジャンプしないで壁から降りたとき
			if (playerJumpCount == 0)
			{
				playerJumpCount = 1;
			}
		}
		//コンベアの処理
		playerConveyorR = 0;
		playerConveyorL = 0;
		playerConveyorU = 0;
	}
	//プレイヤーの画像の数値を計算するためのプログラム
	if (playerWalkCount % 100 > 50 || playerWalkCount % 100 == 0)
	{
		playerWalkG = STAND;
	}
	else if (playerDirection == RIGHT)
	{
		playerWalkG = RIGHT;
	}
	else if (playerDirection == LEFT)
	{
		playerWalkG = LEFT;
	}
	return gameClearFlag;
}
void playModeDraw(const SaveData_t& Data, const Images_t& images) {
	DrawBox(0, 0, 640, 480, GetColor(255, 255, 255), TRUE);    // 背景の白を描画
	for (a = int(-mapY / BLOCK_SIZE); a < int(-mapY / BLOCK_SIZE) + 48; a++)//マップ用の画像
	{
		for (b = int(-mapX / BLOCK_SIZE); b < int(-mapX / BLOCK_SIZE) + 64+1; b++)
		{
			if (mapData[Data.LayerNow][a][b] != -1)
			{
				DrawGraph(b * BLOCK_SIZE + int(mapX), a * BLOCK_SIZE + int(mapY), images.GmapTip[mapData[Data.LayerNow][a][b]], TRUE);
			}
		}
	}
	for (a = 0; a < Data.Canon4Max; a++)
	{
		if (Canon4BulletExist[a] == YES)
		{
			DrawCircle(int(Canon4BulletX[a]), int(Canon4BulletY[a]), BULLET_RADIUS - 2, GetColor(255, 0, 0), TRUE);
		}
	}
	for (a = 0; a < Data.CanonAimMax; a++)
	{
		if (CanonAimBulletExist[a] == YES)
		{
			DrawCircle(int(CanonAimBulletX[a]), int(CanonAimBulletY[a]), BULLET_RADIUS - 2, GetColor(255, 0, 0), TRUE);
		}
	}
	if (playerJumpCount == 0)
	{
		b = 1;
	}
	else
	{
		b = 0;
	}
	for (a = 0; a < playerJumpMax - playerJumpCount - b; a++)
	{
		DrawGraph(a * 10, 0, images.GplayerStatus[0], TRUE);
		if (a == 60)
		{
			break;
		}
	}
	for (a = 0; a < min(playerLife, 64); a++)
	{
		DrawGraph(a * 10, 10, images.GplayerStatus[1], TRUE);
	}
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);		//ブレンドモードをα(200/255)に設定
	DrawBox(0, 20, 50, 40, GetColor(255, 255, 255), TRUE);
	DrawBox(615, 0, 640, 10, GetColor(255, 255, 255), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);		//ブレンドモードをオフ
	DrawGraph(0, 20, images.GmapTip[bCoin], TRUE);
	DrawFormatString(10, 20, GetColor(255, 0, 0), "%d/%d", playerCoin, playerCoinMax);
	DrawFormatString(0, 30, GetColor(255, 0, 0), "Time:%d", playerTime);
	//プレイヤー用の画像
	if (playerInvincibleTime % 10 < 5 && playerLife > 0)
	{
		//-1.5,+0.5は画像にずれがあるので足している
		DrawGraph(int(playerX - PLAYER_WIDTH / 2 - 1.5), int(playerY - PLAYER_TALL / 2 + 0.5), images.GplayerTip[playerWalkG], TRUE);
	}
	//フォントを使う処理
	if (playerLife == 0 && playerCoin < playerCoinMax)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);		//ブレンドモードをα(200/255)に設定
		DrawBox(180, 193, 460, 285, GetColor(255, 255, 255), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);		//ブレンドモードをオフ
		DrawStringToHandle(219, 200, "GAME OVER", GetColor(255, 0, 0), images.Font);
		DrawStringToHandle(197, 260, "[ENTER]で再挑戦", GetColor(255, 255, 255), images.Font);
	}
	if (playerTime == 0 && playerTimeMax != 0)
	{
		if ((playerLife == 0 && playerCoin < playerCoinMax) == 0)
		{
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);		//ブレンドモードをα(200/255)に設定
			DrawBox(230, 225, 410, 255, GetColor(255, 255, 255), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);		//ブレンドモードをオフ
		}
		DrawStringToHandle(258, 230, "時間切れ", GetColor(255, 0, 0), images.Font);
	}
	if (playerCoin >= playerCoinMax || gameClearFlag == true)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);		//ブレンドモードをα(200/255)に設定
		DrawBox(200, 45, 440, 75, GetColor(255, 255, 255), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);		//ブレンドモードをオフ
		DrawStringToHandle(183, 50, "　STAGE CLEAR", GetColor(0, 0, 255), images.Font);
		gameClearFlag = true;
	}
	//デバッグ用の変数表示
	//DrawFormatString(400, 100, GetColor(255, 0, 0), "mapX=%lf,mapY=%lf\nplayerX=%lf,playerY=%lf\n,Canon4X[0]=%lf,Canon4Y[0]=%lf\nCanon4BulletX[0]=%lf\nCanon4BulletY[0]=%lf", mapX, mapY, playerX, playerY, Canon4X[0], Canon4Y[0], Canon4BulletX[0], Canon4BulletY[0]);
}
