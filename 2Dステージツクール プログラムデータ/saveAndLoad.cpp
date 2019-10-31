#include "DxLib.h"
#include "stdlib.h"
#include <stdio.h>
#include <conio.h>//セーブデータで使用する。
#include "definitions.h"
// 保存するファイル名を保存する領域の確保と初期化
static char FileName[MAX_PATH * 2] = {};
//セーブデータを保存・開くファイルを選択するときに使用する関数
int ShowWindow(HWND hwnd, LPCSTR Filter, char *FileName, DWORD Flags, int mode)
{
	OPENFILENAME OFN;

	// 構造体を設定
	ZeroMemory(&OFN, sizeof(OPENFILENAME));
	OFN.lStructSize = sizeof(OPENFILENAME);
	OFN.hwndOwner = hwnd;
	OFN.lpstrFilter = Filter;
	OFN.lpstrFile = FileName;
	OFN.nMaxFile = MAX_PATH * 2;
	OFN.Flags = Flags | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
	if (mode == 1)
	{
		OFN.lpstrTitle = "名前を付けて保存";
		return (GetSaveFileName(&OFN));
	}
	else
	{
		OFN.lpstrTitle = "ファイルを開く";
		return (GetOpenFileName(&OFN));
	}
}
void save(const SaveData_t& Data, bool isFullScreen) {
	if (isFullScreen)SetMouseDispFlag(TRUE);
	FILE *fp;
	errno_t error;
	//初期化(これをしないとごみが入る)
	ZeroMemory(FileName, MAX_PATH * 2);
	// ダイアログを表示　選択されたファイル名はFileNameに保存される
	ShowWindow(GetMainWindowHandle(), "Dat Files(*.dat)\0 * .dat\0All Files(*.*)\0 * .*\0\0", FileName, 0, 1);
	if (error = fopen_s(&fp, FileName, "wb") == 0) {	//エラーの確認
		fwrite(&Data, sizeof(Data), 1, fp);
		fclose(fp);
	}
	if (isFullScreen)SetMouseDispFlag(FALSE);
}
void load(SaveData_t* Data, bool isFullScreen) {
	if (isFullScreen)SetMouseDispFlag(TRUE);
	FILE *fp;
	errno_t error;
	//初期化(これをしないとごみが入る)
	ZeroMemory(FileName, MAX_PATH * 2);
	// ダイアログを表示　選択されたファイル名はFileNameに保存される
	ShowWindow(GetMainWindowHandle(), "Dat Files(*.dat)\0 * .dat\0All Files(*.*)\0 * .*\0\0", FileName, 0, 0);
	if (error = fopen_s(&fp, FileName, "rb") == 0) {	//エラーの確認
		fread(Data, sizeof(*Data), 1, fp);
		fclose(fp);
	}
	if (isFullScreen)SetMouseDispFlag(FALSE);
}
void imagesLoad(Images_t* images) {
	LoadDivGraph("画像/playerTip.png", 25, 5, 5, 10, 10, images->GplayerTip);
	LoadDivGraph("画像/MapTipStage.png", 25, 5, 5, 10, 10, images->GmapTip);
	LoadDivGraph("画像/playerStatus.png", 25, 5, 5, 10, 10, images->GplayerStatus);
	LoadDivGraph("画像/editTip.png", 25, 5, 5, 10, 10, images->GeditTip);
	//フォント読み込みゾーン
	images->Font = CreateFontToHandle("游明朝 Light", 20, 3, DX_FONTTYPE_ANTIALIASING_EDGE);//"メイリオ"  の20pt,太さ3のフォントを作成
	SetFontSize(10);
	SetFontThickness(3);
}
