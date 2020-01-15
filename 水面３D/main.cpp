//=======================================================================================
//
//	[ 水面（３Ｄ） ] : 水面の挙動を再現する（３Ｄバージョン）
//
//=======================================================================================

//----- インクルードファイル
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>

//----- 構造体
struct VECTOR {
	float	x, y, z;		// ３Ｄの各成分
};

struct WAVE {
	int		status;				// ステータス
	VECTOR	position;			// 位置座標 (Position)
	float	amplitude;			// 振幅 (Amplitude)
	float	time;				// 時間
};

//----- 定数定義
#define FRAME_TIME				20						// フレームあたり時間

#define SCREEN_WIDTH			800						// スクリーン　幅
#define SCREEN_HEIGHT			600						// スクリーン　高さ
#define AXIS_X_OFFSET			SCREEN_WIDTH/2			// Ｘ座標オフセット
#define AXIS_Y_OFFSET			SCREEN_HEIGHT/2			// Ｙ座標オフセット

#define WAVE_PITCH				10						// ウェーブポイントの間隔
#define WAVE_QTY				(40+1)					// ウェーブポイントの総数
#define WAVE_AMPLITUDE			20.0f					// ウェーブポイントの振幅
#define WAVE_LENGTH				20.0f					// ウェーブポイントの波長
#define WAVE_CYCLE				1.0f					// ウェーブポイントの周期

//----- グローバル変数
const TCHAR szClassName[] = _T("Water Surface");
const TCHAR szAppName[]   = _T("[ 水面（３Ｄ） ] Water Surface");

WAVE		g_Wave[WAVE_QTY][WAVE_QTY];		// ウェーブポイント

HWND		g_hWndApp;					// ウィンドウハンドル
int			g_iClientWidth;				// クライアント領域サイズ
int			g_iClientHeight;			// クライアント領域サイズ
RECT		g_ClientRect;				// クライアント領域
HDC			g_hMemDC;					// 裏画面ＤＣ
HBITMAP		g_hBitmap;					// 裏画面ビットマップ
HPEN		g_PenGrid0, g_PenGrid1;		// グリッドのペン 
HPEN		g_PenWave;					// ウェーブポイントのペン
HBRUSH		g_BrushWave;				// ウェーブポイントのブラシ

//----- プロトタイプ宣言
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void MainModule();
void ExecWavePoint();
void DispInfo();
void DrawWaveLine();

//---------------------------------------------------------------------------------------
// メイン
//---------------------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR args, int cmdShow)
{
	MSG		msg;
	int		x, z;

	// ウィンドウクラスを登録
	WNDCLASS wndClass = {
		0, WndProc, 0, 0, hInst,
		LoadIcon(hInst, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
		0, NULL, szClassName
	};
	if (RegisterClass(&wndClass) == 0) return false;

	// ウインドウを作成
	g_hWndApp = CreateWindow(
		szClassName, szAppName,
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		SCREEN_WIDTH  + GetSystemMetrics(SM_CXFIXEDFRAME) * 2,
		SCREEN_HEIGHT + GetSystemMetrics(SM_CYFIXEDFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION),
		NULL, NULL, hInst, NULL);
	if (!g_hWndApp) return false;
	ShowWindow(g_hWndApp, cmdShow);
	UpdateWindow(g_hWndApp);

	// 変数初期化
	for (z = 0; z < WAVE_QTY; z++) {
		for (x = 0; x < WAVE_QTY; x++) {
			g_Wave[z][x].status = 0;		// ウェーブポイントの初期化
		}
	}

	// メインループ
	msg.message = WM_NULL;
	while (WM_QUIT != msg.message) {					// WM_QUIT がくるまでループ
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
// メッセージ処理
//-----------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HGDIOBJ	oldfont;
	static HPEN		oldpen;
	static HBRUSH	oldbrush;
	HDC				hdc;
	PAINTSTRUCT		ps;

	switch (uMsg) {
	case WM_CREATE:
		hdc = GetDC(hWnd);
		g_hMemDC = CreateCompatibleDC(hdc);		// 裏画面の作成
		g_hBitmap = CreateCompatibleBitmap(hdc, SCREEN_WIDTH, SCREEN_HEIGHT);
		SelectObject(g_hMemDC, g_hBitmap);
		SetBkMode(g_hMemDC, TRANSPARENT);
		SetTextColor(g_hMemDC, RGB(0, 0, 0));
		oldfont  = SelectObject(g_hMemDC, GetStockObject(SYSTEM_FIXED_FONT));
		g_PenGrid0     = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
		g_PenGrid1     = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
		g_PenWave      = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
		g_BrushWave    = CreateSolidBrush(RGB(0, 0, 255));
		ReleaseDC(hWnd, hdc);
		SetTimer(hWnd, 0, FRAME_TIME, NULL);
		return 0;
	case WM_KEYDOWN:			// キーが押された
		switch (wParam) {
		case VK_ESCAPE:	DestroyWindow(hWnd);	return 0;			// 終了
		}
		break;
	case WM_PAINT:				// 描画
		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, g_hMemDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		return 0;
	case WM_TIMER:
		MainModule();		// メインはココ
		return 0;
	case WM_DESTROY:
		KillTimer(hWnd, 0);
		if (g_hBitmap) DeleteObject(g_hBitmap);
		if (g_hMemDC) {
			DeleteObject(SelectObject(g_hMemDC, g_BrushWave));
			DeleteObject(SelectObject(g_hMemDC, g_PenWave));
			DeleteObject(SelectObject(g_hMemDC, g_PenGrid1));
			DeleteObject(SelectObject(g_hMemDC, g_PenGrid0));
			DeleteObject(SelectObject(g_hMemDC, oldfont));
			DeleteDC(g_hMemDC);
		}
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//---------------------------------------------------------------------------------------
// メインモジュール
//---------------------------------------------------------------------------------------
void MainModule()
{
	// メモリにあるＤＣをクリア
	PatBlt(g_hMemDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITENESS);

	ExecWavePoint();			// ウェーブポイント表示処理実行
	DrawWaveLine();				// ウェーブポイント間をラインで接続

	// 数値表示
	TCHAR	str[256];
	int		nLen;
	nLen = _stprintf(str, _T("時間   (%8.3f)"), g_Wave[0][0].time);
	TextOut(g_hMemDC, 0, 0, str, nLen);

	InvalidateRect(g_hWndApp, NULL, TRUE);		// 描画要求
}

//---------------------------------------------------------------------------------------
// ウェーブポイントモジュール
//---------------------------------------------------------------------------------------
void ExecWavePoint()
{
	int		x, z;
	float	dx, dz, length;
	float	px, py;
	HPEN	oldPen;
	HBRUSH	oldBrush;

	oldPen   = (HPEN)SelectObject(g_hMemDC, g_PenWave);
	oldBrush = (HBRUSH)SelectObject(g_hMemDC, g_BrushWave);
	for (z = 0; z < WAVE_QTY; z++) {
		for (x = 0; x < WAVE_QTY; x++) {
			switch (g_Wave[z][x].status) {
			case 0:		// 初期化
				g_Wave[z][x].position.x = (float)((x - WAVE_QTY / 2) * WAVE_PITCH);	// Ｘ座標
				g_Wave[z][x].position.y = 0.0f;										// Ｙ座標
				g_Wave[z][x].position.z = (float)((z - WAVE_QTY / 2) * WAVE_PITCH);	// Ｚ座標

				g_Wave[z][x].amplitude = WAVE_AMPLITUDE;	// 振幅
				g_Wave[z][x].time      = 0.0f;
				g_Wave[z][x].status    = 1;
				// THRU
			case 1:		// メイン処理

				//===============================================================================
				//
				//    ここに計算式を入れてウェーブポイントを動かす
				//
				//===============================================================================

				// 振動原点からの距離を求める
				dx = g_Wave[z][x].position.x - g_Wave[WAVE_QTY / 2][WAVE_QTY / 2].position.x;
				dz = g_Wave[z][x].position.z - g_Wave[WAVE_QTY / 2][WAVE_QTY / 2].position.z;
				length = sqrtf(dx * dx + dz * dz);

				// 距離を元に振動の式から変位を求める
				g_Wave[z][x].position.y = g_Wave[z][x].??? * ???(2.0f * (float)M_PI * ((??? / WAVE_LENGTH) - (g_Wave[z][x].??? / WAVE_CYCLE)));

				// 時間を進める
				g_Wave[z][x].time += 0.01f;

				break;
			}
			// ウェーブポイントを描画
			px = AXIS_X_OFFSET + g_Wave[z][x].position.x + g_Wave[z][x].position.z;
			py = AXIS_Y_OFFSET - g_Wave[z][x].position.y - g_Wave[z][x].position.z;
			Ellipse(g_hMemDC, (int)(px - 2.0f), (int)(py - 2.0f), (int)(px + 2.0f), (int)(py + 2.0f));
		}
	}
	SelectObject(g_hMemDC, oldPen);
	SelectObject(g_hMemDC, oldBrush);
}

//---------------------------------------------------------------------------------------
// ポイント間をラインで描画
//---------------------------------------------------------------------------------------
void DrawWaveLine()
{
	int		x, z;
	float	px, py, px2, py2;
	HPEN	oldPen;
	
	oldPen   = (HPEN)SelectObject(g_hMemDC, g_PenWave  );
	for (z = 0; z < WAVE_QTY; z++) {
		for (x = 0; x < WAVE_QTY; x++) {
			if (x < WAVE_QTY - 1) {
				px  = AXIS_X_OFFSET + g_Wave[z][x].position.x + g_Wave[z][x].position.z;
				py  = AXIS_Y_OFFSET - g_Wave[z][x].position.y - g_Wave[z][x].position.z;
				px2 = AXIS_X_OFFSET + g_Wave[z][x + 1].position.x + g_Wave[z][x + 1].position.z;
				py2 = AXIS_Y_OFFSET - g_Wave[z][x + 1].position.y - g_Wave[z][x + 1].position.z;
				MoveToEx(g_hMemDC, (int)px, (int)py, NULL);
				LineTo(g_hMemDC, (int)px2, (int)py2);
			}
			if (z < WAVE_QTY - 1) {
				px  = AXIS_X_OFFSET + g_Wave[z][x].position.x + g_Wave[z][x].position.z;
				py  = AXIS_Y_OFFSET - g_Wave[z][x].position.y - g_Wave[z][x].position.z;
				px2 = AXIS_X_OFFSET + g_Wave[z + 1][x].position.x + g_Wave[z + 1][x].position.z;
				py2 = AXIS_Y_OFFSET - g_Wave[z + 1][x].position.y - g_Wave[z + 1][x].position.z;
				MoveToEx(g_hMemDC, (int)px, (int)py, NULL);
				LineTo(g_hMemDC, (int)px2, (int)py2);
			}
		}
	}
	SelectObject(g_hMemDC, oldPen);
}

//=======================================================================================
//	end
//=======================================================================================
