//=================================================================================================
//
//	  [ EdgeDetect ] エッジ検出プログラム
//    スペースキーで「元画像」「処理後画像」を切り替えて表示する
//
//=================================================================================================

//-------- インクルードヘッダ
#define STRICT				// Windowsプログラムに対する「型保証」を強化
#include <windows.h>		// Windowsプログラムにはこれを付ける
#include <tchar.h>			// 汎用テキスト マッピング
#include <math.h>

//-------- 定数定義
#define CLASS_NAME		_T("EdgeDetect")							// ウィンドウクラス名
#define CAPTION_NAME	_T("[ EdgeDetect ] エッジ検出プログラム　＊完成形＊")	// キャプション名
const UINT	SCREEN_WIDTH	= 500;									// スクリーン＆ビットマップ幅
const UINT	SCREEN_HEIGHT	= 500;									// スクリーン＆ビットマップ高さ
const UINT	PIXEL_QTY		= SCREEN_WIDTH * SCREEN_HEIGHT;			// ピクセル総数

#define FILE_NAME		_T("sample.bmp")

//-------- 構造体定義
typedef union {
	DWORD	dword;
	BYTE	byte[4];
} PIXEL;

//-------- グローバル変数定義
HINSTANCE	g_hInst;			// インスタンス
HDC			g_hMemDC[2];		// メモリＤＣ

//-------------------------------------------------------------------------------------------------
// プロトタイプ宣言
//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ExecEdgeDetect(DWORD* srcpt, DWORD* dstpt);

//-------------------------------------------------------------------------------------------------
//	メイン
//-------------------------------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int iCmdShow)
{
	HWND	hWnd;		// ウィンドウハンドル
	MSG		msg;		// メッセージ情報
	BOOL	bRet;

	g_hInst = hInstance;

	// ウィンドウクラスの登録
	WNDCLASSEX wc = {
		sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WindowProc,
		0, 0, hInstance, LoadIcon(hInstance, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)GetStockObject(WHITE_BRUSH), NULL, CLASS_NAME, NULL};
	if (RegisterClassEx(&wc) == 0) return false;	// ウィンドウクラスを登録

	// ウィンドウを作成
	hWnd = CreateWindowEx(
		WS_EX_TOPMOST, CLASS_NAME, CAPTION_NAME, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION,
		CW_USEDEFAULT, CW_USEDEFAULT,
		SCREEN_WIDTH  + GetSystemMetrics(SM_CXFIXEDFRAME) * 2,
		SCREEN_HEIGHT + GetSystemMetrics(SM_CYFIXEDFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION),
		NULL, NULL, hInstance, NULL);
	if (hWnd == NULL) return false;
	ShowWindow(hWnd, iCmdShow);					// ウィンドウの表示状態を設定
	UpdateWindow(hWnd);							// ウィンドウのクライアント領域を更新

	// メッセージループ
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {	// WM_QUIT メッセージが来るまでループ
		if (bRet == -1)									// エラーが発生
			break;
		TranslateMessage(&msg);							// 仮想キーメッセージを文字メッセージへ変換
		DispatchMessage(&msg);							// ウィンドウプロシージャへメッセージを送出
	}
	return msg.wParam;
}

//-------------------------------------------------------------------------------------------------
//	ウィンドウプロシージャ
//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC			hdc;
	HBITMAP		hSrcBmp, hDstBmp;
	BITMAP		SrcBmpInfo, DstBmpInfo;
	PAINTSTRUCT	ps;
	DWORD		*dstpt, *srcpt;
	static int	draw_sw;
	static LPTSTR	strdata[] = { LPTSTR("元画像"), LPTSTR("処理後画像")};

	switch (uMsg) {						// メッセージをチェック
	case WM_CREATE:
		draw_sw = 0;			// 元画像
		hdc = GetDC(hWnd);
		// ビットマップファイルをロード
		// （注意）使用するＢＭＰファイルは 640x480 32bitカラーでないといけない
		hDstBmp = (HBITMAP)LoadImage(g_hInst, FILE_NAME, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
		hSrcBmp = (HBITMAP)LoadImage(g_hInst, FILE_NAME, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
		// ビットマップ情報を取得
		GetObject(hDstBmp, sizeof(BITMAP), &DstBmpInfo);
		GetObject(hSrcBmp, sizeof(BITMAP), &SrcBmpInfo);
		// ピクセルの先頭アドレスを取得
		dstpt = (DWORD*)DstBmpInfo.bmBits;
		srcpt = (DWORD*)SrcBmpInfo.bmBits;

		ExecEdgeDetect(srcpt, dstpt);	// エッジ検出関数

		g_hMemDC[0] = CreateCompatibleDC(hdc);
		g_hMemDC[1] = CreateCompatibleDC(hdc);
		SelectObject(g_hMemDC[0], hSrcBmp);
		SelectObject(g_hMemDC[1], hDstBmp);
		DeleteObject(hDstBmp);
		DeleteObject(hSrcBmp);
		ReleaseDC(hWnd, hdc);
		return 0;
	case WM_CLOSE:
		DeleteDC(g_hMemDC[0]);
		DeleteDC(g_hMemDC[1]);
		DestroyWindow(hWnd);
		return 0;
	case WM_DESTROY:							// 終了指示がきた
		PostQuitMessage(0);							// システムにスレッドの終了を要求
		return 0;
	case WM_KEYDOWN:							// キーボードが押された
		switch (wParam) {							// パラメータをチェック
		case VK_SPACE:								// スペースで画像チェンジ
			draw_sw ^= 1;
			InvalidateRect(hWnd, NULL, false);
			return 0;
		case VK_ESCAPE:								// [ESC]キーが押された
			PostMessage(hWnd, WM_CLOSE, 0, 0);		// 終了のメッセージを発生させる
			return 0;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// メモリＤＣを描画
		BitBlt(hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, g_hMemDC[draw_sw], 0, 0, SRCCOPY);
		// テキスト表示
		TextOut(hdc, 0, 0, strdata[draw_sw], lstrlen(strdata[draw_sw]));
		EndPaint(hWnd, &ps);
		return 0;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);	// 既定ウィンドウプロシージャを呼出
}

//-------------------------------------------------------------------------------------------------
// エッジ検出関数
//-------------------------------------------------------------------------------------------------
void ExecEdgeDetect(DWORD* srcpt, DWORD* dstpt)
{
	int		i, j, x, y;					// ループカウンタ
	int		tmp_v, tmp_h, tmp;			// テンポラリ
	PIXEL	src_pixel[9], ans_pixel;	// 元画像ピクセルデータ、計算後ピクセルデータ

	// 水平方向Sobelフィルタ
	DWORD		sobel_h[9];
	sobel_h[0] = 1;		sobel_h[1] = 2;		sobel_h[2] = 1;
	sobel_h[3] = 0;		sobel_h[4] = 0;		sobel_h[5] = 0;
	sobel_h[6] = -1;	sobel_h[7] = -2;	sobel_h[8] = -1;

	// 垂直方向Sobelフィルタ
	DWORD		sobel_v[9];
	sobel_v[0] = -1;	sobel_v[1] = 0;	sobel_v[2] = 1;
	sobel_v[3] = -2;	sobel_v[4] = 0;	sobel_v[5] = 2;
	sobel_v[6] = -1;	sobel_v[7] = 0;	sobel_v[8] = 1;

	// Ｙ方向：ピクセル数分(=SCREEN_HEIGHT)だけループ
	for (y = 0; y < SCREEN_HEIGHT; y++) {
		// Ｘ方向：ピクセル数分(=SCREEN_WIDTH)だけループ
		for (x = 0; x < SCREEN_WIDTH; x++) {
			// srcpt を中心としたピクセルと周囲８ピクセルの値、合計９個の値を取得して
			// src_pixel に格納（周囲のピクセルが「範囲外」の場合は値を０とする）
			src_pixel[0].dword = ((y <= 0                ) || (x <= 0               ))? 0: *(srcpt + (y - 1) * SCREEN_WIDTH + (x - 1));
			src_pixel[1].dword = ((y <= 0                )                           )? 0: *(srcpt + (y - 1) * SCREEN_WIDTH + (x));
			src_pixel[2].dword = ((y <= 0                ) || (x >= SCREEN_WIDTH - 1))? 0: *(srcpt + (y - 1) * SCREEN_WIDTH + (x + 1));
			src_pixel[3].dword = (                            (x <= 0               ))? 0: *(srcpt + (y    ) * SCREEN_WIDTH + (x - 1));
			src_pixel[4].dword =                                                           *(srcpt + (y    ) * SCREEN_WIDTH + (x));
			src_pixel[5].dword = (                            (x >= SCREEN_WIDTH - 1))? 0: *(srcpt + (y    ) * SCREEN_WIDTH + (x + 1));
			src_pixel[6].dword = ((y >= SCREEN_HEIGHT - 1) || (x <= 0               ))? 0: *(srcpt + (y + 1) * SCREEN_WIDTH + (x - 1));
			src_pixel[7].dword = ((y >= SCREEN_HEIGHT - 1)                           )? 0: *(srcpt + (y + 1) * SCREEN_WIDTH + (x));
			src_pixel[8].dword = ((y >= SCREEN_HEIGHT - 1) || (x >= SCREEN_WIDTH - 1))? 0: *(srcpt + (y + 1) * SCREEN_WIDTH + (x + 1));
			// ＲＧＢのため３回ループで計算を行う
			for (j = 0; j < 3; j++) {
				// 水平方向、垂直方向のテンポラリを０にクリア
				tmp_v = tmp_h = 0;
				// ９ピクセル分ループ
				for (i = 0; i < 9; i++) {
					// ピクセルデータに垂直方向フィルタをかけ、結果を合成
					tmp_v += (src_pixel[i].byte[j] * sobel_v[i]);
					// ピクセルデータに水平方向フィルタをかけ、結果を合成
					tmp_h += (src_pixel[i].byte[j] * sobel_h[i]);
				}
				// テンポラリから最終ピクセル値を求める
				tmp = (int)sqrt((double)tmp_v * tmp_v + tmp_h * tmp_h);
				// 値が閾値をオーバーしていないかチェックし、計算後ピクセルデータを確定
				if (tmp > 128) {
					ans_pixel.byte[0] = 255;
					ans_pixel.byte[1] = 255;
					ans_pixel.byte[2] = 255;
				}
				else {
					ans_pixel.byte[j] = tmp;
				}
			}
			// 確定したピクセルデータを出力バッファのポインタが指し示すアドレスへ書き込む
			*dstpt++ = ans_pixel.dword;
		}
	}
}

//=================================================================================================
//	end
//=================================================================================================
