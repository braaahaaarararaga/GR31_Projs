


#include <windows.h>
#include <math.h>
#include <stdio.h>






// 構造体 ///////////////////////////////////////////////

struct VECTOR2D
{
	float x;
	float y;
};



struct OBJECT
{
	VECTOR2D Position;
	VECTOR2D Velocity;

	float Mass;
};






// プロトタイプ ///////////////////////////////////////////////

LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );




// グローバル変数 ///////////////////////////////////////////////

OBJECT g_Object;


float g_Gravity = 9.8f;

bool g_MouseDown = false;






// メイン関数 //////////////////////////////////////////////////////////////////////
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{


	// ウィンドウクラス
	WNDCLASSEX	wcex =
	{
		sizeof( WNDCLASSEX ),
		CS_CLASSDC,
		WndProc,
		0,
		0,
		hInstance,
		NULL,
		LoadCursor( NULL, IDC_ARROW ),
		(HBRUSH)( COLOR_WINDOW + 1 ),
		NULL,
		"GDIWIndowClass",
		NULL
	};
	
	// ウィンドウクラス登録
	RegisterClassEx( &wcex );




	// ウィンドウ作成
	HWND hWnd = CreateWindow( "GDIWIndowClass",
						"GDIWIndow",
						WS_OVERLAPPEDWINDOW,
						CW_USEDEFAULT,		// ウィンドウの左座標
						CW_USEDEFAULT,		// ウィンドウの上座標
						800 + GetSystemMetrics( SM_CXDLGFRAME ) * 2,									// ウィンドウ横幅
						600 + GetSystemMetrics( SM_CXDLGFRAME ) * 2 + GetSystemMetrics( SM_CYCAPTION ),	// ウィンドウ縦幅
						NULL,
						NULL,
						hInstance,
						NULL);




	// 初期化

	g_Object.Position.x = 300.0f;
	g_Object.Position.y = 300.0f;

	g_Object.Velocity.x = 0.0f;
	g_Object.Velocity.y = 0.0f;

	g_Object.Mass = 1.0f;





	// ウィンドウ表示
	ShowWindow( hWnd, SW_SHOW );


	MSG msg;

	// メッセージループ
	while(1)
	{
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			if( msg.message == WM_QUIT )
			{
				// PostQuitMessage()が呼ばれたらループ終了
				break;
			}
			else
			{
				// メッセージの翻訳とディスパッチ
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
        }
		else
		{

		}
	}



	
	// ウィンドウクラスの登録を解除
	UnregisterClass( "GDIWIndowClass", wcex.hInstance );





	return (int)msg.wParam;
}












// ウィンドウプロシージャ //////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

	switch( uMsg )
	{

		// ウィンドウ生成
		case WM_CREATE:
		{
			// タイマー作成
			SetTimer( hWnd, 1, 30, NULL );
			break;
		}




		// タイマー
		case WM_TIMER:
		{
			float dt = 0.03f;

			// 運動更新 //////////////////////////////


			if( !g_MouseDown )
			{
				VECTOR2D f;

				f.x = 0.0f;
				f.y = 0.0f;


				// 張力
				f.x += (300.0f - g_Object.Position.x) * 100.0f * dt;
				f.y += (300.0f - g_Object.Position.y) * 100.0f * dt;

				// 重力
				f.y += g_Gravity * g_Object.Mass * dt;

				// 抵抗
				f.x -= g_Object.Velocity.x * 10.0f * dt;
				f.y -= g_Object.Velocity.y * 10.0f * dt;



				// 張力による加速
				g_Object.Velocity.x += (f.x / g_Object.Mass) * dt;
				g_Object.Velocity.y += (f.y / g_Object.Mass) * dt;

				// 移動
				g_Object.Position.x += g_Object.Velocity.x * dt;
				g_Object.Position.y += g_Object.Velocity.y * dt;
			}



			// 全画面更新
			InvalidateRect( hWnd, NULL, TRUE );

			break;
		}




		// 終了
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}



		// サイズ変更
		case WM_SIZE:
		{
			InvalidateRect( hWnd, NULL, TRUE );
			break;
		}



		// キー入力
		case WM_KEYDOWN:
		{
			switch(wParam)
			{
				case VK_ESCAPE:
					DestroyWindow( hWnd );
					break;
			}
			break;
		}



		// マウス移動
		case WM_MOUSEMOVE:
		{
			if( g_MouseDown  )
			{
				// オブジェクトを移動
				g_Object.Position.x = (float)LOWORD(lParam);
				g_Object.Position.y = (float)HIWORD(lParam);

				g_Object.Velocity.x = 0.0f;
				g_Object.Velocity.y = 0.0f;
			}

			break;
		}



		// マウスボタンダウン
		case WM_LBUTTONDOWN:
		{
			int x, y;

			x = (short)LOWORD(lParam);
			y = (short)HIWORD(lParam);


			if( g_Object.Position.x - 20 <= x && x <= g_Object.Position.x + 20 &&
				g_Object.Position.y - 20 <= y && y <= g_Object.Position.y + 20 )
			{
				g_MouseDown = true;
			}
			else
			{
				g_MouseDown = false;
			}

			break;
		}



		// マウスボタンアップ
		case WM_LBUTTONUP:
		{
			g_MouseDown = false;
			break;
		}



		// 描画
		case WM_PAINT:
		{

			PAINTSTRUCT ps;
			HPEN pen, oldPen;




			HDC hdc = BeginPaint( hWnd, &ps );





			// オブジェクト描画 /////////////////////////////////////////////////
			pen = CreatePen( PS_SOLID, 1, RGB( 0, 0, 255 ) );
			oldPen = (HPEN)SelectObject( hdc, pen );

			Ellipse( hdc, (int)(g_Object.Position.x - 20),
							(int)(g_Object.Position.y - 20),
							(int)(g_Object.Position.x + 20),
							(int)(g_Object.Position.y + 20) );//楕円描画

			SelectObject( hdc, oldPen );
			DeleteObject( pen );




			// ばね描画 /////////////////////////////////////////////////
			pen = CreatePen( PS_SOLID, 1, RGB( 255, 0, 0 ) );
			oldPen = (HPEN)SelectObject( hdc, pen );

			MoveToEx( hdc, 300, 200, NULL );
			LineTo( hdc, (int)g_Object.Position.x, (int)g_Object.Position.y );

			SelectObject( hdc, oldPen );
			DeleteObject( pen );





			EndPaint( hWnd, &ps );

			break;
		}



		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}




