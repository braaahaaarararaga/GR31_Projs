//=======================================================================================
//
//	  [ 布 ] : バネを連結して布を作る
//
//    （操作）
//    カーソルキー：風の強さを変化させます
//    [ W ] : 布に与える風のＯＮ／ＯＦＦ切り替え
//    マウスで点をドラッグすると移動します（特定の点は移動後その場に固定）
//
//=======================================================================================

#define _CRT_SECURE_NO_WARNINGS
//----- インクルードファイル
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <math.h>
#include <mmsystem.h>

//----- ライブラリファイル
#pragma comment(lib, "winmm.lib")

//----- マクロ定義
#define F_ZERO(v)			v.x=v.y=v.z=0.0f
#define F_SET(v,px,py,pz)	v.x=px;v.y=py;v.z=pz

//----- 定数定義
#define FRAME_TIME				16				// フレームあたり時間

#define SCREEN_WIDTH			800				// スクリーン　幅
#define SCREEN_HEIGHT			600				// スクリーン　高さ
#define AXIS_X_OFFSET			SCREEN_WIDTH/2	// Ｘ座標オフセット
#define AXIS_Y_OFFSET			SCREEN_HEIGHT/2	// Ｙ座標オフセット

#define PARTICLE_HORIZONTAL		10				// 粒子　縦方向総数
#define PARTICLE_VERTICAL		10				// 粒子　横方向総数

#define SPRING_NUMS				(PARTICLE_HORIZONTAL*(PARTICLE_VERTICAL+1) + \
								 PARTICLE_VERTICAL*(PARTICLE_HORIZONTAL+1) + \
								 PARTICLE_VERTICAL*PARTICLE_HORIZONTAL*2)	// スプリング総数

#define SPRING_DEFAULT_LENGTH	40.0f			// バネ自然長
#define SPRING_COEFFICIENT		40.0f			// バネ係数
#define SPRING_MASS				2.0f			// バネ質量

#define ATT_COEFFICIENT			0.5f			// 減衰係数(Attenuation)
#define DELTA_TIME				0.1f			// 積分単位時間

#define WIND_FORCE_DEF_X		40.0f			// 風力Ｘ成分
#define WIND_FORCE_DEF_Y		20.0f			// 風力Ｙ成分
#define WIND_FORCE_DEF_Z		30.0f			// 風力Ｚ成分

//----- 構造体
struct VECTOR {
	float	x, y, z;			// 各成分
};

struct WINDFORCE {
	int		status;				// ステータス
	VECTOR	force;				// 風力ベクトル
};

struct PARTICLE {
	bool	onLock;				// 固定フラグ
	bool	onDrag;				// マウスドラッグ中判定フラグ
	VECTOR	position;			// 位置座標 (Position)
	VECTOR	velocity;			// 速度 (Velocity)
	VECTOR	acceleration;		// 加速度 (Acceleration)
	VECTOR	resultant;			// 合力 (Resultant)
	VECTOR	gravity;			// 重力 (Gravity)
};

struct P_REF {
	int		horz;				// 横方向インデックス
	int		vert;				// 縦方向インデックス
};

struct SPRING {
	P_REF	p1;					// 接続されている粒子１
	P_REF	p2;					// 接続されている粒子２
	float	length;				// 自然長
};

struct CLOTH {
	int			status;														// ステータス
	PARTICLE	Particle[PARTICLE_VERTICAL+1][PARTICLE_HORIZONTAL+1];		// 構成粒子
	SPRING		Spring[SPRING_NUMS];										// 粒子間のバネ
};

//----- グローバル変数
const TCHAR szClassName[] = _T("Cloth");
const TCHAR szAppName[]   = _T("[ 布 ] Cloth");

CLOTH		g_Cloth;					// 布オブジェクト
WINDFORCE	g_WindForce;				// 風力オブジェクト

HWND		g_hWndApp;					// ウィンドウハンドル
int			g_iClientWidth;				// クライアント領域サイズ
int			g_iClientHeight;			// クライアント領域サイズ
RECT		g_ClientRect;				// クライアント領域
HDC			g_hMemDC;					// 裏画面ＤＣ
HBITMAP		g_hBitmap;					// 裏画面ビットマップ
bool		g_bOnButton;				// マウスボタンクリックフラグ
bool		g_bOnInfo;					// ベクトル・数値情報表示スイッチ
bool		g_bOnWind;					// 風力スイッチ

//----- プロトタイプ宣言
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void MainModule();
void ExecGrid();
void ExecWindForce();
void ExecCloth();
void DispInfo();

//---------------------------------------------------------------------------------------
// メイン
//---------------------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR args, int cmdShow)
{
	MSG		msg;
	int		time, oldtime;

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
	g_bOnButton = false;		// マウスボタンフラグをクリア
	g_bOnInfo   = true;			// 情報表示スイッチをＯＮ
	g_bOnWind   = true;			// 風スイッチをＯＮ
	g_WindForce.status = 0;		// 風力ステータスの初期化
	g_Cloth.status     = 0;		// 布ステータスの初期化

	// メインループ
	timeBeginPeriod(1);
	msg.message = WM_NULL;
	while (WM_QUIT != msg.message) {					// WM_QUIT がくるまでループ
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		time = timeGetTime();
		if (time - oldtime > FRAME_TIME) {
			oldtime = time;
			MainModule();		// メインはココ
		}
		Sleep(1);
	}
	timeEndPeriod(1);
	return 0;
}

//-----------------------------------------------------------------------------
// メッセージ処理
//-----------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC				hdc;
	PAINTSTRUCT		ps;
	float			mx = 0.0f, my = 0.0f;
	int				i, j;

	switch (uMsg) {
	case WM_CREATE:
		hdc = GetDC(hWnd);
		g_hMemDC = CreateCompatibleDC(hdc);		// 裏画面の作成
		g_hBitmap = CreateCompatibleBitmap(hdc, SCREEN_WIDTH, SCREEN_HEIGHT);
		SelectObject(g_hMemDC, g_hBitmap);
		SetBkMode(g_hMemDC, TRANSPARENT);
		SetTextColor(g_hMemDC, RGB(0, 0, 0));
		ReleaseDC(hWnd, hdc);
		return 0;
	case WM_KEYDOWN:			// キーが押された
		switch (wParam) {
		case VK_ESCAPE:	DestroyWindow(hWnd);	return 0;	// 終了
		case 'I':	g_bOnInfo = !g_bOnInfo;		return 0;	// 情報表示スイッチの切り替え
		case 'W':	g_bOnWind = !g_bOnWind;		return 0;	// 風スイッチの切り替え
		}
		break;
	case WM_MOUSEMOVE:			// マウスが動いた
		if (g_bOnButton) {
			mx = (float)( (short)LOWORD(lParam) - AXIS_X_OFFSET);
			my = (float)(((short)HIWORD(lParam) - AXIS_Y_OFFSET) * -1);
			for (j = 0; j <= PARTICLE_VERTICAL; j++) {
				for (i = 0; i <= PARTICLE_HORIZONTAL; i++) {
					if (g_Cloth.Particle[j][i].onDrag) {
						g_Cloth.Particle[j][i].position.x = mx;		// ドラッグ中の点があれば移動
						g_Cloth.Particle[j][i].position.y = my;
					}
				}
			}
		}
		break;
	case WM_LBUTTONDOWN:		// マウス左ボタンが押された
		g_bOnButton = true;
		mx = (float)( (short)LOWORD(lParam) - AXIS_X_OFFSET);
		my = (float)(((short)HIWORD(lParam) - AXIS_Y_OFFSET) * -1);
		for (j = 0; j <= PARTICLE_VERTICAL; j++) {
			for (i = 0; i <= PARTICLE_HORIZONTAL; i++) {
				// 点から一定範囲内ならクリックしたとみなす
				if (g_Cloth.Particle[j][i].position.x - 6.0f <= mx && g_Cloth.Particle[j][i].position.x + 6.0f >= mx &&
					g_Cloth.Particle[j][i].position.y - 6.0f <= my && g_Cloth.Particle[j][i].position.y + 6.0f >= my) {
					g_Cloth.Particle[j][i].position.x = mx;
					g_Cloth.Particle[j][i].position.y = my;
					g_Cloth.Particle[j][i].onDrag = true;		// ドラッグ中
				}
			}
		}
		break;
	case WM_LBUTTONUP:			// マウス左ボタンが離された
		g_bOnButton = false;
		for (j = 0; j <= PARTICLE_VERTICAL; j++) {
			for (i = 0; i <= PARTICLE_HORIZONTAL; i++) {
				if (g_Cloth.Particle[j][i].onDrag) g_Cloth.Particle[j][i].onDrag = false;	// ドラッグフラグを解除
			}
		}
		break;
	case WM_PAINT:				// 描画
		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, g_hMemDC, 0, 0, SRCCOPY);	// 裏画面ＤＣをコピー
		EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		DeleteObject(g_hBitmap);
		DeleteDC(g_hMemDC);
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

	ExecGrid();									// グリッド表示処理実行
	ExecWindForce();							// 風力処理実行
	ExecCloth();								// 布処理実行
	if (g_bOnInfo) DispInfo();					// 情報表示

	InvalidateRect(g_hWndApp, NULL, TRUE);		// 描画要求
}


//---------------------------------------------------------------------------------------
// グリッド表示モジュール
//---------------------------------------------------------------------------------------
void ExecGrid()
{
	// グリッドのペン 
	HPEN PenGrid0 = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
	HPEN PenGrid1 = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));

	// 枠を描画
	HPEN hpenOld = (HPEN)SelectObject(g_hMemDC, PenGrid0);
	for (int x = 0; x < SCREEN_WIDTH; x += 20) {
		MoveToEx(g_hMemDC, x, 0, NULL);
		LineTo(g_hMemDC, x, SCREEN_HEIGHT);
	}
	for (int y = 0; y < SCREEN_HEIGHT; y += 20) {
		MoveToEx(g_hMemDC, 0, y, NULL);
		LineTo(g_hMemDC, SCREEN_WIDTH, y);
	}
	SelectObject(g_hMemDC, PenGrid1);
	MoveToEx(g_hMemDC, 0, AXIS_Y_OFFSET, NULL);
	LineTo(g_hMemDC, SCREEN_WIDTH, AXIS_Y_OFFSET);
	MoveToEx(g_hMemDC, AXIS_X_OFFSET, 0, NULL);
	LineTo(g_hMemDC, AXIS_X_OFFSET, SCREEN_HEIGHT);
	SelectObject(g_hMemDC, hpenOld);

	DeleteObject(PenGrid1);
	DeleteObject(PenGrid0);
}

//---------------------------------------------------------------------------------------
// 布モジュール
//---------------------------------------------------------------------------------------
void ExecWindForce()
{
	switch (g_WindForce.status) {
	case 0:
		g_WindForce.force.x = WIND_FORCE_DEF_X;		// 風力の初期値を設定
		g_WindForce.force.y = WIND_FORCE_DEF_Y;
		g_WindForce.force.z = WIND_FORCE_DEF_Z;
		g_WindForce.status = 1;
		// THRU
	case 1:
		if (GetAsyncKeyState(VK_UP   ) & 0x8000) g_WindForce.force.y += 1.0f;	// カーソルキーで風力を変える
		if (GetAsyncKeyState(VK_DOWN ) & 0x8000) g_WindForce.force.y -= 1.0f;
		if (GetAsyncKeyState(VK_LEFT ) & 0x8000) g_WindForce.force.x -= 1.0f;
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000) g_WindForce.force.x += 1.0f;
		if (GetAsyncKeyState(VK_PRIOR) & 0x8000) g_WindForce.force.z -= 1.0f;
		if (GetAsyncKeyState(VK_NEXT ) & 0x8000) g_WindForce.force.z += 1.0f;
		if (GetAsyncKeyState(VK_HOME ) & 0x0001) g_WindForce.status = 0;
		break;
	}
}

//---------------------------------------------------------------------------------------
// 布モジュール
//---------------------------------------------------------------------------------------
void ExecCloth()
{
	VECTOR	vec_spr;				// 粒子２点間のベクトル
	VECTOR	resistance;				// 抵抗力ベクトル
	VECTOR	windforce;				// 風力ベクトル
	float	f1, f2;					// バネが粒子に与える力（２種）
	int		h1, v1, h2, v2;			// バネ端にある粒子のインデックス（横、縦別２個分）
	float	f1x, f1y, f1z;			// 長さからバネに発生する力
	float	f2x, f2y, f2z;
	float	length;					// 粒子間の距離
	float	xx, yy, zz;				// 粒子間の距離（成分毎）
	int		count;					// バネカウント用
	int		i, j;					// ループカウンタ

	switch (g_Cloth.status) {
	case 0:			// 初期化
		// 粒子の初期化
		for (j = 0; j <= PARTICLE_VERTICAL; j++) {
			for (i = 0; i <= PARTICLE_HORIZONTAL; i++) {
				// 座標を設定
				g_Cloth.Particle[j][i].position.x =  SPRING_DEFAULT_LENGTH * (i - PARTICLE_HORIZONTAL/2);
				g_Cloth.Particle[j][i].position.y = -SPRING_DEFAULT_LENGTH * (j - PARTICLE_VERTICAL/2);
				g_Cloth.Particle[j][i].position.z = 0.0f;
				F_ZERO(g_Cloth.Particle[j][i].velocity);									// 速度初期化
				F_ZERO(g_Cloth.Particle[j][i].acceleration);								// 加速度初期化
				F_ZERO(g_Cloth.Particle[j][i].resultant);									// 合力初期化
				F_SET(g_Cloth.Particle[j][i].gravity, 0.0f, SPRING_MASS * -9.8f, 0.0f);		// 重力初期化
				// 特定の粒子はロック状態へ
				g_Cloth.Particle[j][i].onLock = (i == 0 && (j == 0 || j == PARTICLE_VERTICAL));	// ロック
				g_Cloth.Particle[j][i].onDrag = false;					// ドラッグされてない
			}
		}

		// バネの初期化
		count = 0;
		for (j = 0; j <= PARTICLE_VERTICAL; j++) {
			for (i = 0; i <= PARTICLE_HORIZONTAL; i++) {
				// バネ参照インデックスの設定
				if (i < PARTICLE_HORIZONTAL) {
					g_Cloth.Spring[count].p1.horz = i;		// １個目の参照粒子インデックスを設定
					g_Cloth.Spring[count].p1.vert = j;
					g_Cloth.Spring[count].p2.horz = i+1;	// ２個目の参照粒子インデックスを設定
					g_Cloth.Spring[count].p2.vert = j;
					// ２個の粒子間の距離を求め、バネの自然長とする
					xx = g_Cloth.Particle[j][i].position.x - g_Cloth.Particle[j][i+1].position.x;
					yy = g_Cloth.Particle[j][i].position.y - g_Cloth.Particle[j][i+1].position.y;
					zz = g_Cloth.Particle[j][i].position.z - g_Cloth.Particle[j][i+1].position.z;
					g_Cloth.Spring[count].length = sqrtf(xx*xx + yy*yy + zz*zz);
					count++;
				}
				if (j < PARTICLE_VERTICAL) {
					g_Cloth.Spring[count].p1.horz = i;
					g_Cloth.Spring[count].p1.vert = j;
					g_Cloth.Spring[count].p2.horz = i;
					g_Cloth.Spring[count].p2.vert = j+1;
					xx = g_Cloth.Particle[j][i].position.x - g_Cloth.Particle[j+1][i].position.x;
					yy = g_Cloth.Particle[j][i].position.y - g_Cloth.Particle[j+1][i].position.y;
					zz = g_Cloth.Particle[j][i].position.z - g_Cloth.Particle[j+1][i].position.z;
					g_Cloth.Spring[count].length = sqrtf(xx*xx + yy*yy + zz*zz);
					count++;
				}
				if (i < PARTICLE_HORIZONTAL && j < PARTICLE_VERTICAL) {
					g_Cloth.Spring[count].p1.horz = i;
					g_Cloth.Spring[count].p1.vert = j;
					g_Cloth.Spring[count].p2.horz = i+1;
					g_Cloth.Spring[count].p2.vert = j+1;
					xx = g_Cloth.Particle[j][i].position.x - g_Cloth.Particle[j+1][i+1].position.x;
					yy = g_Cloth.Particle[j][i].position.y - g_Cloth.Particle[j+1][i+1].position.y;
					zz = g_Cloth.Particle[j][i].position.z - g_Cloth.Particle[j+1][i+1].position.z;
					g_Cloth.Spring[count].length = sqrtf(xx*xx + yy*yy + zz*zz);
					count++;
				}
				if (i > 0 && j < PARTICLE_VERTICAL) {
					g_Cloth.Spring[count].p1.horz = i;
					g_Cloth.Spring[count].p1.vert = j;
					g_Cloth.Spring[count].p2.horz = i-1;
					g_Cloth.Spring[count].p2.vert = j+1;
					xx = g_Cloth.Particle[j][i].position.x - g_Cloth.Particle[j+1][i-1].position.x;
					yy = g_Cloth.Particle[j][i].position.y - g_Cloth.Particle[j+1][i-1].position.y;
					zz = g_Cloth.Particle[j][i].position.z - g_Cloth.Particle[j+1][i-1].position.z;
					g_Cloth.Spring[count].length = sqrtf(xx*xx + yy*yy + zz*zz);
					count++;
				}
			}
		}
		g_Cloth.status = 1;
		break;
	case 1:
		// 粒子の処理
		for (j = 0; j <= PARTICLE_VERTICAL; j++) {
			for (i = 0; i <= PARTICLE_HORIZONTAL; i++) {
				// 合力を０にする
				g_Cloth.Particle[j][i].resultant.x = 0;
				g_Cloth.Particle[j][i].resultant.y = 0;
				g_Cloth.Particle[j][i].resultant.z = 0;

				if (!g_Cloth.Particle[j][i].onLock && !g_Cloth.Particle[j][i].onDrag) {
					// 重力を求める
					F_SET(g_Cloth.Particle[j][i].gravity, 0.0f, SPRING_MASS * -9.8f, 0.0f);	// 重力

					// 抵抗力を求める
					resistance.x = -g_Cloth.Particle[j][i].velocity.x * ATT_COEFFICIENT;
					resistance.y = -g_Cloth.Particle[j][i].velocity.y * ATT_COEFFICIENT;
					resistance.z = -g_Cloth.Particle[j][i].velocity.z * ATT_COEFFICIENT;

					// 風力を求める
					if (g_bOnWind) {
						windforce.x = g_WindForce.force.x + (float)(rand() % 30 - 15);		// Ｘ成分
						windforce.y = g_WindForce.force.y + (float)(rand() % 30 - 15);		// Ｙ成分
						windforce.z = g_WindForce.force.z + (float)(rand() % 30 - 15);		// Ｚ成分
					} else {
						windforce.x = 0;
						windforce.y = 0;
						windforce.z = 0;
					}

					// 合力＝重力＋抵抗力＋風力
					g_Cloth.Particle[j][i].resultant.x = g_Cloth.Particle[j][i].gravity.x + resistance.x + windforce.x;
					g_Cloth.Particle[j][i].resultant.y = g_Cloth.Particle[j][i].gravity.y + resistance.y + windforce.y;
					g_Cloth.Particle[j][i].resultant.z = g_Cloth.Particle[j][i].gravity.z + resistance.z + windforce.z;
				}
			}
		}

		// バネの処理
		for (i = 0; i < SPRING_NUMS; i++) {
			// バネの参照粒子（２個）を取得
			h1 = g_Cloth.Spring[i].p1.horz;
			v1 = g_Cloth.Spring[i].p1.vert;
			h2 = g_Cloth.Spring[i].p2.horz;
			v2 = g_Cloth.Spring[i].p2.vert;
			// ２個の粒子間のベクトルを求める
			vec_spr.x = g_Cloth.Particle[v1][h1].position.x - g_Cloth.Particle[v2][h2].position.x;
			vec_spr.y = g_Cloth.Particle[v1][h1].position.y - g_Cloth.Particle[v2][h2].position.y;
			vec_spr.z = g_Cloth.Particle[v1][h1].position.z - g_Cloth.Particle[v2][h2].position.z;
			// 粒子間の距離を求める
			length = sqrtf(vec_spr.x * vec_spr.x + vec_spr.y * vec_spr.y + vec_spr.z * vec_spr.z);
			// 距離、自然長、バネ係数からかかる力を求める（２つ目は逆方向）
			f1 = -SPRING_COEFFICIENT * (length - g_Cloth.Spring[i].length);
			f2 = -f1;
			// ベクトルの成分に力をかける
			f1x = f1 * (vec_spr.x / length);
			f1y = f1 * (vec_spr.y / length);
			f1z = f1 * (vec_spr.z / length);
			f2x = f2 * (vec_spr.x / length);
			f2y = f2 * (vec_spr.y / length);
			f2z = f2 * (vec_spr.z / length);
			// 求めた力を合力に加える
			g_Cloth.Particle[v1][h1].resultant.x += f1x;
			g_Cloth.Particle[v1][h1].resultant.y += f1y;
			g_Cloth.Particle[v1][h1].resultant.z += f1z;
			g_Cloth.Particle[v2][h2].resultant.x += f2x;
			g_Cloth.Particle[v2][h2].resultant.y += f2y;
			g_Cloth.Particle[v2][h2].resultant.z += f2z;
		}

		for (j = 0; j <= PARTICLE_VERTICAL; j++) {
			for (i = 0; i <= PARTICLE_HORIZONTAL; i++) {
				// 固定されていればスキップ
				if (g_Cloth.Particle[j][i].onLock || g_Cloth.Particle[j][i].onDrag) continue;

				// 合力と質量から加速度を求める
				g_Cloth.Particle[j][i].acceleration.x = g_Cloth.Particle[j][i].resultant.x / SPRING_MASS;
				g_Cloth.Particle[j][i].acceleration.y = g_Cloth.Particle[j][i].resultant.y / SPRING_MASS;
				g_Cloth.Particle[j][i].acceleration.z = g_Cloth.Particle[j][i].resultant.z / SPRING_MASS;

				// 速度に加速度を加える
				g_Cloth.Particle[j][i].velocity.x += g_Cloth.Particle[j][i].acceleration.x * DELTA_TIME;
				g_Cloth.Particle[j][i].velocity.y += g_Cloth.Particle[j][i].acceleration.y * DELTA_TIME;
				g_Cloth.Particle[j][i].velocity.z += g_Cloth.Particle[j][i].acceleration.z * DELTA_TIME;

				// 速度から座標を移動
				g_Cloth.Particle[j][i].position.x += g_Cloth.Particle[j][i].velocity.x * DELTA_TIME;
				g_Cloth.Particle[j][i].position.y += g_Cloth.Particle[j][i].velocity.y * DELTA_TIME;
				g_Cloth.Particle[j][i].position.z += g_Cloth.Particle[j][i].velocity.z * DELTA_TIME;
			}
		}
		break;
	}

	// 布を描画
	HPEN PenCloth = CreatePen(PS_SOLID, 2, RGB(0, 128, 0));
	HPEN hpenOld = (HPEN)SelectObject(g_hMemDC, PenCloth);
	for (i = 0; i < SPRING_NUMS; i++) {
		// バネの参照粒子（２個）を取得
		h1 = g_Cloth.Spring[i].p1.horz;
		v1 = g_Cloth.Spring[i].p1.vert;
		h2 = g_Cloth.Spring[i].p2.horz;
		v2 = g_Cloth.Spring[i].p2.vert;
		// バネを描画
		MoveToEx(g_hMemDC, AXIS_X_OFFSET + (int)g_Cloth.Particle[v1][h1].position.x, AXIS_Y_OFFSET - (int)g_Cloth.Particle[v1][h1].position.y, NULL);
		LineTo(g_hMemDC, AXIS_X_OFFSET + (int)g_Cloth.Particle[v2][h2].position.x, AXIS_Y_OFFSET - (int)g_Cloth.Particle[v2][h2].position.y);
	}
	for (j = 0; j <= PARTICLE_VERTICAL; j++) {
		for (i = 0; i <= PARTICLE_HORIZONTAL; i++) {
			// 点を描画
			Ellipse(g_hMemDC,
				AXIS_X_OFFSET + (int)g_Cloth.Particle[j][i].position.x - 4, AXIS_Y_OFFSET - (int)g_Cloth.Particle[j][i].position.y - 4,
				AXIS_X_OFFSET + (int)g_Cloth.Particle[j][i].position.x + 4, AXIS_Y_OFFSET - (int)g_Cloth.Particle[j][i].position.y + 4);
		}
	}
	SelectObject(g_hMemDC, hpenOld);
	DeleteObject(PenCloth);
}

//---------------------------------------------------------------------------------------
// 情報表示モジュール
//---------------------------------------------------------------------------------------
void DispInfo()
{
	TCHAR	str[256];
	int		nLen;

	HFONT hfontOld = (HFONT)SelectObject(g_hMemDC, GetStockObject(SYSTEM_FIXED_FONT));
	// 風力表示
	nLen = _stprintf(str, _T("Wind (%8.3f,%8.3f,%8.3f)"), g_WindForce.force.x, g_WindForce.force.y, g_WindForce.force.z);
	TextOut(g_hMemDC, 0, 0, str, nLen);
	if (g_bOnWind)	nLen = _stprintf(str, _T("< Wind On  >"));
	else			nLen = _stprintf(str, _T("< Wind Off >"));
	TextOut(g_hMemDC, 0, 20, str, nLen);
	SelectObject(g_hMemDC, hfontOld);
}

//=======================================================================================
//	end
//=======================================================================================
