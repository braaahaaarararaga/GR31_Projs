//=================================================================================================
//    [ 頂点ブレンディング ] DirectX Graphics 3D sample
//    頂点ブレンディングサンプル
//
//    操作
//    　Ｗ　　　　ワイヤーフレーム描画スイッチ
//    　スペース　モーションスイッチ
//
//=================================================================================================
//-------- インクルードヘッダ
#define _CRT_SECURE_NO_WARNINGS
#define STRICT
#include <windows.h>		// Windowsプログラムにはこれを付ける
#include <tchar.h>			// 汎用テキスト マッピング
#include <d3d9.h>			// DirectX Graphics 関連のヘッダー
#include <d3dx9.h>			// DirectX Graphics 関連のヘッダー
#include <stdio.h>			// 文字列処理で使用
#include <mmsystem.h>		// 時間管理で使用

//-------- ライブラリのリンク（こう記述しておけば別途リンク設定が不要となる）
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")

//-------- マクロ
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x)	if(x){ x->Release(); x=NULL; }
#endif

//-------- 定数定義
#define CLASS_NAME		_T("VertexBlending")			// ウィンドウクラス名
#define CAPTION_NAME	_T("[ 頂点ブレンディング ]")	// キャプション名

const float FRAME_RATE		= 1000.0f/60.0f;			// フレームレート
const int	SCREEN_WIDTH	= 640;						// スクリーンの幅
const int	SCREEN_HEIGHT	= 480;						// スクリーンの高さ
const float	RAD				= D3DX_PI/180.0f;			// ラジアン

//----- 頂点フォーマット定義
struct VERTEX {
	float		x, y, z;			// 頂点座標（座標変換あり）
	float		blend;				// ブレンド値
	D3DCOLOR	color;				// ディフューズ色
};
#define FVF_VERTEX (D3DFVF_XYZB1 | D3DFVF_DIFFUSE)

typedef struct {
	int			status;		// ステータス
	D3DXVECTOR3	position;	// 座標
	D3DXVECTOR3	rotation;	// 回転角
	float		degree;		// 角度変化サインカーブ用角度
} PRISM;

//----- プリズム面モデル定義
//　（注意）法線ベクトル、テクスチャ座標は計算で求める
VERTEX prism_vertex0[] = {
	//      x        y        z  blend  color
	{ -400.0f, -100.0f,    0.0f,  0.0f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// 頂点０
	{ -400.0f,  100.0f,    0.0f,  0.0f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// 頂点０
	{ -300.0f, -100.0f,    0.0f,  0.03f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// 頂点０
	{ -300.0f,  100.0f,    0.0f,  0.03f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// 頂点０
	{ -200.0f, -100.0f,    0.0f,  0.12f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// 頂点０
	{ -200.0f,  100.0f,    0.0f,  0.12f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// 頂点０
	{ -100.0f, -100.0f,    0.0f,  0.3f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// 頂点０
	{ -100.0f,  100.0f,    0.0f,  0.3f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// 頂点０
	{    0.0f, -100.0f,    0.0f,  0.5f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// 頂点０
	{    0.0f,  100.0f,    0.0f,  0.5f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// 頂点０
	{  100.0f, -100.0f,    0.0f,  0.7f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// 頂点０
	{  100.0f,  100.0f,    0.0f,  0.7f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// 頂点０
	{  200.0f, -100.0f,    0.0f,  0.88f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// 頂点０
	{  200.0f,  100.0f,    0.0f,  0.88f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// 頂点０
	{  300.0f, -100.0f,    0.0f,  0.97f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// 頂点０
	{  300.0f,  100.0f,    0.0f,  0.97f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// 頂点０
	{  400.0f, -100.0f,    0.0f,  1.0f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// 頂点０
	{  400.0f,  100.0f,    0.0f,  1.0f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// 頂点０
};

VERTEX prism_vertex1[] = {
	//      x        y        z  blend  color
	{ -400.0f, -100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// 頂点０
	{ -400.0f,  100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// 頂点０
	{ -300.0f, -100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// 頂点０
	{ -300.0f,  100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// 頂点０
	{ -200.0f, -100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// 頂点０
	{ -200.0f,  100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// 頂点０
	{ -100.0f, -100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// 頂点０
	{ -100.0f,  100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// 頂点０
	{    0.0f, -100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// 頂点０
	{    0.0f,  100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// 頂点０
	{  100.0f, -100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// 頂点０
	{  100.0f,  100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// 頂点０
	{  200.0f, -100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// 頂点０
	{  200.0f,  100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// 頂点０
	{  300.0f, -100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// 頂点０
	{  300.0f,  100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// 頂点０
	{  400.0f, -100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// 頂点０
	{  400.0f,  100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// 頂点０
};

VERTEX prism_vertex2[] = {
	//      x        y        z  blend  color
	{ -400.0f, -100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// 頂点０
	{ -400.0f,  100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// 頂点０
	{ -300.0f, -100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// 頂点０
	{ -300.0f,  100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// 頂点０
	{ -200.0f, -100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// 頂点０
	{ -200.0f,  100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// 頂点０
	{ -100.0f, -100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// 頂点０
	{ -100.0f,  100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// 頂点０
	{    0.0f, -100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// 頂点０
	{    0.0f,  100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// 頂点０
	{  100.0f, -100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// 頂点０
	{  100.0f,  100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// 頂点０
	{  200.0f, -100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// 頂点０
	{  200.0f,  100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// 頂点０
	{  300.0f, -100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// 頂点０
	{  300.0f,  100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// 頂点０
	{  400.0f, -100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// 頂点０
	{  400.0f,  100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// 頂点０
};

//----- 視点情報
D3DXVECTOR3 view_pos(0, 0, -800);		// 視点位置
D3DXVECTOR3 view_ref(0, 0,    0);		// 注視位置
D3DXVECTOR3 view_up (0, 1,    0);		// アップベクトル

//----- グローバル変数

// DirectX 関連
LPDIRECT3D9				g_pD3D;				// Direct3D9 オブジェクト
LPDIRECT3DDEVICE9		g_pD3DDevice;		// Direct3DDevice9 オブジェクト
D3DPRESENT_PARAMETERS	g_D3Dpp;			// プレゼンテーションパラメータ
LPD3DXFONT				g_pD3DXFont;		// D3DXFont オブジェクト
D3DXMATRIX				g_MatView;			// ビューマトリックス
D3DXMATRIX				g_MatProj;			// プロジェクションマトリックス

PRISM					g_Prism;			// プリズムオブジェクト

TCHAR					g_szDebug[4096];	// 出力文字列バッファ
float					g_FPS;				// フレーム数カウント用
bool					g_Wireframe;		// ワイヤーフレーム描画スイッチ
bool					g_OnMove;			// モーションスイッチ

//----- プロトタイプ宣言
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
long Initialize3DEnvironment(HWND hWnd, bool bWindow);
HRESULT Cleanup3DEnvironment(HWND hWnd);
HRESULT Render3DEnvironment();
void ExecBG();
void ExecPrism();
void DrawInfo();

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
	g_Prism.status = 0;
	g_Wireframe = false;
	g_OnMove = true;

	// メインウインドウループ
    msg.message = WM_NULL;
	while (WM_QUIT != msg.message) {								// WM_QUIT がくるまでループ
		if (PeekMessage(&msg,NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			dwCurrentTime = timeGetTime();								// 現在のタイマー値を取得
			if (dwCurrentTime - dwFPSLastTime >= 500) {					// ０．５秒ごとに計測
				dt = (float)(dwCurrentTime - dwFPSLastTime) / 1000.0f;	// フレーム数を計算
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
	Cleanup3DEnvironment(hWnd);
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
		case VK_ESCAPE:	DestroyWindow(hWnd);	return 0;
		case 'W':		g_Wireframe = !g_Wireframe;	return 0;	// ワイヤーフレームの切り替え
		case VK_SPACE:	g_OnMove = !g_OnMove;	return 0;		// モーションの切り替え
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
	// デバイス情報テーブル
	struct TCreateDevice {
		D3DDEVTYPE type;			// デバイスの種類
		DWORD      behavior;		// デバイスの動作
	};
	const int c_nMaxDevice = 3;
	const TCreateDevice device[c_nMaxDevice] = {
	    { D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING },	// T&L HAL
	    { D3DDEVTYPE_HAL, D3DCREATE_SOFTWARE_VERTEXPROCESSING },	// HAL
	    { D3DDEVTYPE_REF, D3DCREATE_SOFTWARE_VERTEXPROCESSING },	// REF
	};
	int			nDev;				// デバイス種別カウンタ

	// Direct3Dオブジェクトを生成
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION))) {
		MessageBox(hWnd, _T("Direct3Dオブジェクトの作成に失敗しました"), _T("error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// Direct3D 初期化パラメータの設定
	ZeroMemory(&g_D3Dpp, sizeof(g_D3Dpp));
	if (bWindow) {
		// ウィンドウモード
		g_D3Dpp.Windowed			= TRUE;
		g_D3Dpp.BackBufferFormat	= D3DFMT_UNKNOWN;			// バックバッファフォーマット
	} else {
		// フルスクリーンモード
		ShowCursor(FALSE);										// マウスカーソルを表示しない
		g_D3Dpp.Windowed			= FALSE;
		g_D3Dpp.BackBufferFormat	= D3DFMT_X8R8G8B8;			// バックバッファフォーマット
	}
	g_D3Dpp.BackBufferWidth				= SCREEN_WIDTH;						// ゲーム画面サイズ
	g_D3Dpp.BackBufferHeight			= SCREEN_HEIGHT;
	g_D3Dpp.SwapEffect					= D3DSWAPEFFECT_DISCARD;			// バッファスワップ動作
	g_D3Dpp.EnableAutoDepthStencil		= TRUE;								// Ｚバッファ有効
	g_D3Dpp.AutoDepthStencilFormat		= D3DFMT_D16;						// Ｚバッファフォーマット
	g_D3Dpp.FullScreen_RefreshRateInHz	= D3DPRESENT_RATE_DEFAULT;
	g_D3Dpp.PresentationInterval		= D3DPRESENT_INTERVAL_IMMEDIATE;	// VSYNCを待たない

	// デバイスオブジェクトを作成
	for (nDev = 0; nDev < c_nMaxDevice; nDev++) {
		if (SUCCEEDED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, device[nDev].type, hWnd,	// デバイスを作成
					device[nDev].behavior, &g_D3Dpp, &g_pD3DDevice))) break;			// 作成できたら break
	}
	if (nDev >= c_nMaxDevice) {		// ループを回りきってしまったらデバイス作成は出来なかったと判断
		MessageBox(hWnd, _T("デバイスの作成に失敗しました"), _T("error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// フォントセットアップ
	D3DXCreateFont(g_pD3DDevice, 18, 0, 0, 0, FALSE, SHIFTJIS_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Terminal"), &g_pD3DXFont);

	// ビューマトリックス設定
	D3DXMatrixLookAtLH(&g_MatView, &view_pos, &view_ref, &view_up);
	g_pD3DDevice->SetTransform(D3DTS_VIEW, &g_MatView);		// ビューマトリックスをセット

	// プロジェクションマトリックス設定
	D3DXMatrixPerspectiveFovLH(&g_MatProj, 60.0f * D3DX_PI/180.0f, (float)SCREEN_WIDTH/SCREEN_HEIGHT, 100.0f, 10000.0f);
	g_pD3DDevice->SetTransform(D3DTS_PROJECTION, &g_MatProj);	// プロジェクションマトリックスをセット

	// レンダリングステートを設定
	g_pD3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE);					// Ｚバッファ無効化
	g_pD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);				// ライト無効化
	g_pD3DDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS);	// ウェイトを設定

	return S_OK;
}

//-------------------------------------------------------------------------------------------------
// ３Ｄ環境のクリーンアップ
//-------------------------------------------------------------------------------------------------
HRESULT Cleanup3DEnvironment(HWND hWnd)
{
	SAFE_RELEASE(g_pD3DXFont);		// D3DXFont解放
	SAFE_RELEASE(g_pD3DDevice);		// Direct3DDevice オブジェクト解放
	SAFE_RELEASE(g_pD3D);			// Direct3D オブジェクト解放
	return S_OK;
}

//-------------------------------------------------------------------------------------------------
// ３Ｄ環境のレンダリング
//-------------------------------------------------------------------------------------------------
HRESULT Render3DEnvironment()
{
	g_szDebug[0] = _T('\0');		// デバッグストリングバッファ初期化

	// バックバッファとＺバッファをクリア
	g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0x00, 0x60, 0x80), 1.0f, 0);

	if (SUCCEEDED(g_pD3DDevice->BeginScene())) {	// シーン開始
		ExecPrism();					// プリズムモデル実行、描画
		DrawInfo();						// インフォメーション表示
		g_pD3DDevice->EndScene();		// 描画終了
	}

	// バックバッファをプライマリバッファにコピー
	if (FAILED(g_pD3DDevice->Present(NULL,NULL,NULL,NULL))) g_pD3DDevice->Reset(&g_D3Dpp);

	return S_OK;
}

//-------------------------------------------------------------------------------------------------
// プリズムオブジェクト
//-------------------------------------------------------------------------------------------------
void ExecPrism()
{
	D3DXMATRIX	matRotateX;				// Ｘ軸回転マトリックス
	D3DXMATRIX	matRotateY;				// Ｙ軸回転マトリックス
	D3DXMATRIX	matRotateZ;				// Ｚ軸回転マトリックス
	D3DXMATRIX	matTrans;				// 平行移動マトリックス
	D3DXMATRIX	matWorld;				// ワールド変換マトリックス
	D3DXMATRIX	matWorld2;				// ワールド変換マトリックス

	switch (g_Prism.status) {
	case 0:
		// モデル角度、位置の初期化
		g_Prism.rotation.x = g_Prism.rotation.y = g_Prism.rotation.z = 0.0f;
		g_Prism.position.x = g_Prism.position.y = g_Prism.position.z = 0.0f;
		g_Prism.degree = 0.0f;
		g_Prism.status = 1;
		// THRU
	case 1:
		// 自動的に回転
		if (g_OnMove) {
			g_Prism.rotation.z = (float)sin(g_Prism.degree);
			g_Prism.degree += 0.01f;
		}
		break;
	default:
		break;
	}
	// ワールドマトリックスを合成
	D3DXMatrixRotationX(&matRotateX, g_Prism.rotation.x);	// Ｘ軸回転マトリックスを生成
	D3DXMatrixRotationY(&matRotateY, g_Prism.rotation.y);	// Ｙ軸回転マトリックスを生成
	D3DXMatrixRotationZ(&matRotateZ, g_Prism.rotation.z);	// Ｚ軸回転マトリックスを生成
	D3DXMatrixTranslation(&matTrans,
		g_Prism.position.x, g_Prism.position.y, g_Prism.position.z);	// 平行移動マトリックスを生成
	matWorld = matRotateX * matRotateY * matRotateZ * matTrans;			// ワールドマトリックスを合成

	if (g_Wireframe) {
		g_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);	// ワイヤーフレーム描画
	} else {
		g_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);		// ソリッド描画
	}

	D3DXMatrixIdentity(&matWorld2);
	g_pD3DDevice->SetFVF(FVF_VERTEX);									// 頂点フォーマット設定
	g_pD3DDevice->SetTransform(D3DTS_WORLDMATRIX(0), &matWorld);		// ワールドマトリックスをセット
	g_pD3DDevice->SetTransform(D3DTS_WORLDMATRIX(1), &matWorld2);		// ワールドマトリックスをセット

	// マトリックス１で描画
	g_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 16, &prism_vertex1[0], sizeof(VERTEX));

	// ブレンディングモデルを描画
	g_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 16, &prism_vertex2[0], sizeof(VERTEX));

	// マトリックス０で描画
	g_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 16, &prism_vertex0[0], sizeof(VERTEX));

}

//-------------------------------------------------------------------------------------------------
// インフォメーション表示
//-------------------------------------------------------------------------------------------------
void DrawInfo()
{
//	int			i;
	TCHAR		str[256];
	static RECT	rcStr = {0, 0, 640, 480};

	// デバッグストリング描画
	_stprintf(str, _T("FPS : %4.1f\n"), g_FPS);
	lstrcat(g_szDebug, str);
//	for (i = 0; i < 4; i++) {
//		_stprintf(str, _T("%2d: tu = %4.2f, tv = %4.2f\n"), i, prism_vertex[i].tu, prism_vertex[i].tv);
//		lstrcat(g_szDebug, str);
//	}
	g_pD3DXFont->DrawText(NULL, g_szDebug, -1, &rcStr, DT_LEFT, D3DCOLOR_ARGB(0xff, 0xff, 0xff, 0xff));
}

//=================================================================================================
//	end of file
//=================================================================================================
