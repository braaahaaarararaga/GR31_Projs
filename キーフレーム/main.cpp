//=======================================================================================
//
//	キーフレームアニメーション
//
//=======================================================================================

#define _CRT_SECURE_NO_WARNINGS
// インクルードファイル
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <math.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define RAD				3.14159265f/180.0f

#define JINTAI_PARTS 10			// 人体パーツ個数
#define KEYFRAME_MAX 26			// キーフレーム最大数

//----- 構造体定義 -----
typedef struct {
	float	m[3][3];
} MATRIX;

typedef struct {
	int		parent;				// 親パーツ番号
	int		type;				// 描画種別
	float	length;				// 長さ
	float	offx, offy;			// オフセット値
	float	angle;				// 角度
	MATRIX	matrix;				// 回転マトリックス
} PARTS;

typedef struct {
	int		status;					// ステータス
	int		timer;					// タイマー
	int		anim_index;				// アニメインデックス
	PARTS	parts[JINTAI_PARTS];	// パーツ
} JINTAI;

typedef struct {
	int		parent;				// 親パーツ番号
	int		type;				// 描画種別（０：直線、１：円）
	float	length;				// 長さ
	float	offx, offy;			// オフセット値
} PARTS_DATA;

typedef struct {
	int		time;					// 時刻
	float	angle[JINTAI_PARTS];	// 角度
} ANIM_DATA;

//----- 変数 -----
JINTAI	jintai;				// 人体モデル
bool	g_bOnTimer;			// アニメ実行中フラグ
HPEN	g_hpenBlue;			// カスタムペン

PARTS_DATA jintai_data[JINTAI_PARTS] = {				// パーツ初期データ
	// parent type length offx offy
	{      -1,   0,    80,   0,   0,},	// パーツ０　胴体
	{       0,   1,    80,   0, -80,},	// パーツ１　頭
	{       0,   0,    50,   0, -80,},	// パーツ２　右上腕
	{       2,   0,    50,   0, -50,},	// パーツ３　右下腕
	{       0,   0,    50,   0, -80,},	// パーツ４　左上腕
	{       4,   0,    50,   0, -50,},	// パーツ５　左下腕
	{       0,   0,    70,   0,   0,},	// パーツ６　右上腿
	{       6,   0,    70,   0, -70,},	// パーツ７　右下腿
	{       0,   0,    70,   0,   0,},	// パーツ８　左上腿
	{       8,   0,    70,   0, -70,},	// パーツ９　左下腿
};

ANIM_DATA anim_data[KEYFRAME_MAX] = {					// アニメーションデータ（角度のみ）
	// time angle
	{     0,    0, 0, 135, 45, -135, -45,  160, 20, -160, -20,},		// フレーム０（立ちポーズ）

	{    30,    0, 0,  90,  0,  -90,   0,  140, 30, -140, -30,},		// フレーム１
	{    60,    0, 0,  90,  0,  -90,   0,  140, 30, -140, -30,},		// フレーム２
	{    70,   45, 0, 135, 90,    0,  20,   90, 30, -180, -30,},		// フレーム３
	{    80,    0, 0,  90,  0,  -90,   0,  140, 30, -140, -30,},		// フレーム４
	{    90,   45, 0, 135, 90,    0,  20,   90, 30, -180, -30,},		// フレーム５
	{   100,    0, 0,  90,  0,  -90,   0,  140, 30, -140, -30,},		// フレーム６
	{   110,  -45, 0,   0,-20, -135, -90,  180, 30,  -90, -30,},		// フレーム７
	{   120,    0, 0,  90,  0,  -90,   0,  140, 30, -140, -30,},		// フレーム８
	{   130,  -45, 0,   0,-20, -135, -90,  180, 30,  -90, -30,},		// フレーム９
	{   140,    0, 0,  90,  0,  -90,   0,  140, 30, -140, -30,},		// フレーム１０

	{   150,   45, 0, 135, 90,    0,  20,   90, 30, -180, -30,},		// フレーム１１
	{   160,    0, 0,  90,  0,  -90,   0,  140, 30, -140, -30,},		// フレーム１２
	{   170,   45, 0, 135, 90,    0,  20,   90, 30, -180, -30,},		// フレーム１３
	{   200,   45, 0, 135, 90,    0,  20,   90, 30, -180, -30,},		// フレーム１４
	{   201,   50, 0, 125, 80,    0,  20,   85, 30, -185, -30,},		// フレーム１５
	{   240,   50, 0, 125, 80,    0,  20,   85, 30, -185, -30,},		// フレーム１６

	{   242,   50, 0, 125, 80,    0,  20,   85, 30, -185, -25,},		// フレーム１７
	{   244,   50, 0, 125, 80,    0,  20,   85, 30, -185, -30,},		// フレーム１８
	{   246,   50, 0, 125, 80,    0,  20,   85, 30, -185, -25,},		// フレーム１９
	{   260,   50, 0, 125, 80,    0,  20,   85, 30, -185, -30,},		// フレーム２０
	{   262,   50, 0, 125, 80,    0,  20,   85, 30, -185, -25,},		// フレーム２１
	{   264,   50, 0, 125, 80,    0,  20,   85, 30, -185, -30,},		// フレーム２２
	{   266,   50, 0, 125, 80,    0,  20,   85, 30, -185, -25,},		// フレーム２３
	{   280,   50, 0, 125, 80,    0,  20,   85, 30, -185, -30,},		// フレーム２４
	{   350,   50, 0, 125, 80,    0,  20,   85, 30, -185, -130,},		// フレーム２５
};

static TCHAR szClassName[] = _T("KeyFrameAnim");

// プロトタイプ
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CalcMatrix();
void SetAnimData(int index);
void ExecJintai();
void DrawModel(HDC hdc);

//---------------------------------------------------------------------------------------
//	メイン
//---------------------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int iCmdShow)
{
	HWND	hWnd;
	MSG		msg;
	int		frx, fry, cpy;

	// ウィンドウクラスの登録
	WNDCLASSEX wc;
	wc.cbSize			= sizeof(WNDCLASSEX);
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= WindowProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon			= LoadIcon(hInstance, IDI_APPLICATION);
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= szClassName;
	wc.hIconSm			= NULL;
	if (RegisterClassEx(&wc) == 0) return false;	// ウィンドウクラスを登録

	// ウィンドウを作成
	frx = GetSystemMetrics(SM_CXFIXEDFRAME);
	fry = GetSystemMetrics(SM_CYFIXEDFRAME);
	cpy = GetSystemMetrics(SM_CYCAPTION);
	hWnd = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW, szClassName, _T("キーフレームアニメーション"),
		WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION,
		CW_USEDEFAULT, CW_USEDEFAULT,
		SCREEN_WIDTH + frx * 2, SCREEN_HEIGHT + fry * 2 + cpy,
		NULL, NULL, hInstance, NULL);
	if (hWnd == NULL) return false;
	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);

	g_bOnTimer    = false;
	jintai.status = 0;
	// メッセージループ
	msg.message = WM_NULL;
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		Sleep(1);
	}
	return msg.wParam;
}

//---------------------------------------------------------------------------------------
//	ウィンドウプロシージャ
//---------------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC			hdc;
	PAINTSTRUCT	ps;
	TCHAR		buf[256];
	int			len;

	switch (uMsg) {
	case WM_CREATE:
		g_hpenBlue = CreatePen(PS_SOLID, 4,RGB(0,0,255));	// カスタムペンを作成
		SetTimer(hWnd, 1, 20, NULL);
		return 0;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return 0;
	case WM_DESTROY:
		DeleteObject(g_hpenBlue);		// ペンを削除
		PostQuitMessage(0);
		return 0;
	case WM_TIMER:
		ExecJintai();
		InvalidateRect(hWnd, NULL, true);
		return 0;
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_ESCAPE:			// [ESC]
			PostMessage(hWnd, WM_DESTROY, 0, 0);
			return 0;
		case VK_SPACE:			// [SPACE]
			if (g_bOnTimer) {
				jintai.status = 0;		// フレーム確認モードへ
				g_bOnTimer = false;
			} else {
				jintai.status = 2;		// アニメ実行モードへ
				g_bOnTimer = true;
			}
			return 0;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		DrawModel(hdc);		// モデルを描画
		// 各種情報を描画
		if (g_bOnTimer) {
			len = _stprintf(buf, _T("アニメ実行モード"));					TextOut(hdc, 0,  0, buf, len);
			len = _stprintf(buf, _T("フレーム＝%d"), jintai.anim_index);	TextOut(hdc, 0, 20, buf, len);
			len = _stprintf(buf, _T("タイマー＝%d"), jintai.timer);			TextOut(hdc, 0, 40, buf, len);
		} else {
			len = _stprintf(buf, _T("フレーム確認モード"));					TextOut(hdc, 0,  0, buf, len);
			len = _stprintf(buf, _T("フレーム＝%d"), jintai.anim_index);	TextOut(hdc, 0, 20, buf, len);
		}
		EndPaint(hWnd, &ps);
		return 0;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//---------------------------------------------------------------------------------------
//	角度からマトリックスを計算（全パーツ分）
//---------------------------------------------------------------------------------------
void CalcMatrix()
{
	for (int i = 0; i < JINTAI_PARTS; i++) {
		jintai.parts[i].matrix.m[0][0] =  cosf(jintai.parts[i].angle * RAD);
		jintai.parts[i].matrix.m[0][1] = -sinf(jintai.parts[i].angle * RAD);
		jintai.parts[i].matrix.m[0][2] = 0.0f;
		jintai.parts[i].matrix.m[1][0] =  sinf(jintai.parts[i].angle * RAD);
		jintai.parts[i].matrix.m[1][1] =  cosf(jintai.parts[i].angle * RAD);
		jintai.parts[i].matrix.m[1][2] = 0.0f;
		jintai.parts[i].matrix.m[2][0] = jintai.parts[i].offx;
		jintai.parts[i].matrix.m[2][1] = jintai.parts[i].offy;
		jintai.parts[i].matrix.m[2][2] = 1.0f;
	}
}

//---------------------------------------------------------------------------------------
//	アニメーションデータを人体モデルにセット
//---------------------------------------------------------------------------------------
void SetAnimData(int index)
{
	for (int i = 0; i < JINTAI_PARTS; i++) {
		jintai.parts[i].angle = anim_data[index].angle[i];
	}
	CalcMatrix();
	jintai.anim_index = index;
}

//---------------------------------------------------------------------------------------
//	人体モデル　メイン処理
//---------------------------------------------------------------------------------------
void ExecJintai()
{
	static	bool	key_on;
	float			dt, dg;		// 時間の差分、角度の差分
	int				i;

	switch (jintai.status) {
	case 0:			// 初期化
		for (i = 0; i < JINTAI_PARTS; i++) {
			jintai.parts[i].parent = jintai_data[i].parent;
			jintai.parts[i].type   = jintai_data[i].type;
			jintai.parts[i].length = jintai_data[i].length;
			jintai.parts[i].offx   = jintai_data[i].offx;
			jintai.parts[i].offy   = jintai_data[i].offy;
			jintai.parts[i].angle  = anim_data[0].angle[i];
		}
		CalcMatrix();
		jintai.timer      = 0;
		jintai.anim_index = 0;
		key_on            = false;
		jintai.status     = 1;
		// THRU
	case 1:			// キー入力でフレームデータを確認
		if (GetAsyncKeyState(VK_LEFT) & 0x8000) {				// カーソル（左）
			if (key_on == false) {
				if (jintai.anim_index > 0) SetAnimData(--jintai.anim_index);
				key_on = true;
			}
		} else if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {		// カーソル（右）
			if (key_on == false) {
				if (jintai.anim_index < KEYFRAME_MAX - 1) SetAnimData(++jintai.anim_index);
				key_on = true;
			}
		} else {
			key_on = false;
		}
		break;
	case 2:			// アニメ実行準備
		jintai.timer      = 0;
		jintai.anim_index = 0;
		jintai.status     = 3;
		// THRU
	case 3:			// アニメ実行中

		// 時間から角度を線形補間する
		dt = (float)(jintai.timer - anim_data[jintai.anim_index].time) /
					(anim_data[jintai.anim_index + 1].time - anim_data[jintai.anim_index].time);
		for (i = 0; i < JINTAI_PARTS; i++) {
			dg = (float)(anim_data[jintai.anim_index + 1].angle[i] - anim_data[jintai.anim_index].angle[i]);
			jintai.parts[i].angle = anim_data[jintai.anim_index].angle[i] + (int)(dg*dt);
		}

		CalcMatrix();

		// 時間とフレームを進める
		jintai.timer++;
		if (jintai.timer >= anim_data[jintai.anim_index + 1].time) {
			// 次のフレームへ移行
			jintai.anim_index++;
			if (jintai.anim_index >= KEYFRAME_MAX - 1) {
				jintai.status = 0;		// 最後まで完了したのでフレーム確認モードへ
				g_bOnTimer = false;
			}
		}
		break;
	}
}

//---------------------------------------------------------------------------------------
//	人体モデル　描画処理
//---------------------------------------------------------------------------------------
void DrawModel(HDC hdc)
{
	int		i, pr;
	float	x0, y0, x1, y1, xx, yy;
	MATRIX	m0, m1;
	HPEN	hpenOld;

	if (jintai.status == 0) return;		// まだ初期化されてなかったら即終了

	hpenOld = (HPEN)SelectObject(hdc, g_hpenBlue);	// ペンを選択
	for (i = 0; i < JINTAI_PARTS; i++) {
		m0.m[0][0] = jintai.parts[i].matrix.m[0][0];
		m0.m[0][1] = jintai.parts[i].matrix.m[0][1];
		m0.m[0][2] = jintai.parts[i].matrix.m[0][2];
		m0.m[1][0] = jintai.parts[i].matrix.m[1][0];
		m0.m[1][1] = jintai.parts[i].matrix.m[1][1];
		m0.m[1][2] = jintai.parts[i].matrix.m[1][2];
		m0.m[2][0] = jintai.parts[i].matrix.m[2][0];
		m0.m[2][1] = jintai.parts[i].matrix.m[2][1];
		m0.m[2][2] = jintai.parts[i].matrix.m[2][2];
		pr = i;
		while (jintai.parts[pr].parent != -1) {		// ワールドまでたどる
			// 親パーツのマトリックスを乗算する
			pr = jintai.parts[pr].parent;
			m1.m[0][0] =  m0.m[0][0] * jintai.parts[pr].matrix.m[0][0]
						+ m0.m[0][1] * jintai.parts[pr].matrix.m[1][0]
						+ m0.m[0][2] * jintai.parts[pr].matrix.m[2][0];
			m1.m[0][1] =  m0.m[0][0] * jintai.parts[pr].matrix.m[0][1]
						+ m0.m[0][1] * jintai.parts[pr].matrix.m[1][1]
						+ m0.m[0][2] * jintai.parts[pr].matrix.m[2][1];
			m1.m[0][2] =  m0.m[0][0] * jintai.parts[pr].matrix.m[0][2]
						+ m0.m[0][1] * jintai.parts[pr].matrix.m[1][2]
						+ m0.m[0][2] * jintai.parts[pr].matrix.m[2][2];
			m1.m[1][0] =  m0.m[1][0] * jintai.parts[pr].matrix.m[0][0]
						+ m0.m[1][1] * jintai.parts[pr].matrix.m[1][0]
						+ m0.m[1][2] * jintai.parts[pr].matrix.m[2][0];
			m1.m[1][1] =  m0.m[1][0] * jintai.parts[pr].matrix.m[0][1]
						+ m0.m[1][1] * jintai.parts[pr].matrix.m[1][1]
						+ m0.m[1][2] * jintai.parts[pr].matrix.m[2][1];
			m1.m[1][2] =  m0.m[1][0] * jintai.parts[pr].matrix.m[0][2]
						+ m0.m[1][1] * jintai.parts[pr].matrix.m[1][2]
						+ m0.m[1][2] * jintai.parts[pr].matrix.m[2][2];
			m1.m[2][0] =  m0.m[2][0] * jintai.parts[pr].matrix.m[0][0]
						+ m0.m[2][1] * jintai.parts[pr].matrix.m[1][0]
						+ m0.m[2][2] * jintai.parts[pr].matrix.m[2][0];
			m1.m[2][1] =  m0.m[2][0] * jintai.parts[pr].matrix.m[0][1]
						+ m0.m[2][1] * jintai.parts[pr].matrix.m[1][1]
						+ m0.m[2][2] * jintai.parts[pr].matrix.m[2][1];
			m1.m[2][2] =  m0.m[2][0] * jintai.parts[pr].matrix.m[0][2]
						+ m0.m[2][1] * jintai.parts[pr].matrix.m[1][2]
						+ m0.m[2][2] * jintai.parts[pr].matrix.m[2][2];
			m0 = m1;
		}
		// パーツの描画用端点を求める
		xx = 0;		yy = 0;
		x0 = xx * m0.m[0][0] + yy * m0.m[1][0] + m0.m[2][0];
		y0 = xx * m0.m[0][1] + yy * m0.m[1][1] + m0.m[2][1];
		xx = 0;		yy = 0 - jintai.parts[i].length;
		x1 = xx * m0.m[0][0] + yy * m0.m[1][0] + m0.m[2][0];
		y1 = xx * m0.m[0][1] + yy * m0.m[1][1] + m0.m[2][1];
		// タイプによって描画を変える
		switch (jintai.parts[i].type) {
		case 0:			// 直線
			MoveToEx(hdc, (int)x0 + SCREEN_WIDTH/2, (int)y0 + SCREEN_HEIGHT/2, NULL);
			LineTo  (hdc, (int)x1 + SCREEN_WIDTH/2, (int)y1 + SCREEN_HEIGHT/2);
			break;
		case 1:			// 円
			Ellipse(hdc, (int)(x0 - jintai.parts[i].length/2) + SCREEN_WIDTH /2,
						 (int)(y0 - jintai.parts[i].length  ) + SCREEN_HEIGHT/2,
						 (int)(x0 + jintai.parts[i].length/2) + SCREEN_WIDTH /2,
						 (int)(y0 + SCREEN_HEIGHT/2));
			break;
		}
	}
	SelectObject(hdc, hpenOld);
}

//=======================================================================================
//	end
//=======================================================================================
