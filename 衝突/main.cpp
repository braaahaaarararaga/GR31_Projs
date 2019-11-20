//=======================================================================================
//
//	[ C_Ball ] : Collision of Balls  物体の衝突
//
//=======================================================================================

//----- インクルードファイル
#include <windows.h>
#include <math.h>
#include <mmsystem.h>
#include <tchar.h>			// 汎用テキスト マッピング

//----- ライブラリファイル
#pragma comment(lib, "winmm.lib")

//----- 構造体
struct BALL {
	float	x, y;			// 位置座標
	float	vx, vy;			// 速度
	float	m;				// 質量
	float	r;				// 半径
};

//----- 定数定義
#define	BALL_QTY		2		// 初期ボール個数
#define FRAME_TIME		10		// フレームあたり時間

#define SCREEN_WIDTH	800		// スクリーン　幅
#define SCREEN_HEIGHT	600		// スクリーン　高さ

#define BOUND_CONST		1.0f	// はねかえり係数

BALL initBallData[BALL_QTY] = {
	//     x       y      vx      vy       m      r
	{ 100.0f, 100.0f, 150.0f,  80.0f, 200.0f, 50.0f },		// ボール０
	{ 100.0f, 500.0f, 150.0f, -80.0f, 200.0f, 50.0f },		// ボール１
};

//----- グローバル変数
const TCHAR szClassName[] = _T("C_Ball");
const TCHAR szAppName[]   = _T("Collision of Balls");

BALL		g_Ball[BALL_QTY];	// ボール本体

HWND		g_hWndApp;				// ウィンドウハンドル
int			g_iClientWidth;			// クライアント領域サイズ
int			g_iClientHeight;		// クライアント領域サイズ
RECT		g_ClientRect;			// クライアント領域
HDC			g_hMemDC;				// 裏画面ＤＣ
HBITMAP		g_hBitmap;				// 裏画面ビットマップ

//----- プロトタイプ宣言
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void MainModule();

//-----------------------------------------------------------------------------
// メイン
//-----------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR args, int cmdShow)
{
	MSG		msg;
	int		time, oldtime;
	int		i;

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
	oldtime = timeGetTime();

	// ボールの初期化
	for (i = 0; i < BALL_QTY; i++) {
		g_Ball[i] = initBallData[i];
	}

	// メインループ
	timeBeginPeriod(1);
	msg.message = WM_NULL;
	while (WM_QUIT != msg.message) {				// WM_QUIT がくるまでループ
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		time = timeGetTime();
		if (time - oldtime > FRAME_TIME) {
			oldtime = time;
			MainModule();		// メインはココ
		}
	}
	timeEndPeriod(1);
	return (int)msg.wParam;
}

//-----------------------------------------------------------------------------
// メッセージ処理
//-----------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HBITMAP	oldbmp;
	static HFONT	oldfont;
	static HPEN		oldpen;
	static HBRUSH	oldbrush;
	HDC				hdc;
	PAINTSTRUCT		ps;

	switch (uMsg) {
	case WM_CREATE:
		hdc = GetDC(hWnd);
		g_hMemDC = CreateCompatibleDC(hdc);		// 裏画面の作成
		g_hBitmap = CreateCompatibleBitmap(hdc, SCREEN_WIDTH, SCREEN_HEIGHT);
		oldbmp   = (HBITMAP)SelectObject(g_hMemDC, g_hBitmap);
		SetBkMode(g_hMemDC, TRANSPARENT);
		SetTextColor(g_hMemDC, RGB(0, 0, 0));
		oldfont  = (HFONT)SelectObject(g_hMemDC, GetStockObject(SYSTEM_FIXED_FONT));
		oldpen   = (HPEN)SelectObject(g_hMemDC, CreatePen(PS_SOLID, 1, RGB(0, 0, 255)));
		oldbrush = (HBRUSH)SelectObject(g_hMemDC, CreateSolidBrush(RGB(0, 0, 255)));
		ReleaseDC(hWnd, hdc);
		return 0;
	case WM_KEYDOWN:			// キーが押された
		switch (wParam) {
		case VK_ESCAPE:
			DestroyWindow(hWnd);				// 終了
			return 0;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, g_hMemDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		if (g_hMemDC) {
			DeleteObject(SelectObject(g_hMemDC, oldbrush));
			DeleteObject(SelectObject(g_hMemDC, oldpen));
			SelectObject(g_hMemDC, oldfont);
			SelectObject(g_hMemDC, oldbmp);
			DeleteDC(g_hMemDC);
		}
		if (g_hBitmap) DeleteObject(g_hBitmap);
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// メインモジュール
//-----------------------------------------------------------------------------
void MainModule()
{
	static float	before_length = 0.0f;		// 前回のボール間の実際の距離
	float	length;								// ボール間の実際の距離
	float	dt;									// 微少時間Δｔ
	float	dx, dy;								// 座標差分
	float	e;									// はねかえり係数
	float	vx0_new, vy0_new, vx1_new, vy1_new;	// 衝突後の速度
	int		i;
	int		out;

	// メモリにあるＤＣをクリア
	PatBlt(g_hMemDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITENESS);

	// ボールの移動
	dt = 0.01f;
	for (i = out = 0; i < BALL_QTY; i++) {
		// 移動
		g_Ball[i].x += g_Ball[i].vx * dt;
		g_Ball[i].y += g_Ball[i].vy * dt;
		if (g_Ball[i].x > SCREEN_WIDTH + g_Ball[i].r ||
			g_Ball[i].y > SCREEN_HEIGHT + g_Ball[i].r) {
			out++;
		}
	}
	if (out >= BALL_QTY) {
		for (i = 0; i < BALL_QTY; i++) {
			g_Ball[i] = initBallData[i];
		}
	}
	// ボール同士のヒットチェック
	dx = g_Ball[1].x - g_Ball[0].x;				// ボール間のＸ方向差分
	dy = g_Ball[1].y - g_Ball[0].y;				// ボール間のＹ方向差分
	length = sqrtf(dx * dx + dy * dy);			// ボール間の距離を計算
	if (before_length > length && length < g_Ball[0].r + g_Ball[1].r) {
		// 距離が短くなっており、かつ半径の合計よりも距離の方が短いので当たったと判定
		e = BOUND_CONST;		// はねかえり係数ｅ
		// はねかえりの法則 e=-(v1'-v2')/(v1-v2) と運動量保存則 m1v1'+m2v2' = m1v1+m2v2 を連立させる
		vx0_new = ((g_Ball[0].m - e * g_Ball[1].m) * g_Ball[0].vx + (1 + e) * g_Ball[1].m * g_Ball[1].vx) / (g_Ball[0].m + g_Ball[1].m);
		vy0_new = ((g_Ball[0].m - e * g_Ball[1].m) * g_Ball[0].vy + (1 + e) * g_Ball[1].m * g_Ball[1].vy) / (g_Ball[0].m + g_Ball[1].m);
		vx1_new = ((g_Ball[1].m - e * g_Ball[0].m) * g_Ball[1].vx + (1 + e) * g_Ball[0].m * g_Ball[0].vx) / (g_Ball[0].m + g_Ball[1].m);
		vy1_new = ((g_Ball[1].m - e * g_Ball[0].m) * g_Ball[1].vy + (1 + e) * g_Ball[0].m * g_Ball[0].vy) / (g_Ball[0].m + g_Ball[1].m);
		g_Ball[0].vx = vx0_new;
		g_Ball[0].vy = vy0_new;
		g_Ball[1].vx = vx1_new;
		g_Ball[1].vy = vy1_new;
	}
	before_length = length;						// 距離を保存
	// ボールを描画
	for (i = 0; i < BALL_QTY; i++) {
		Ellipse(g_hMemDC,
			(int)(g_Ball[i].x - g_Ball[i].r), (int)(g_Ball[i].y - g_Ball[i].r),
			(int)(g_Ball[i].x + g_Ball[i].r), (int)(g_Ball[i].y + g_Ball[i].r));
	}
	InvalidateRect(g_hWndApp, NULL, TRUE);		// 描画要求
}

//=======================================================================================
//	end
//=======================================================================================
