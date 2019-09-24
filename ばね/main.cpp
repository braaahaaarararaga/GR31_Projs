


#include <windows.h>
#include <math.h>
#include <stdio.h>






// �\���� ///////////////////////////////////////////////

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






// �v���g�^�C�v ///////////////////////////////////////////////

LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );




// �O���[�o���ϐ� ///////////////////////////////////////////////

OBJECT g_Object;


float g_Gravity = 9.8f;

bool g_MouseDown = false;






// ���C���֐� //////////////////////////////////////////////////////////////////////
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{


	// �E�B���h�E�N���X
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
	
	// �E�B���h�E�N���X�o�^
	RegisterClassEx( &wcex );




	// �E�B���h�E�쐬
	HWND hWnd = CreateWindow( "GDIWIndowClass",
						"GDIWIndow",
						WS_OVERLAPPEDWINDOW,
						CW_USEDEFAULT,		// �E�B���h�E�̍����W
						CW_USEDEFAULT,		// �E�B���h�E�̏���W
						800 + GetSystemMetrics( SM_CXDLGFRAME ) * 2,									// �E�B���h�E����
						600 + GetSystemMetrics( SM_CXDLGFRAME ) * 2 + GetSystemMetrics( SM_CYCAPTION ),	// �E�B���h�E�c��
						NULL,
						NULL,
						hInstance,
						NULL);




	// ������

	g_Object.Position.x = 300.0f;
	g_Object.Position.y = 300.0f;

	g_Object.Velocity.x = 0.0f;
	g_Object.Velocity.y = 0.0f;

	g_Object.Mass = 1.0f;





	// �E�B���h�E�\��
	ShowWindow( hWnd, SW_SHOW );


	MSG msg;

	// ���b�Z�[�W���[�v
	while(1)
	{
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			if( msg.message == WM_QUIT )
			{
				// PostQuitMessage()���Ă΂ꂽ�烋�[�v�I��
				break;
			}
			else
			{
				// ���b�Z�[�W�̖|��ƃf�B�X�p�b�`
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
        }
		else
		{

		}
	}



	
	// �E�B���h�E�N���X�̓o�^������
	UnregisterClass( "GDIWIndowClass", wcex.hInstance );





	return (int)msg.wParam;
}












// �E�B���h�E�v���V�[�W�� //////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

	switch( uMsg )
	{

		// �E�B���h�E����
		case WM_CREATE:
		{
			// �^�C�}�[�쐬
			SetTimer( hWnd, 1, 30, NULL );
			break;
		}




		// �^�C�}�[
		case WM_TIMER:
		{
			float dt = 0.03f;

			// �^���X�V //////////////////////////////


			if( !g_MouseDown )
			{
				VECTOR2D f;

				f.x = 0.0f;
				f.y = 0.0f;


				// ����
				f.x += (300.0f - g_Object.Position.x) * 100.0f * dt;
				f.y += (300.0f - g_Object.Position.y) * 100.0f * dt;

				// �d��
				f.y += g_Gravity * g_Object.Mass * dt;

				// ��R
				f.x -= g_Object.Velocity.x * 10.0f * dt;
				f.y -= g_Object.Velocity.y * 10.0f * dt;



				// ���͂ɂ�����
				g_Object.Velocity.x += (f.x / g_Object.Mass) * dt;
				g_Object.Velocity.y += (f.y / g_Object.Mass) * dt;

				// �ړ�
				g_Object.Position.x += g_Object.Velocity.x * dt;
				g_Object.Position.y += g_Object.Velocity.y * dt;
			}



			// �S��ʍX�V
			InvalidateRect( hWnd, NULL, TRUE );

			break;
		}




		// �I��
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}



		// �T�C�Y�ύX
		case WM_SIZE:
		{
			InvalidateRect( hWnd, NULL, TRUE );
			break;
		}



		// �L�[����
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



		// �}�E�X�ړ�
		case WM_MOUSEMOVE:
		{
			if( g_MouseDown  )
			{
				// �I�u�W�F�N�g���ړ�
				g_Object.Position.x = (float)LOWORD(lParam);
				g_Object.Position.y = (float)HIWORD(lParam);

				g_Object.Velocity.x = 0.0f;
				g_Object.Velocity.y = 0.0f;
			}

			break;
		}



		// �}�E�X�{�^���_�E��
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



		// �}�E�X�{�^���A�b�v
		case WM_LBUTTONUP:
		{
			g_MouseDown = false;
			break;
		}



		// �`��
		case WM_PAINT:
		{

			PAINTSTRUCT ps;
			HPEN pen, oldPen;




			HDC hdc = BeginPaint( hWnd, &ps );





			// �I�u�W�F�N�g�`�� /////////////////////////////////////////////////
			pen = CreatePen( PS_SOLID, 1, RGB( 0, 0, 255 ) );
			oldPen = (HPEN)SelectObject( hdc, pen );

			Ellipse( hdc, (int)(g_Object.Position.x - 20),
							(int)(g_Object.Position.y - 20),
							(int)(g_Object.Position.x + 20),
							(int)(g_Object.Position.y + 20) );//�ȉ~�`��

			SelectObject( hdc, oldPen );
			DeleteObject( pen );




			// �΂˕`�� /////////////////////////////////////////////////
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




