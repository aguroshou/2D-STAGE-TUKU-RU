#ifndef INCLUDED_DEFINITIONS_H
#define INCLUDED_DEFINITIONS_H

#define PLAYER_TALL 10 
#define PLAYER_WIDTH 6
#define BLOCK_SIZE 10

//プレイヤーの画像の配列番号に合わせている。
#define STAND 0
#define LEFT 1
#define RIGHT 2
#define NO 0
#define YES 1

#define PLAYER_DECELERATION 0.80
#define MAP_X_MAX 300
#define MAP_Y_MAX 300

#define EDIT_LAYER_MAX 2
#define EDIT_TIME_MAX 99999//Data.editTimeMaxが取ることの出来る値の範囲

//コンベアの動く距離
#define EDIT_CONVEYOR_LR_S 1
#define EDIT_CONVEYOR_LR_M 2
#define EDIT_CONVEYOR_LR_L 2.9
#define EDIT_CONVEYOR_UP_S -2
#define EDIT_CONVEYOR_UP_M -3
#define EDIT_CONVEYOR_UP_L -4

//4方向キャノンの最大数
#define EDIT_4DIRECTION_CANON_MAX 1000
//自機狙いキャノンの最大数
#define EDIT_AIM_CANON_MAX 1000
#define EDIT_BULLET_SPEED_MAX 300//キャノンの弾の速さの最大値
#define EDIT_BULLET_LIVE_TIME_MAX 60000//キャノンの弾の残る時間の最大値
#define EDIT_BULLET_REST_TIME_MAX 60000//弾を次に撃つまでの待機時間の最大値
#define BULLET_RADIUS 5 //キャノンの弾の半径

typedef struct {
	int LayerNow, TimeMax, CoinExist, CoinMax, CoinNeed;
	int PlayerStartX, PlayerStartY;//ブロックのマス目の値を代入させる
	int CursorX[EDIT_LAYER_MAX], CursorY[EDIT_LAYER_MAX];
	int MapX[EDIT_LAYER_MAX], MapY[EDIT_LAYER_MAX];//レイヤー毎のマップの位置を入れる
	int MapXmax[EDIT_LAYER_MAX], MapYmax[EDIT_LAYER_MAX];//各レイヤーの端
	char MapData[EDIT_LAYER_MAX][MAP_Y_MAX][MAP_X_MAX];//大量に変数の宣言をしているけれど、大丈夫なのだろうか...
													   //4方向キャノン
	short int Canon4Max = 0;//EDIT_4DIRECTION_CANON_MAXとは違い、実際にステージ上にある数を代入させる。
	char Canon4Number[EDIT_4DIRECTION_CANON_MAX] = { 0 };//キャノンの配列の番号を代入する。
	char Canon4Direction[EDIT_4DIRECTION_CANON_MAX] = { 0 };//キャノンの4方向のいずれかを代入させる。5↑,6←,7→,8↓
	short int Canon4X[EDIT_4DIRECTION_CANON_MAX] = { 0 };
	short int Canon4Y[EDIT_4DIRECTION_CANON_MAX] = { 0 };
	short int Canon4BulletSpeed[EDIT_4DIRECTION_CANON_MAX] = { 0 };
	int Canon4BulletLiveTime[EDIT_4DIRECTION_CANON_MAX] = { 0 };
	int Canon4BulletRestTime[EDIT_4DIRECTION_CANON_MAX] = { 0 };
	//自機狙いキャノン
	short int CanonAimMax = 0;//EDIT_4DIRECTION_CANON_MAXとは違い、実際にステージ上にある数を代入させる。
	char CanonAimNumber[EDIT_4DIRECTION_CANON_MAX] = { 0 };//キャノンの配列の番号を代入する。
	char CanonAimDirection[EDIT_4DIRECTION_CANON_MAX] = { 0 };//キャノンの4方向のいずれかを代入させる。
	short int CanonAimX[EDIT_4DIRECTION_CANON_MAX] = { 0 };
	short int CanonAimY[EDIT_4DIRECTION_CANON_MAX] = { 0 };
	short int CanonAimBulletSpeed[EDIT_4DIRECTION_CANON_MAX] = { 0 };
	int CanonAimBulletLiveTime[EDIT_4DIRECTION_CANON_MAX] = { 0 };
	int CanonAimBulletRestTime[EDIT_4DIRECTION_CANON_MAX] = { 0 };
	//ステージ作成時に使用していく
	short int CanonBulletSpeedNow = 10;
	int CanonBulletLiveTimeNow = 60;
	int CanonBulletRestTimeNow = 0;
	//ゲームをクリア出来るか
	bool isThisStageCleared = false;
}SaveData_t;
typedef struct {
	int GplayerTip[25];//画像の順番は0人静止,1人←,2人→,3人左横向き,4人右横向き,黒赤黄青緑の順番
	int GmapTip[25];//画像の数は、0■壁,1右上坂,2左上坂,3すり抜け足場,4氷足場,5動く壁静止,6動く壁上,7動く壁左,8動く壁右,9動く壁下,10ダメージ壁■,11即死針,12ジャンプ台小,13ジャンプ台中,14ジャンプ台強,15左コンベア小,16左コンベア中,17左コンベア大,18右コンベア小,19右コンベア中,20右コンベア強,21扉,22ハート,23お金,24青い○,
	int GplayerStatus[25];//画像の数は、0ジャンプ可能数,1体力,2お金,3,4,黒赤黄青緑の順番
	int GeditTip[25];//画像の数は、0赤□,1赤□,2下三角,3上三角,4,5,6,7,8,9
	int Font;
}Images_t;
enum Blocks {
	bAir = -1,
	//0,3~25までは画像配列番号にも対応してる 1,2は未使用画像
	bBlock,
	bBridge = 3,
	bIceBlock,
	bCanonAim,
	bCanon4DirectionUp,
	bCanon4DirectionLeft,
	bCanon4DirectionRight,
	bCanon4DirectionDown,
	bDamageBlock,
	bDieBlock,
	bConveyorUpS,
	bConveyorUpM,
	bConveyorUpL,
	bConveyorLeftS,
	bConveyorLeftM,
	bConveyorLeftL,
	bConveyorRightS,
	bConveyorRightM,
	bConveyorRightL,
	bGoal,
	bHeart,
	bCoin,
	bJump,
	bStart
};
#endif
