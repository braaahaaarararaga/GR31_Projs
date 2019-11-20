//=================================================================================================
//
//	  Alpha Blend Test Program
//
//=================================================================================================

//-------- インクルードヘッダ
#define STRICT				// Windowsプログラムに対する「型保証」を強化
#include <windows.h>		// Windowsプログラムにはこれを付ける
#include <tchar.h>			// 汎用テキスト マッピング
#include <stdio.h>

//-------- 定数定義
#define CLASS_NAME		_T("AlphaBlend")					// ウィンドウクラス名
#define CAPTION_NAME	_T("Test Program")	// キャプション名
#define SCREEN_WIDTH	640									// スクリーン＆ビットマップ幅
#define SCREEN_HEIGHT	480									// スクリーン＆ビットマップ高さ
#define ALPHA_VALUE		0.5									// アルファ値
#define TILE_MAX		100									// タイル総数

//-------- 構造体定義
typedef union {				// ピクセル構造
	DWORD	rgba;
	BYTE	color[4];
} PIXEL;

typedef struct {			// タイルオブジェクト
	int		status;				// ステータス
	int		px, py;				// 座標
	int		vx, vy;				// 速度
	int		w, h;				// サイズ
	BYTE	r;				// カラー
	BYTE	g;				// カラー
	BYTE	b;				// カラー
} OBJ_TILE;

//-------- グローバル変数定義
HDC					g_hMemDC;				// メモリＤＣ
HBITMAP				g_hMemBitmap;			// メモリビットマップハンドル
bool				g_bDrawEnable;			// 画面描画許可フラグ
DWORD				*g_pBitmap;				// ビットマップ本体へのポインタ
int					g_LineLength;			// １行あたりのバイト数
OBJ_TILE			g_ObjTile[TILE_MAX];	// タイルオブジェクト

//-------------------------------------------------------------------------------------------------
// プロトタイプ宣言
//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ExecTileObject();

//-------------------------------------------------------------------------------------------------
//	メイン
//-------------------------------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int iCmdShow)
{
	HWND	hWnd;		// ウィンドウハンドル
	MSG		msg;		// メッセージ情報

	g_bDrawEnable = false;
	srand(GetTickCount());

	// ウィンドウクラスの登録
	WNDCLASSEX wc = {
		sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WindowProc,
		0, 0, hInstance, LoadIcon(hInstance, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)GetStockObject(WHITE_BRUSH), NULL, CLASS_NAME, NULL};
	if (RegisterClassEx(&wc) == 0) return false;	// ウィンドウクラスを登録

	// ウィンドウを作成
	hWnd = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW, CLASS_NAME, CAPTION_NAME, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION,
		CW_USEDEFAULT, CW_USEDEFAULT,
		SCREEN_WIDTH  + (GetSystemMetrics(SM_CXFIXEDFRAME) + GetSystemMetrics(SM_CXEDGE)) * 2,
		SCREEN_HEIGHT + (GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYEDGE)) * 2 + GetSystemMetrics(SM_CYCAPTION),
		NULL, NULL, hInstance, NULL);
	if (hWnd == NULL) return false;
	ShowWindow(hWnd, iCmdShow);					// ウィンドウの表示状態を設定
	UpdateWindow(hWnd);							// ウィンドウのクライアント領域を更新

	// メッセージループ
	msg.message = WM_NULL;
	while (WM_QUIT != msg.message) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {	// メッセージをチェック
			TranslateMessage(&msg);						// 仮想キーメッセージを文字メッセージへ変換
			DispatchMessage(&msg);						// ウィンドウプロシージャへメッセージを送出
		} else if (g_bDrawEnable) {
			ExecTileObject();							// タイル移動＆半透明合成処理
			InvalidateRect(hWnd, NULL, FALSE);			// 画面の再描画
		}
		Sleep(1);
	}
	return msg.wParam;
}

//-------------------------------------------------------------------------------------------------
//	ウィンドウプロシージャ
//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC			hdc;
	PAINTSTRUCT	ps;
	UINT		i;
	LPBYTE		biBits;
	BITMAP		bitmap;	// メモリビットマップ
	static BITMAPINFOHEADER	bi;
	static HBITMAP	hbmOld;


	switch (uMsg) {						// メッセージをチェック
	case WM_CREATE:
		hdc = GetDC(hWnd);
		ZeroMemory(&bi, sizeof(bi));
		bi.biSize        = sizeof(bi);
		bi.biWidth       = SCREEN_WIDTH;
		bi.biHeight      = SCREEN_HEIGHT;
		bi.biPlanes      = 1;
		bi.biBitCount    = 32;
		bi.biCompression = BI_RGB;
		g_hMemBitmap = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, (VOID**)&biBits, NULL, 0);
		g_hMemDC     = CreateCompatibleDC(hdc);
		hbmOld = (HBITMAP)SelectObject(g_hMemDC, g_hMemBitmap);
		ReleaseDC(hWnd, hdc);

		// ビットマップ情報を取得
		GetObject(g_hMemBitmap, sizeof(BITMAP), &bitmap);
		g_pBitmap = (DWORD*)bitmap.bmBits;				// ピクセルの先頭アドレスを取得
		g_LineLength = bitmap.bmWidthBytes / 4;			// １行あたりのピクセル数を求める

		PatBlt(g_hMemDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x00000000);	// 黒で塗りつぶし

		for (i = 0; i < TILE_MAX; i++) g_ObjTile[i].status = 0;		// タイルオブジェクトのステータスを初期化

		g_bDrawEnable = true;
		return 0;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return 0;
	case WM_DESTROY:							// 終了指示がきた
		g_bDrawEnable = false;
		SelectObject(g_hMemDC, hbmOld);
		DeleteObject(g_hMemBitmap);
		DeleteDC(g_hMemDC);
		PostQuitMessage(0);							// システムにスレッドの終了を要求
		return 0;
	case WM_KEYDOWN:							// キーボードが押された
		switch (wParam) {							// パラメータをチェック
		case VK_ESCAPE:								// [ESC]キーが押された
			DestroyWindow(hWnd);					// ウィンドウを破棄する	
			return 0;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, g_hMemDC, 0, 0, SRCCOPY);		// メモリＤＣを描画
		EndPaint(hWnd, &ps);
		return 0;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);	// 既定ウィンドウプロシージャを呼出
}

//-------------------------------------------------------------------------------------------------
// タイルオブジェクト
//-------------------------------------------------------------------------------------------------
void ExecTileObject()
{
	int			i, x, y;

	// 白で塗りつぶす
	PatBlt(g_hMemDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x00000000);
	
	for (i = 0; i < TILE_MAX; i++) {
		switch (g_ObjTile[i].status) {
		case 0:
			g_ObjTile[i].w  = rand() % 128 + 64;
			g_ObjTile[i].h  = rand() % 128 + 64;
			g_ObjTile[i].px = rand() % (SCREEN_WIDTH  - g_ObjTile[i].w);
			g_ObjTile[i].py = rand() % (SCREEN_HEIGHT - g_ObjTile[i].h);
			g_ObjTile[i].vx = rand() % 11 - 5;
			g_ObjTile[i].vy = rand() % 11 - 5;
			g_ObjTile[i].r = 16;
			g_ObjTile[i].g = 16;
			g_ObjTile[i].b = 16;
			g_ObjTile[i].status = 1;
			// THRU
		case 1:
			g_ObjTile[i].px += g_ObjTile[i].vx;
			g_ObjTile[i].py += g_ObjTile[i].vy;
			if (g_ObjTile[i].px < 0 || g_ObjTile[i].px >= (int)(SCREEN_WIDTH  - g_ObjTile[i].w)) {
				g_ObjTile[i].vx *= -1;
				g_ObjTile[i].px += g_ObjTile[i].vx;
			}
			if (g_ObjTile[i].py < 0 || g_ObjTile[i].py >= (int)(SCREEN_HEIGHT - g_ObjTile[i].h)) {
				g_ObjTile[i].vy *= -1;
				g_ObjTile[i].py += g_ObjTile[i].vy;
			}
			break;
		}

		// ピクセル数分ループ
		for (y = 0; y < g_ObjTile[i].h; y++) {
			for (x = 0; x < g_ObjTile[i].w; x++) {
				// タイルの座標から先頭ピクセルの位置を求める
				DWORD* buf = g_pBitmap + ((g_ObjTile[i].py + y) * g_LineLength + (g_ObjTile[i].px + x));

				PIXEL tmp;
				tmp.rgba = *buf;

				float r1 = tmp.color[2];
				float g1 = tmp.color[1];
				float b1 = tmp.color[0];

				float r2 = g_ObjTile[i].r;
				float g2 = g_ObjTile[i].g;
				float b2 = g_ObjTile[i].b;
					
				{
					float r3 = r1 + r2;
					float g3 = g1 + g2;
					float b3 = b1 + b2;

					if (r3 > 255.0f) r3 = 255.0f;
					if (g3 > 255.0f) g3 = 255.0f;
					if (b3 > 255.0f) b3 = 255.0f;

					tmp.color[2] = (BYTE)r3;
					tmp.color[1] = (BYTE)g3;
					tmp.color[0] = (BYTE)b3;
					tmp.color[3] = 0x00;
				}
				*buf = tmp.rgba;
			}
		}
	}
}

//=================================================================================================
//	end
//=================================================================================================
