//=================================================================================================
//    [ 環境マッピング（スフィア） ] OpenGL sample
//    環境マッピング（スフィア）サンプル
//
//    操作
//    　カーソル　鏡面モデル回転
//
//=================================================================================================
//-------- インクルードヘッダ
#define _CRT_SECURE_NO_WARNINGS
#define STRICT
#include <windows.h>		// Windowsプログラムにはこれを付ける
#include <tchar.h>			// 汎用テキスト マッピング
#include <GL/gl.h>			// OpenGL 関連のヘッダー
#include <GL/glu.h>			// OpenGL 関連のヘッダー
#include <stdio.h>			// 文字列処理で使用
#include <mmsystem.h>		// 時間管理で使用
#define _USE_MATH_DEFINES
#include <math.h>			// 円周率で必要

#include "tga.h"
#include "matrix.h"

//-------- ライブラリのリンク（こう記述しておけば別途リンク設定が不要となる）
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "winmm.lib")

//-------- 定数定義
#define CLASS_NAME		_T("Sphere")								// ウィンドウクラス名
#define CAPTION_NAME	_T("[ 環境マッピング（スフィア） 完成例 ]")		// キャプション名

const float FRAME_RATE		= 1000/60;								// フレームレート
const int	SCREEN_WIDTH	= 640;									// スクリーンの幅
const int	SCREEN_HEIGHT	= 480;									// スクリーンの高さ

const int	FONT_WIDTH		= 10;
const int	FONT_HEIGHT		= 16;

//----- 頂点フォーマット定義
struct VERTEX {
	// GL_T2F_N3F_V3F
	float		tu, tv;				// テクスチャ座標
	float		nx, ny, nz;			// 法線ベクトル
	float		x, y, z;			// 頂点座標（座標変換あり）
};

struct VERTEX2 {
	// GL_T2F_V3F
	float		tu, tv;				// テクスチャ座標
	float		x, y, z;			// 頂点座標
};

typedef struct {
	int			status;				// ステータス
	VECTOR		position;			// 座標
	VECTOR		rotation;			// 回転角
} MIRROR;

//----- 鏡面モデル定義
VERTEX mirror_vertex[] = {
	//  tu    tv     nx     ny       nz        x       y     z
	{ 0.0f, 0.0f, -0.3f,  0.3f, -0.905f, -256.0f, 256.0f, 0.0f},	// 頂点０
	{ 0.0f, 0.0f,  0.3f,  0.3f, -0.905f,  256.0f, 256.0f, 0.0f},	// 頂点１
	{ 0.0f, 0.0f,  0.3f, -0.3f, -0.905f,  256.0f,-256.0f, 0.0f},	// 頂点２
	{ 0.0f, 0.0f, -0.3f, -0.3f, -0.905f, -256.0f,-256.0f, 0.0f},	// 頂点３
};

//----- グローバル変数

// OpenGL 関連
HDC						g_hDC;						// デバイスコンテキスト
HGLRC					g_hRC;						// カレントコンテキスト
GLuint					g_uFontTexID;				// フォントテクスチャ ID
GLuint					g_uTextureID;				// テクスチャ ID
MATRIX					g_MatView;					// ビューマトリックス

MIRROR					g_Mirror;					// 鏡オブジェクト

TCHAR					g_szDebug[4096];			// 出力文字列バッファ
float					g_FPS;						// フレーム数カウント用

//----- プロトタイプ宣言
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
long Initialize3DEnvironment(HWND hWnd, bool bWindow);
HRESULT Cleanup3DEnvironment(HWND hWnd, bool bWindow);
HRESULT Render3DEnvironment();
void Init2D();
void Init3D();
void ExecMirror();
void DrawInfo();
GLuint LoadTexture(HWND hWnd, const char *fname);
void ReleaseTexture(GLuint* puTextureID);
void DrawChar(int x, int y, int c);
void DrawStr(int x, int y, LPCTSTR psz);

//-------------------------------------------------------------------------------------------------
// メイン
//-------------------------------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int iCmdShow)
{
	HWND	hWnd;
	MSG		msg;
	DWORD	dwExecLastTime, dwFPSLastTime, dwCurrentTime, dwFrameCount;
	float	dt;
	bool	bWindow;

	// ウィンドウクラスを登録
	WNDCLASS wndClass = {
		CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hInst, LoadIcon(hInst, IDI_APPLICATION),
		LoadCursor(NULL, IDC_ARROW), (HBRUSH)GetStockObject(WHITE_BRUSH), NULL, CLASS_NAME
	};
	if (RegisterClass(&wndClass) == 0) return false;

	// ウインドウを作成
	hWnd = CreateWindow(
		CLASS_NAME, CAPTION_NAME, WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		SCREEN_WIDTH  + GetSystemMetrics(SM_CXFIXEDFRAME) * 2,
		SCREEN_HEIGHT + GetSystemMetrics(SM_CYFIXEDFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION),
		NULL, NULL, hInst, NULL);
	if (hWnd == 0) return false;

	bWindow = false;
	if (IDYES == MessageBox(hWnd, _T("ウィンドウモードで実行しますか？"), _T("画面モード"), MB_YESNO))
		bWindow = true;
	if (FAILED(Initialize3DEnvironment(hWnd, bWindow))) return 0;	// ３Ｄ環境の初期化

	// 変数初期化
	timeBeginPeriod(1);									// システムタイマーの分解能を１ｍｓに設定
	dwFPSLastTime = dwExecLastTime = timeGetTime();		// 現在のシステムタイマーを取得
	dwFrameCount = 0;
	g_Mirror.status = 0;

	// メインウインドウループ
	msg.message = WM_NULL;
	while (WM_QUIT != msg.message) {									// WM_QUIT がくるまでループ
		if (PeekMessage(&msg,NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			dwCurrentTime = timeGetTime();								// 現在のタイマー値を取得
			if (dwCurrentTime - dwFPSLastTime >= 500) {					// ０．５秒ごとに計測
				dt = (float)(dwCurrentTime - dwFPSLastTime) / 1000;		// フレーム数を計算
				g_FPS = (float)dwFrameCount / dt;
				dwFPSLastTime = dwCurrentTime;							// タイマー値を更新
				dwFrameCount = 0;										// フレームカウンタをリセット
			}
			// この辺で時間管理
			if (dwCurrentTime - dwExecLastTime >= FRAME_RATE) {			// 一定時間が経過したら・・・
				dwExecLastTime = dwCurrentTime;							// タイマー値を更新
				Render3DEnvironment();									// 描画処理
				dwFrameCount++;											// フレームカウントを＋１
			}
		}
		Sleep(1);
	}
	timeEndPeriod(1);						// システムタイマーの分解能を元に戻す
	Cleanup3DEnvironment(hWnd, bWindow);
	return msg.wParam;
}

//-------------------------------------------------------------------------------------------------
// メッセージハンドラ
//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_DESTROY:
		ShowCursor(TRUE);
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_ESCAPE:
			DestroyWindow(hWnd);
			return 0;
		}
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//-------------------------------------------------------------------------------------------------
// ３Ｄ環境の初期化
//-------------------------------------------------------------------------------------------------
HRESULT Initialize3DEnvironment(HWND hWnd, bool bWindow)
{
	// ピクセルフォーマット
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,						// Version
		PFD_DRAW_TO_WINDOW |	// Supports GDI
		PFD_SUPPORT_OPENGL |	// Supports OpenGL
		PFD_DOUBLEBUFFER,		// Use double buffering (more efficient drawing)
		PFD_TYPE_RGBA,			// No pallettes
		24, 					// Number of color planes
		 						// in each color buffer
		8,	0,					// for Red-component
		8,	0,					// for Green-component
		8,	0,					// for Blue-component
		8,	0,					// for Alpha-component
		0,						// Number of planes
								// of Accumulation buffer
		16,						// for Red-component
		16,						// for Green-component
		16,						// for Blue-component
		16,						// for Alpha-component
		24, 					// Depth of Z-buffer
		0,						// Depth of Stencil-buffer
		0,						// Depth of Auxiliary-buffer
		0,						// Now is ignored
		0,						// Number of planes
		0,						// Now is ignored
		0,						// Color of transparent mask
		0						// Now is ignored
	};

	// ウィンドウ／フルスクリーン切り替え
	if (!bWindow) {
		DEVMODE	devmode;
		devmode.dmFields		= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		devmode.dmBitsPerPel	= 32;
		devmode.dmPelsWidth		= SCREEN_WIDTH;
		devmode.dmPelsHeight	= SCREEN_HEIGHT;
		devmode.dmSize			= sizeof devmode;
		if (ChangeDisplaySettings(&devmode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
			MessageBox(hWnd, _T("フルスクリーン切り替えに失敗しました"),
				_T("error"), MB_OK | MB_ICONERROR);
			return E_FAIL;
		}
		SetWindowLong(hWnd, GWL_STYLE, WS_VISIBLE);
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0,
			SCREEN_WIDTH, SCREEN_HEIGHT, SWP_SHOWWINDOW);
	}

	// ウィンドウコンテキストの取得
	g_hDC = GetDC(hWnd);

	// 合致する最も近いピクセルフォーマットの選択
	int nPixelFormat = ChoosePixelFormat(g_hDC, &pfd);
	if (!nPixelFormat) {
		MessageBox(hWnd, _T("ピクセルフォーマットの選択に失敗しました"), _T("error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// ピクセルフォーマットの設定
	if (!SetPixelFormat(g_hDC, nPixelFormat, &pfd)) {
		MessageBox(hWnd, _T("ピクセルフォーマットの設定に失敗しました"), _T("error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// OpenGLレンダリングコンテキストを作成
	g_hRC = wglCreateContext(g_hDC);
	if (!g_hRC) {
		MessageBox(hWnd, _T("レンダリングコンテキストの生成に失敗しました"), _T("error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// OpenGLレンダリングコンテキストをメイク
	if (!wglMakeCurrent(g_hDC, g_hRC)) {
		MessageBox(hWnd, _T("レンダリングコンテキストのメイクに失敗しました"), _T("error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// ビューポート設定
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// フォントセットアップ
	g_uFontTexID = LoadTexture(hWnd, "font.tga");

	// テクスチャ読み込み
	g_uTextureID = LoadTexture(hWnd, "spheremap.bmp");

	// レンダリングステートを設定
	glDisable(GL_LIGHTING);					// ライト無効化
	glEnable(GL_TEXTURE_2D);				// テクスチャを有効にする

	return S_OK;
}

//-------------------------------------------------------------------------------------------------
// ３Ｄ環境のクリーンアップ
//-------------------------------------------------------------------------------------------------
HRESULT Cleanup3DEnvironment(HWND hWnd, bool bWindow)
{
	ReleaseTexture(&g_uTextureID);		// テクスチャ解放
	ReleaseTexture(&g_uFontTexID);		// フォント用テクスチャ解放

	wglMakeCurrent(NULL, NULL);
	if (g_hRC) {						// カレントコンテキストを解放
		wglDeleteContext(g_hRC);
		g_hRC = NULL;
	}
	if (g_hDC) {						// デバイスコンテキストを解放
		ReleaseDC(hWnd, g_hDC);
		g_hDC = NULL;
	}
	if (!bWindow)
		ChangeDisplaySettings(NULL, 0);
	return S_OK;
}

//-------------------------------------------------------------------------------------------------
// ３Ｄ環境のレンダリング
//-------------------------------------------------------------------------------------------------
HRESULT Render3DEnvironment()
{
	g_szDebug[0] = _T('\0');		// デバッグストリングバッファ初期化

	// バックバッファとＺバッファをクリア
	glClearColor(0.0f, 0.375f, 0.5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Init3D();
	ExecMirror();					// 鏡面モデル実行、描画

	Init2D();
	DrawInfo();						// インフォメーション表示

	// バックバッファをプライマリバッファにコピー
	SwapBuffers(g_hDC);

	return S_OK;
}

//---------------------------------------------------------------------------------------
// 2D用レンダリングステート初期化
//---------------------------------------------------------------------------------------
void Init2D()
{
	// 描画設定
	glDisable(GL_CULL_FACE);	// カリングは行わない

	// 透過色(抜き色)を可能にする
	glEnable(GL_BLEND);			// αブレンド可
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// 透過色(抜き色)の部分をZバッファに書き込まない
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.0);

	// Zバッファ深度を無効化
	glDisable(GL_DEPTH_TEST);

	// 正射影マトリックスをプロジェクションマトリックスとして設定
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-SCREEN_WIDTH/2.0, SCREEN_WIDTH/2,
		-SCREEN_HEIGHT/2.0, SCREEN_HEIGHT/2.0, 0.0, 1.0);

	// ビューマトリックス設定
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

//---------------------------------------------------------------------------------------
// 3D用レンダリングステート初期化
//---------------------------------------------------------------------------------------
void Init3D()
{
	// ポリゴンの片面だけ描画を有効化
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// 透過色(抜き色)を無効にする
	glDisable(GL_BLEND);

	// αテストを無効にする
	glDisable(GL_ALPHA_TEST);

	// Zバッファ深度を有効化
	glEnable(GL_DEPTH_TEST);

	// プロジェクションマトリックス設定
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, (float)SCREEN_WIDTH/SCREEN_HEIGHT, 100.0f, 10000.0f);

	// ビューマトリックス設定
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, -800.0f,
			  0.0f, 0.0f, 0.0f,
			  0.0f, 1.0f, 0.0f);
	glGetFloatv(GL_MODELVIEW_MATRIX, g_MatView[0]);
}

//-------------------------------------------------------------------------------------------------
// 鏡面オブジェクト
//-------------------------------------------------------------------------------------------------
void ExecMirror()
{
	MATRIX	matRotateX, matRotateY, matRotateZ;		// 回転マトリックス（３種）
	MATRIX	matTrans;								// 平行移動マトリックス
	MATRIX	matWorld, matWV;
	VECTOR	v_norm;
	VECTOR	v_norm2;

	switch (g_Mirror.status) {
	case 0:
		g_Mirror.rotation[0] = g_Mirror.rotation[1] = g_Mirror.rotation[2] = 0.0f;
		g_Mirror.position[0] = g_Mirror.position[1] = g_Mirror.position[2] = 0.0f;
		g_Mirror.rotation[3] = g_Mirror.position[3] = 1.0f;
		g_Mirror.status = 1;
		// THRU
	case 1:
		// キーによる視点位置および鏡面モデルの移動
		if (GetAsyncKeyState(VK_UP     ) & 0x8000) g_Mirror.rotation[0] += 0.02f;		// カーソル（上）
		if (GetAsyncKeyState(VK_DOWN   ) & 0x8000) g_Mirror.rotation[0] -= 0.02f;		// カーソル（下）
		if (GetAsyncKeyState(VK_LEFT   ) & 0x8000) g_Mirror.rotation[1] -= 0.02f;		// カーソル（左）
		if (GetAsyncKeyState(VK_RIGHT  ) & 0x8000) g_Mirror.rotation[1] += 0.02f;		// カーソル（右）
		break;
	default:
		break;
	}
	// ワールドマトリックスを合成
	MatrixRotationX(matRotateX, g_Mirror.rotation[0]);							// Ｘ軸回転マトリックスを生成
	MatrixRotationY(matRotateY, g_Mirror.rotation[1]);							// Ｙ軸回転マトリックスを生成
	MatrixRotationZ(matRotateZ, g_Mirror.rotation[2]);							// Ｚ軸回転マトリックスを生成
	MatrixTranslation(matTrans, g_Mirror.position[0], g_Mirror.position[1], g_Mirror.position[2]);	// 平行移動マトリックスを生成
	MatrixMultiply(matWorld, matTrans, matRotateZ);
	MatrixMultiply(matWorld, matWorld, matRotateY);
	MatrixMultiply(matWorld, matWorld, matRotateX);

	// 現在のワールド、ビューマトリックスからビュー空間への変換マトリックスを生成
	MatrixMultiply(matWV, g_MatView, matWorld);

	//*************************************************************************
	//
	//	ここでスフィアマップによるテクスチャ座標計算処理をおこなう
	//
	//*************************************************************************
	for (int i = 0; i < 4; i++) {		// 鏡面モデルは４頂点
		// 鏡面モデルの頂点にある法線ベクトルをビュー空間に回転
		v_norm[0] = mirror_vertex[i].nx;
		v_norm[1] = mirror_vertex[i].ny;
		v_norm[2] = mirror_vertex[i].nz;
		v_norm[3] = 1.0f;

		VectorTransformCoord(v_norm2, matWV, v_norm);

		// スフィアマップのテクスチャ座標を割り当てる
		mirror_vertex[i].tu = 0.5f + 0.5f * v_norm2[0];
		mirror_vertex[i].tv = 0.5f + 0.5f * v_norm2[1];
	}
	//*************************************************************************

	// 描画
	glMultMatrixf(matWorld[0]);
	glInterleavedArrays(GL_T2F_N3F_V3F, 0, mirror_vertex);						// 頂点フォーマット設定
	glBindTexture(GL_TEXTURE_2D, g_uTextureID);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

//-------------------------------------------------------------------------------------------------
// インフォメーション表示
//-------------------------------------------------------------------------------------------------
void DrawInfo()
{
	static VERTEX2 m_info_vertex[] = {
		{ 0.0f, 0.0f, (float)SCREEN_WIDTH / 2 - 256.0f, (float)-SCREEN_HEIGHT / 2         , 0.0f},	// 頂点０
		{ 1.0f, 0.0f, (float)SCREEN_WIDTH / 2         , (float)-SCREEN_HEIGHT / 2         , 0.0f},	// 頂点１
		{ 1.0f, 1.0f, (float)SCREEN_WIDTH / 2         , (float)-SCREEN_HEIGHT / 2 + 256.0f, 0.0f},	// 頂点２
		{ 0.0f, 1.0f, (float)SCREEN_WIDTH / 2 - 256.0f, (float)-SCREEN_HEIGHT / 2 + 256.0f, 0.0f},	// 頂点３
	};

	static float	frame_vertex[2 * 5];
	int			i;
	TCHAR		str[256];

	// スフィアマップ　テクスチャ表示
	glBindTexture(GL_TEXTURE_2D, g_uTextureID);
	glInterleavedArrays(GL_T2F_V3F, 0, m_info_vertex);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// スフィアマップ　ポジション表示
	for (i = 0; i < 4; i++) {
		frame_vertex[i * 2    ] = (mirror_vertex[i].tu * 256.0f) + (float)(SCREEN_WIDTH  / 2 - 256);
		frame_vertex[i * 2 + 1] = (mirror_vertex[i].tv * 256.0f) - (float)(SCREEN_HEIGHT / 2);
	}
	frame_vertex[4 * 2    ] = frame_vertex[0 * 2    ];
	frame_vertex[4 * 2 + 1] = frame_vertex[0 * 2 + 1];
	glBindTexture(GL_TEXTURE_2D, 0);
	glInterleavedArrays(GL_V2F, 0, frame_vertex);
	glColor3f(1.0f, 0.0f, 0.0f);
	glDrawArrays(GL_LINE_STRIP, 0, 5);

	// デバッグストリング描画
	_stprintf(str, _T("FPS : %4.1f\n"), g_FPS);
	lstrcat(g_szDebug, str);
	for (i = 0; i < 4; i++) {
		_stprintf(str, _T("%2d: tu = %4.2f, tv = %4.2f\n"), i, mirror_vertex[i].tu, mirror_vertex[i].tv);
		lstrcat(g_szDebug, str);
	}
	glColor3f(1.0f, 1.0f, 1.0f);
	DrawStr(0, 0, g_szDebug);
}

//---------------------------------------------------------------------------------------
// BMP読込
//---------------------------------------------------------------------------------------
static TGAImage *LoadBMP(const char *pszFName)
{
	TGAImage			*pBmp;
	FILE				*fp;
	BITMAPFILEHEADER	bmfh;
	BITMAPINFOHEADER	bmih;
	unsigned			uSize;
	unsigned char		bTmp;
	unsigned char		*pBGR;

	pBmp = (TGAImage *)malloc(sizeof(TGAImage));
	if (pBmp == NULL) {
		return NULL;	// メモリ確保エラー
	}
	fp = fopen(pszFName, "rb");
	if (fp == NULL) {
		free(pBmp);
		return NULL;	// ファイルが見つからない
	}
	if (fread(&bmfh, sizeof(bmfh), 1, fp) != 1) {
		fclose(fp);
		free(pBmp);
		return NULL;	// ファイルが読み込めない
	}
	if (bmfh.bfType != 'MB') {
		fclose(fp);
		free(pBmp);
		return NULL;	// ビットマップ形式でない
	}
	if (fread(&bmih, sizeof(bmih), 1, fp) != 1) {
		fclose(fp);
		free(pBmp);
		return NULL;	// ファイルが読み込めない
	}
	if (bmih.biBitCount != 24 && bmih.biBitCount != 32) {
		fclose(fp);
		free(pBmp);
		return NULL;	// 対応形式でない
	}
	if (bmih.biCompression != BI_RGB) {
		fclose(fp);
		free(pBmp);
		return NULL;	// 対応形式でない
	}
	uSize = bmfh.bfSize - bmfh.bfOffBits;
	pBmp->data = (unsigned char *)malloc(uSize);
	if (pBmp->data == NULL) {
		fclose(fp);
		free(pBmp);
		return NULL;	// メモリ確保エラー
	}
	fseek(fp, bmfh.bfOffBits, SEEK_SET);
	if (fread(pBmp->data, uSize, 1, fp) != 1) {
		fclose(fp);
		free(pBmp->data);
		free(pBmp);
		return NULL;	// ファイルが読み込めない
	}
	pBmp->alphaDepth = 0;
	pBmp->pixelDepth = bmih.biBitCount;
	pBmp->sizeX = bmih.biWidth;
	pBmp->sizeY = bmih.biHeight;
	pBmp->type = bmih.biBitCount / 8;
	// BGRをRGBに並び換え
	for (pBGR = pBmp->data; pBGR + pBmp->type - 1 < pBmp->data + uSize; pBGR += pBmp->type) {
		bTmp = pBGR[0];
		pBGR[0] = pBGR[2];
		pBGR[2] = bTmp;
	}
	fclose(fp);
	return pBmp;
}

//---------------------------------------------------------------------------------------
// テクスチャロード
//---------------------------------------------------------------------------------------
GLuint LoadTexture(HWND hWnd, const char *fname)
{
	TCHAR		szMsg[1024];
	GLuint		uTexID;
	TGAImage	*pTextureImage;
	char		szExt[_MAX_EXT];
	bool		bIsTGA;

	_splitpath(fname, NULL, NULL, NULL, szExt);
	bIsTGA = _stricmp(szExt, ".tga") == 0;

	if (bIsTGA)
		pTextureImage = LoadTGA(fname);
	else
		pTextureImage = LoadBMP(fname);

	if (pTextureImage == NULL) {
		_stprintf(szMsg, _T("ERROR(%hs):テクスチャの読み込みに失敗しました。"), fname);
		MessageBox(hWnd, szMsg, _T("LoadTexture"), MB_OK | MB_ICONSTOP);
		return 0;
	}

	if (bIsTGA) {
		if (pTextureImage->type != 3 && pTextureImage->type != 4) {
			if (pTextureImage->data)
				free(pTextureImage->data);
			free(pTextureImage);
			_stprintf(szMsg, _T("ERROR(%hs):サポートしていない形式です。"), fname);
			MessageBox(hWnd, szMsg, _T("LoadTexture"), MB_OK | MB_ICONSTOP);
			return 0;
		}
	}

	glGenTextures(1, &uTexID);
	glBindTexture(GL_TEXTURE_2D, uTexID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	switch (pTextureImage->type) {
	case 3:
		glTexImage2D(GL_TEXTURE_2D, 0, 3, pTextureImage->sizeX, pTextureImage->sizeY,
			0, GL_RGB, GL_UNSIGNED_BYTE, pTextureImage->data);
		break;
	case 4:
		glTexImage2D(GL_TEXTURE_2D, 0, 4, pTextureImage->sizeX, pTextureImage->sizeY,
			0, GL_RGBA, GL_UNSIGNED_BYTE, pTextureImage->data);
		break;
	default:
		glDeleteTextures(1, &uTexID);
		uTexID = 0;
		break;
	}

	if (pTextureImage->data)
		free(pTextureImage->data);
	free(pTextureImage);

	return uTexID;
}

//---------------------------------------------------------------------------------------
// テクスチャ解放
//---------------------------------------------------------------------------------------
void ReleaseTexture(GLuint* puTextureID)
{
	if (puTextureID && *puTextureID) {
		glDeleteTextures(1, puTextureID);	// テクスチャオブジェクトを解放
		*puTextureID = 0;
	}
}

//---------------------------------------------------------------------------------------
// 1文字描画
//---------------------------------------------------------------------------------------
void DrawChar(int x, int y, int c)
{
	VERTEX2 vtxQuad[4];

	x = x - SCREEN_WIDTH / 2;
	y = -(y - SCREEN_HEIGHT / 2);
	vtxQuad[0].tu = (float)(c & 15) / FONT_HEIGHT;
	vtxQuad[0].tv = (float)(15 - (c >> 4)) / FONT_HEIGHT;
	vtxQuad[0].x  = (float)x;
	vtxQuad[0].y  = (float)y - FONT_HEIGHT;
	vtxQuad[0].z  = 0.0f;
	vtxQuad[1].tu = (float)((c & 15) + 1.0f) / FONT_HEIGHT;
	vtxQuad[1].tv = (float)(15 - (c >> 4)) / FONT_HEIGHT;
	vtxQuad[1].x  = (float)x + FONT_HEIGHT;
	vtxQuad[1].y  = (float)y - FONT_HEIGHT;
	vtxQuad[1].z  = 0.0f;
	vtxQuad[2].tu = (float)((c & 15) + 1.0f) / FONT_HEIGHT;
	vtxQuad[2].tv = (float)(15 - (c >> 4) + 1.0f) / FONT_HEIGHT;
	vtxQuad[2].x  = (float)x + FONT_HEIGHT;
	vtxQuad[2].y  = (float)y;
	vtxQuad[2].z  = 0.0f;
	vtxQuad[3].tu = (float)(c & 15) / FONT_HEIGHT;
	vtxQuad[3].tv = (float)(15 - (c >> 4) + 1.0f) / FONT_HEIGHT;
	vtxQuad[3].x  = (float)x;
	vtxQuad[3].y  = (float)y;
	vtxQuad[3].z  = 0.0f;

	// ビューマトリックス設定
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBindTexture(GL_TEXTURE_2D, g_uFontTexID);
	glInterleavedArrays(GL_T2F_V3F, 0, vtxQuad);
	glDrawArrays(GL_QUADS, 0, 4);
}

//---------------------------------------------------------------------------------------
// 文字列描画
//---------------------------------------------------------------------------------------
void DrawStr(int x, int y, LPCTSTR psz)
{
	for (; *psz; ++psz) {
		switch (*psz) {
		case _T('\n'):
			x = 0;
			y += FONT_HEIGHT;
			break;
		default:
			DrawChar(x, y, *psz);
			x += FONT_WIDTH;
			break;
		}
	}
}

//=================================================================================================
//	end of file
//=================================================================================================
