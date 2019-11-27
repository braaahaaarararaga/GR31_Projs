//=======================================================================================
//
//	  [ モーフィング ] : DirectX Sample Program
//		Ｘファイルを２つ読み込んでモーフィングで変形
//
//　　操作方法
//　　［１］：モデル０を描画
//　　［２］：モデル１を描画
//　　［３］：モーフィング中モデルを描画
//
//=======================================================================================
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
#define SAFE_RELEASE(x)	if(x){x->Release();x=NULL;}
#endif
#ifndef SAFE_DELETE
#define SAFE_DELETE(x)	if(x){delete x;x=NULL;}
#endif

//-------- 定数定義
#define CLASS_NAME		_T("Morphing")								// ウィンドウクラス名
#define CAPTION_NAME	_T("[ モーフィング ] (制作途中の例)")		// キャプション名

const float FRAME_RATE		= 1000.0f/60.0f;						// フレームレート
const int	SCREEN_WIDTH	= 640;									// スクリーンの幅
const int	SCREEN_HEIGHT	= 480;									// スクリーンの高さ
const float	ASPECT_RATIO	= (float)SCREEN_WIDTH/SCREEN_HEIGHT;	// アスペクト比
const int	FONT_SIZE		= 14;									// デバッグ文字フォントサイズ
LPCTSTR		FONT_TYPE		= _T("ＭＳ Ｐゴシック");				// デバッグ文字フォント種別
const float	NEAR_CLIP		= 100.0f;								// ニアクリップを行う距離
const float	FAR_CLIP		= 30000.0f;								// ファークリップを行う距離
const float	RAD				= D3DX_PI/180.0f;						// ラジアン
const float ROTATE_SPEED	= 0.04f;								// 回転速度

#define MODEL_MAX	2

//----- 頂点フォーマット定義
struct VERTEX {
	float		x, y, z;			// 頂点座標（座標変換あり）
	float		nx, ny, nz;			// 法線ベクトル
	float		tu, tv;				// テクスチャ座標
};
#define FVF_VERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)

//----- 構造体定義
struct OBJECT {			//--- オブジェクト用構造体
	int			status;				// ステータス
	float		x, y, z;			// 座標
	float		rotx, roty, rotz;	// 角度
	D3DXMATRIX	matrix;				// 変換マトリックス
	int			morph_count;		// モーフィング計算用時間カウント値
	int			morph_dir;			// モーフィングの方向
	int			morph_time[2];		// モーフィングに要する時間
};

//=======================================================================================
// グローバル変数定義
//=======================================================================================
LPDIRECT3D9					g_pD3D;					// Direct3D8 オブジェクト
LPDIRECT3DDEVICE9			g_pD3DDevice;			// Direct3DDevice8 オブジェクト
D3DPRESENT_PARAMETERS		g_D3DPresentParam;		// PRESENT PARAMETERS
LPD3DXFONT					g_pD3DXFont;			// D3DXFont オブジェクト
float						g_FPS;					// フレーム数カウント用
LPDIRECT3DTEXTURE9			g_pD3DTexture;			// Direct3DTexture8 オブジェクト
OBJECT						g_Object;				// オブジェクト情報

VERTEX						*pt_vertex[MODEL_MAX];	// 頂点データへのポインタ
WORD						*pt_index[MODEL_MAX];	// インデックスデータへのポインタ
int							num_vertex[MODEL_MAX];	// 頂点数
int							num_face[MODEL_MAX];	// 面数
VERTEX						*morph_vertex;			// モーフィング用頂点ワークへのポインタ
WORD						*morph_index;			// モーフィング用インデックスワークへのポインタ

int							g_DrawMode;				// 描画モデル選択
bool						g_Wireframe;			// ワイヤーフレーム描画スイッチ

//=======================================================================================
// プロトタイプ宣言
//=======================================================================================
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT InitializeGraphics(HWND hWnd, bool bWindow);
HRESULT CleanupGraphics();
HRESULT RenderGraphics();
int SetModelData(HWND hWnd);
void ExecPat();
void DrawPat();

//---------------------------------------------------------------------------------------
//	メイン
//---------------------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int iCmdShow)
{
	HWND	hWnd;
	MSG		msg;
	DWORD	dwExecLastTime, dwFPSLastTime, dwCurrentTime, dwFrameCount;
	float	dt;
	bool	bWindow;

	// ウィンドウクラスの登録
	WNDCLASSEX wc = {
		sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WindowProc,
		0, 0, hInstance, LoadIcon(hInstance, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)GetStockObject(WHITE_BRUSH), NULL, CLASS_NAME, NULL};
	if (RegisterClassEx(&wc) == 0) return false;	// ウィンドウクラスを登録

	// ウィンドウを作成
	hWnd = CreateWindow(
		CLASS_NAME, CAPTION_NAME, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION, 100, 100,
		SCREEN_WIDTH  + GetSystemMetrics(SM_CXFIXEDFRAME) * 2,
		SCREEN_HEIGHT + GetSystemMetrics(SM_CYFIXEDFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION),
		NULL, NULL, hInstance, NULL);
	if (hWnd == NULL) return false;
	ShowWindow(hWnd, iCmdShow);		// ウィンドウを表示
	UpdateWindow(hWnd);				// ウィンドウをアップデート

	// グラフィック環境の初期化
	bWindow = false;
	if (IDYES == MessageBox(hWnd, _T("ウィンドウモードで実行しますか？"), _T("画面モード"), MB_YESNO))
		bWindow = true;
	if (FAILED(InitializeGraphics(hWnd, bWindow))) return false;	// ここで初期化

	// 各種初期化
	srand(timeGetTime());
	dwFPSLastTime = dwExecLastTime = timeGetTime();		// 現在のシステムタイマーを取得
	dwFrameCount = 0;
	SetModelData(hWnd);					// モデルデータの準備
	g_Object.status = 0;				// ステータスを０（＝初期化）に設定
	g_DrawMode  = 0;
	g_Wireframe = false;
	// メッセージループ
	timeBeginPeriod(1);					// システムタイマーの分解能を１ｍｓに設定
    msg.message = WM_NULL;
	while (WM_QUIT != msg.message) {						// WM_QUIT がくるまでループ
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {		// メッセージをチェック
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			// この辺でフレーム数カウント
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
				ExecPat();												// パターン移動実行
				RenderGraphics();										// レンダリング
				dwFrameCount++;											// フレームカウントを＋１
			}
		}
		Sleep(1);
	}
	CleanupGraphics();		// グラフィック環境のクリーンアップ
	timeEndPeriod(1);		// システムタイマーの分解能を元に戻す
	return msg.wParam;
}

//---------------------------------------------------------------------------------------
//	ウィンドウプロシージャ
//---------------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_DESTROY:				//----- 終了指示がきた
		PostQuitMessage(0);				// システムにスレッドの終了を要求
		return 0;
	case WM_KEYDOWN:				//----- キーボードが押された
		switch (wParam) {
		case '1':		g_DrawMode = 0;				break;		// １つ目のモデルを描画
		case '2':		g_DrawMode = 1;				break;		// ２つ目のモデルを描画
		case '3':		g_DrawMode = 2;				break;		// モーフィングモデルを描画
		case 'W':		g_Wireframe = !g_Wireframe;	break;		// ワイヤーフレームの切り替え
		case VK_ESCAPE:	DestroyWindow(hWnd);		break;		// [ESC]キーが押された
		}
		return 0;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//=======================================================================================
//    DirectX Graphics 関連
//=======================================================================================

//---------------------------------------------------------------------------------------
// グラフィック環境の初期化
//---------------------------------------------------------------------------------------
HRESULT InitializeGraphics(HWND hWnd, bool bWindow)
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
	D3DDISPLAYMODE	disp;				// ディスプレイモードを記述
	int				nDev;				// デバイス種別カウンタ
	D3DXMATRIX		matView, matProj;	// ビューマトリックス、プロジェクションマトリックス
	D3DMATERIAL9	mtrl;				// マテリアルオブジェクト
	D3DXVECTOR3		vecDir;				// 方向を指定するベクトル
	D3DLIGHT9		light;				// ライトオブジェクト

	// Direct3Dオブジェクトを生成
	if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL) {
		MessageBox(hWnd, _T("Direct3Dオブジェクトの作成に失敗しました"), _T("error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}
	// 現在の画面モードを取得
	if (FAILED(g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &disp))) {
		MessageBox(hWnd, _T("ディスプレイモードの取得に失敗しました"), _T("error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}
	// Direct3D 初期化パラメータの設定
	ZeroMemory(&g_D3DPresentParam, sizeof(D3DPRESENT_PARAMETERS));
	g_D3DPresentParam.BackBufferWidth        = SCREEN_WIDTH;			// ゲーム画面サイズ
	g_D3DPresentParam.BackBufferHeight       = SCREEN_HEIGHT;
	g_D3DPresentParam.Windowed               = bWindow;					// ウィンドウモードか?
	g_D3DPresentParam.BackBufferFormat       = (bWindow) ? disp.Format : D3DFMT_R5G6B5;	// バックバッファ
	g_D3DPresentParam.SwapEffect             = D3DSWAPEFFECT_DISCARD;
	g_D3DPresentParam.EnableAutoDepthStencil = true;					// Ｚバッファ有効
	g_D3DPresentParam.AutoDepthStencilFormat = D3DFMT_D16;				// Ｚバッファフォーマット

	// デバイスオブジェクトを作成
	for (nDev = 0; nDev < c_nMaxDevice; nDev++) {
		if (SUCCEEDED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, device[nDev].type, hWnd,	// デバイスを作成
					device[nDev].behavior, &g_D3DPresentParam, &g_pD3DDevice))) break;
	}
	if (nDev >= c_nMaxDevice) {
		MessageBox(hWnd, _T("デバイスの作成に失敗しました"), _T("error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// 情報表示用フォントを設定
	D3DXCreateFont(g_pD3DDevice, FONT_SIZE, 0, 0, 0, FALSE, SHIFTJIS_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, FONT_TYPE, &g_pD3DXFont);

	// レンダリングステートを設定
	g_pD3DDevice->SetRenderState(D3DRS_ZENABLE, TRUE);			// Ｚバッファ有効化
	g_pD3DDevice->SetRenderState(D3DRS_AMBIENT, 0x00303030);	// 環境光の設定

	//-------- ここから３Ｄ関連
	// ビューマトリックス設定
	D3DXMatrixLookAtLH(&matView,
						&D3DXVECTOR3(  0.0,  0.0,-1000.0),		// 視点座標
						&D3DXVECTOR3(  0.0,  0.0,    0.0),		// 注視点座標
						&D3DXVECTOR3(  0.0,  1.0,    0.0));		// アップベクトル
	g_pD3DDevice->SetTransform(D3DTS_VIEW, &matView);

	// プロジェクションマトリックス設定
	D3DXMatrixPerspectiveFovLH(&matProj, 30 * RAD, ASPECT_RATIO, NEAR_CLIP, FAR_CLIP);
	g_pD3DDevice->SetTransform(D3DTS_PROJECTION, &matProj);

	// マテリアルの設定
	ZeroMemory(&mtrl, sizeof(mtrl));			// いったんゼロでクリア
	mtrl.Diffuse.r = mtrl.Diffuse.g = mtrl.Diffuse.b = mtrl.Diffuse.a = 1.0;
	mtrl.Ambient.r = mtrl.Ambient.g = mtrl.Ambient.b = mtrl.Ambient.a = 1.0;
	g_pD3DDevice->SetMaterial(&mtrl);

	// ライトの設定
	ZeroMemory(&light, sizeof(light));
	light.Type = D3DLIGHT_DIRECTIONAL;			// 平行光源
	light.Diffuse.r = light.Diffuse.g = light.Diffuse.b = light.Diffuse.a = 1.0;
	vecDir = D3DXVECTOR3(1.0, -1.0, 1.0);							// 光の向きを決める
	D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, &vecDir);		// 正規化したベクトルを返す
	g_pD3DDevice->SetLight(0, &light);
	g_pD3DDevice->LightEnable(0, TRUE);			// ライト０を有効

	return S_OK;
}

//---------------------------------------------------------------------------------------
// グラフィック環境のクリーンアップ
//---------------------------------------------------------------------------------------
HRESULT CleanupGraphics()
{
	int		i;

	for (i = 0; i < MODEL_MAX; i++) {
		SAFE_DELETE(pt_vertex[i]);	// 頂点データの領域を解放
		SAFE_DELETE(pt_index[i]);	// インデックスデータの領域を解放
	}
	SAFE_DELETE(morph_vertex);		// モーフィング用頂点データの領域を解放
	SAFE_DELETE(morph_index);		// モーフィング用インデックスデータの領域を解放

	SAFE_RELEASE(g_pD3DXFont);		// D3DXFont オブジェクトを解放
	SAFE_RELEASE(g_pD3DTexture);	// テクスチャオブジェクトを解放
	SAFE_RELEASE(g_pD3DDevice);		// Direct3DDevice オブジェクトを解放
	SAFE_RELEASE(g_pD3D);			// Direct3D オブジェクトを解放
	return S_OK;
}

//---------------------------------------------------------------------------------------
// グラフィック環境のレンダリング
//---------------------------------------------------------------------------------------
HRESULT RenderGraphics()
{
	RECT	rcStr;
	TCHAR	str[1024], tmp[256];
	
	// ビューポートをクリア
	g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0x00, 0x40, 0x80), 1.0, 0);

	if (SUCCEEDED(g_pD3DDevice->BeginScene())) {		// シーン開始

		DrawPat();		// パターンを描画

		// デバッグストリング描画
		str[0] = _T('\0');
		_stprintf(tmp, _T("FPS = %6.2f\n\n"), g_FPS);	lstrcat(str, tmp);			// ＦＰＳ
		_stprintf(tmp, _T("DrawMode = %d\n\n"), g_DrawMode);	lstrcat(str, tmp);	// 描画モード
		_stprintf(tmp, _T("Time = %d\n\n"), g_Object.morph_count);	lstrcat(str, tmp);	// 時間
		_stprintf(tmp, _T("Rotate X(%6.2f), Y(%6.2f), Z(%6.2f)\n"),
						g_Object.rotx, g_Object.roty, g_Object.rotz);	lstrcat(str, tmp);	// 角度
		SetRect(&rcStr, 0, 0, 640, 480);
		g_pD3DXFont->DrawText(NULL, str, -1, &rcStr, 0, D3DCOLOR_ARGB(0xff, 0xff, 0xff, 0xff));

		g_pD3DDevice->EndScene();					// シーン終了
	}

	// バックバッファに描画した内容を表示
	if (FAILED(g_pD3DDevice->Present(NULL, NULL, NULL, NULL))) {
		g_pD3DDevice->Reset(&g_D3DPresentParam);
	}
	return S_OK;
}

//---------------------------------------------------------------------------------------
// モデルデータセットアップ
//---------------------------------------------------------------------------------------
int SetModelData(HWND hWnd)
{
	LPD3DXMESH				mesh;			// メッシュオブジェクト
	LPDIRECT3DVERTEXBUFFER9	vertex_buffer;	// 頂点バッファ
	LPDIRECT3DINDEXBUFFER9	index_buffer;	// インデックスバッファ

	DWORD			dwNumMaterials = 0L;
    LPD3DXBUFFER	pD3DXMtrlBuffer, pD3DXEffectInst;
	VERTEX*		pVertices;		// 頂点データへのポインタ
	WORD*		pIndices;		// ポリゴン構成（頂点リンク）データへのポインタ
	VERTEX*		pVertices_2;
	WORD*		pIndices_2;

	static LPCTSTR filename[] = {
		_T("ishi.x"), _T("ishi2.x")		// モデル（Ｘファイル）のファイル名（２個）
	};
	int			i;

	for (i = 0; i < MODEL_MAX; i++) {
	    // Ｘファイルからメッシュデータを読み込む
		D3DXLoadMeshFromX(filename[i], D3DXMESH_SYSTEMMEM, g_pD3DDevice, NULL,	// ここでＸファイルを指定
							&pD3DXMtrlBuffer, &pD3DXEffectInst, &dwNumMaterials, &mesh);
		mesh->GetVertexBuffer(&vertex_buffer);	// 頂点バッファオブジェクトへのポインタをゲット
		mesh->GetIndexBuffer(&index_buffer);	// インデックスバッファオブジェクトへのポインタをゲット
		num_vertex[i] = mesh->GetNumVertices();	// 頂点数をゲット
		num_face[i]  = mesh->GetNumFaces();		// 面数をゲット

		// 頂点データ、インデックスデータをメモリにコピー
		pt_vertex[i] = new VERTEX[num_vertex[i]];						// 頂点ワーク領域を確保
		pt_index[i]  = new WORD[num_face[i]*3];							// インデックスワーク領域を確保
		
		pVertices_2 = pt_vertex[i];
		vertex_buffer->Lock(0, 0, (void**)&pVertices, 0);				// 頂点バッファをロック
		memcpy(pVertices_2, pVertices, sizeof(VERTEX)*num_vertex[i]);	// 頂点データをワークにコピー
		vertex_buffer->Unlock();										// 頂点バッファをアンロック

		pIndices_2 = pt_index[i];
		index_buffer ->Lock(0, 0, (void**)&pIndices , 0);				// インデックスバッファをロック
		memcpy(pIndices_2, pIndices, sizeof(WORD)*num_face[i]*3);		// インデックスデータをワークにコピー
		index_buffer ->Unlock();										// インデックスバッファをアンロック
	}

	// モーフィング用に頂点バッファ、インデックスバッファを作成
	morph_vertex = new VERTEX[num_vertex[0]];						// モーフィング用頂点ワーク領域を確保
	morph_index  = new WORD[num_face[0]*3];							// モーフィング用インデックスワーク領域を確保

	vertex_buffer->Lock(0, 0, (void**)&pVertices, 0);				// 頂点バッファをロック
	memcpy(morph_vertex, pVertices, sizeof(VERTEX)*num_vertex[0]);	// 頂点データをワークにコピー
	vertex_buffer->Unlock();										// 頂点バッファをアンロック
	index_buffer ->Lock(0, 0, (void**)&pIndices , 0);				// インデックスバッファをロック
	memcpy(morph_index, pIndices, sizeof(WORD)*num_face[0]*3);		// インデックスデータをワークにコピー
	index_buffer ->Unlock();										// インデックスバッファをアンロック

	// テクスチャファイルを読み込む
	D3DXCreateTextureFromFile(g_pD3DDevice, _T("ishi.bmp"), &g_pD3DTexture);
	return 0;
}

//---------------------------------------------------------------------------------------
//　パターン処理
//---------------------------------------------------------------------------------------
void ExecPat()
{
	D3DXMATRIX	matRotateX, matRotateY, matRotateZ;	// 回転マトリックス（３種）
	D3DXMATRIX	matTrans;							// 平行移動マトリックス
	OBJECT*		pt;									// オブジェクトへのポインタ
	float		ratio;								// 時間から求めた変形の割合
	VERTEX		*spt0, *spt1, *dpt;					// 各頂点データへのポインタ
	int			i;

	//----- 処理本体
	pt = &g_Object;
	switch (pt->status) {
	case 0:		// Initialize
		// 位置の初期化
		pt->x  = 0.0;		pt->y  = 0.0;		pt->z  = 0.0;
		// 回転角の初期化
		pt->rotx = 0.0;		pt->roty = 0.0;		pt->rotz = 0.0;
		// モーフィング用変数の初期化
		pt->morph_dir = 0;
		pt->morph_count = 0;
		pt->morph_time[0] = 0;
		pt->morph_time[1] = 30;	// ここで変形時間が決まる
		pt->status = 1;
		// THRU
	case 1:		// Move
		// キー入力による回転
		if (GetAsyncKeyState(VK_UP   ) & 0x8000) pt->rotx += ROTATE_SPEED;	// カーソル（上）
		if (GetAsyncKeyState(VK_DOWN ) & 0x8000) pt->rotx -= ROTATE_SPEED;	// カーソル（下）
		if (GetAsyncKeyState(VK_LEFT ) & 0x8000) pt->roty -= ROTATE_SPEED;	// カーソル（左）
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000) pt->roty += ROTATE_SPEED;	// カーソル（右）
		if (GetAsyncKeyState(VK_PRIOR) & 0x8000) pt->rotz += ROTATE_SPEED;	// Page Up
		if (GetAsyncKeyState(VK_NEXT ) & 0x8000) pt->rotz -= ROTATE_SPEED;	// Page Down
		// マトリックスを合成
		D3DXMatrixRotationX(&matRotateX, pt->rotx);							// Ｘ軸回転マトリックスを生成
		D3DXMatrixRotationY(&matRotateY, pt->roty);							// Ｙ軸回転マトリックスを生成
		D3DXMatrixRotationZ(&matRotateZ, pt->rotz);							// Ｚ軸回転マトリックスを生成
		D3DXMatrixTranslation(&matTrans, pt->x, pt->y, pt->z);				// 平行移動マトリックスを生成
		pt->matrix = matRotateX * matRotateY * matRotateZ * matTrans;		// マトリックスを合成（順序に注意！）

		//**************************************************************************
		//
		//　ここにモーフィングの計算処理を追加する。
		//
		//　（１）変形前モデルの頂点へのポインタ（pt_vertex[0]）
		//　　　　変形後モデルの頂点へのポインタ（pt_vertex[1]）
		//　　　　計算後モデルの頂点へのポインタ（morph_vertex）をワーク変数にセット
		//　（２）変形前の時間と変形後の時間と現在の時間から割合を求める
		//　（３）割合と変形前後の座標の差分から変形量を求める
		//　      計算後モデルの頂点ワークに格納
		//　      ポインタをそれぞれ＋１
		//　      以上の処理を頂点数だけループして全ての頂点を処理する
		//
		//　＊補足
		//　　変形前の時間    g_Object.morph_time[0]
		//　　変形後の時間    g_Object.morph_time[1]
		//　　現在の時間      g_Object.morph_count
		//
		//**************************************************************************
		//（１）
		spt0 = pt_vertex[0];
		spt1 = pt_vertex[1];
		dpt  = morph_vertex;

		//（２）
		ratio = (float)(g_Object.morph_count - g_Object.morph_time[0]) / (float)(g_Object.morph_time[1] - g_Object.morph_time[0]);

		//（３）
		for (i = 0; i < num_vertex[0]; i++) {
			dpt->x = (spt1->x - spt0->x) * ratio + spt0->x;
			dpt->y = (spt1->y - spt0->y) * ratio + spt0->y;
			dpt->z = (spt1->z - spt0->z) * ratio + spt0->z;
			spt0++;
			spt1++;
			dpt++;
		}
		//**************************************************************************

		switch (pt->morph_dir) {
		case 0:			// 0 > 1
			if(++(pt->morph_count) >= pt->morph_time[1]) pt->morph_dir = 1;	// 方向を反転
			break;
		case 1:			// 1 > 0
			if(--(pt->morph_count) <= 0                ) pt->morph_dir = 0;	// 方向を反転
			break;
		}
		break;
	default:
		break;
	}
}

//----- 描画
void DrawPat()
{
	g_pD3DDevice->SetFVF(FVF_VERTEX);							// 頂点フォーマット指定
	g_pD3DDevice->SetTransform(D3DTS_WORLD, &g_Object.matrix);	// ワールドマトリクスをセット

	if (g_Wireframe) {
		g_pD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);				// ライト無効化
		g_pD3DDevice->SetTexture(0, NULL);									// テクスチャ無し
		g_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);	// ワイヤーフレーム描画
	} else {
		g_pD3DDevice->SetRenderState(D3DRS_LIGHTING, TRUE);					// ライト有効化
		g_pD3DDevice->SetTexture(0, g_pD3DTexture);							// テクスチャ設定
		g_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);		// ソリッド描画
	}

	switch (g_DrawMode) {
	case 0:
		g_pD3DDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,		// モデル０を描画
						0, num_vertex[0], num_face[0], pt_index[0],
						D3DFMT_INDEX16, pt_vertex[0], sizeof(VERTEX));
		break;
	case 1:
		g_pD3DDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,		// モデル１を描画
						0, num_vertex[1], num_face[1], pt_index[1],
						D3DFMT_INDEX16, pt_vertex[1], sizeof(VERTEX));
		break;
	case 2:
		g_pD3DDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,		// モーフィングモデルを描画
						0, num_vertex[0], num_face[0], morph_index,
						D3DFMT_INDEX16, morph_vertex, sizeof(VERTEX));
		break;
	}
}

//=======================================================================================
//	end of files
//=======================================================================================
