#include "DxLib.h"
#include "stdlib.h"
#include <stdio.h>
#include <math.h>
#include "definitions.h"
#include "editMode.h"
#include "playMode.h"
#include "saveAndLoad.h"

//ＦＰＳを計算するためのクラス
class Fps {
	int mStartTime;         //測定開始時刻
	int mCount;             //カウンタ
	float mFps;             //fps
	static const int N = 60;//平均を取るサンプル数
	static const int FPS = 60;	//設定したFPS
public:
	Fps() {
		mStartTime = 0;
		mCount = 0;
		mFps = 0;
	}
	bool Update() {
		if (mCount == 0) { //1フレーム目なら時刻を記憶
			mStartTime = GetNowCount();
		}
		if (mCount == N) { //60フレーム目なら平均を計算する
			int t = GetNowCount();
			mFps = 1000.f / ((t - mStartTime) / (float)N);
			mCount = 0;
			mStartTime = t;
		}
		mCount++;
		return true;
	}
	void Draw() {
		DrawFormatString(617, 0, GetColor(255, 0, 0), "%.1f", mFps);
	}
	void Wait() {
		int tookTime = GetNowCount() - mStartTime;	//かかった時間
		int waitTime = mCount * 1000 / FPS - tookTime;	//待つべき時間
		if (waitTime > 0) {
			Sleep(waitTime);	//待機
		}
	}
};
void keyCheck(int *Key) {
	char tmpKey[256] = {};
	GetHitKeyStateAll(tmpKey); // 全てのキーの入力状態を得る
	for (int i = 0; i < 256; i++)
	{
		if (tmpKey[i] != 0)
		{ // i番のキーコードに対応するキーが押されていたら
			Key[i]++;     // 加算
		}
		else
		{              // 押されていなければ
			Key[i] = 0;   // 0にする
		}
	}

}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	//ログを出さなくする
	SetOutApplicationLogValidFlag(FALSE);
	SetBackgroundColor(0, 0, 0);
	bool isFullScreen;//フルスクリーンならYES,そうでないならばNOとする
	if (MessageBox(NULL, "フルスクリーンで起動しますか？", "起動オプション", MB_YESNO) == IDYES)
	{
		isFullScreen = YES;
		ChangeWindowMode(FALSE);
	}
	else
	{
		isFullScreen = NO;
		ChangeWindowMode(TRUE);	// ウィンドウモードで表示
		SetWindowSizeChangeEnableFlag(TRUE, TRUE);// ウインドウのサイズを手動で拡大する設定
		SetGraphMode(640, 480, 32);// 解像度を640x480 にする		
		SetWindowSize(640, 480);// 画面サイズを640x480 にする
	}
	if (DxLib_Init() == -1)	// ＤＸライブラリ初期化処理
		return -1;	// エラーが起きたら直ちに終了
	SetMainWindowText("2Dステージツクール");// タイトルを 2Dステージツクール に変更
	SetWindowIconID(1);//ウィンドウのアイコンを変更
	static int Key[256] = {}; // キーが押されているフレーム数を格納する
	//音読み込み
	static int soundPlayBgm = LoadSoundMem("音声/stageBgm.mp3"); //プレイモードの音楽
	static int soundEditBgm = LoadSoundMem("音声/editBgm.mp3"); //エディットモードの音楽
	ChangeVolumeSoundMem(255 * 40 / 100, soundPlayBgm);//音の大きさを調整している。
	ChangeVolumeSoundMem(255 * 40 / 100, soundEditBgm);//音の大きさを調整している。
	//画像読み込み
	static Images_t images;//画像データ用の構造体
	imagesLoad(&images);
	Fps fps;//FPS用のクラス
	SaveData_t Data;//セーブデータ用の構造体	
	editModeInitialize(&Data);
	playModeInitialize();
	while (ProcessMessage() == 0 && ClearDrawScreen() == 0 && Key[KEY_INPUT_ESCAPE] == 0)
	{
		//ここからステージ作成
		PlaySoundMem(soundEditBgm, DX_PLAYTYPE_LOOP);//ステージ作成用の音楽再生。
		while (ProcessMessage() == 0 && ClearDrawScreen() == 0 && Key[KEY_INPUT_ESCAPE] == 0 && Key[KEY_INPUT_TAB] != 1)
		{
			keyCheck(Key);//この引数Keyは&を付けていないがポインタ渡しである。
			if (Key[KEY_INPUT_Z] == 1)
			{
				save(Data, isFullScreen);
			}
			if (Key[KEY_INPUT_X] == 1)
			{
				load(&Data, isFullScreen);
			}
			editModeExecute(&Data, Key);
			editModeDraw(Data, images);
			fps.Update();	//fps更新
			fps.Draw();		//fps描画
			ScreenFlip();
			fps.Wait();		//待機
		}
		playModeDataInput(Data);
		Key[KEY_INPUT_TAB] = 2;
		StopSoundMem(soundEditBgm);//ステージ作成用の音楽を止める。
		PlaySoundMem(soundPlayBgm, DX_PLAYTYPE_LOOP);//ステージ用の音楽再生。
		while (ProcessMessage() == 0 && ClearDrawScreen() == 0 && Key[KEY_INPUT_ESCAPE] == 0 && Key[KEY_INPUT_TAB] != 1)
		{
			keyCheck(Key);//この引数Keyは&を付けていないがポインタ渡しである。
			if (Key[KEY_INPUT_RETURN] == 1)//エンターキー
			{
				playModeDataInput(Data);
			}
			if (play(Data, Key))
			{
				Data.isThisStageCleared = true;
			}
			playModeDraw(Data, images);
			fps.Update();	//fps更新
			fps.Draw();		//fps描画
			ScreenFlip();
			fps.Wait();		//待機
		}
		Key[KEY_INPUT_TAB] = 2;
		StopSoundMem(soundPlayBgm);//プレイモードの音楽を止める。
	}
	DxLib_End();		// ＤＸライブラリ使用の終了処理
	return 0;		// ソフトの終了
}