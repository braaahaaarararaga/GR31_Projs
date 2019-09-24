//-------- �C���N���[�h�w�b�_
#define STRICT				// Windows�v���O�����ɑ΂���u�^�ۏ؁v������
#include <windows.h>		// Windows�v���O�����ɂ͂����t����
#include <tchar.h>			// �ėp�e�L�X�g �}�b�s���O
#include <math.h>

//-------- �萔��`
#define CLASS_NAME		_T("GR31_Hermite2")		// �E�B���h�E�N���X��
#define CAPTION_NAME	_T("[ GR31 Hermite2 ] (������)")	// �L���v�V������
const UINT	SCREEN_WIDTH	= 640;				// �X�N���[�����r�b�g�}�b�v��
const UINT	SCREEN_HEIGHT	= 480;				// �X�N���[�����r�b�g�}�b�v����

enum _bmp {
	BMP_BG = 0,		// 0
	BMP_PLAYER,		// 1
	BMP_ENEMY,		// 2
	BMP_QTY,		// 3 BMP�̑���������
};

LPCTSTR		bmp_file[] = {
	_T("bg.bmp"),		// 0 �a�f
	_T("player.bmp"),	// 1 ���@
	_T("enemy.bmp"),	// 2 �G�@
};

const UINT	MOVE_SPEED	= 8;				// ���@�ړ��X�s�[�h
const UINT	BG_W		= 640;				// �a�f�@��
const UINT	BG_H		= 480;				// �a�f�@����
const UINT	PLAYER_W	= 48;				// ���@�@��
const UINT	PLAYER_H	= 32;				// ���@�@����
const UINT	ENEMY_W		= 32;				// �G�@�@��
const UINT	ENEMY_H		= 32;				// �G�@�@����
const UINT	CURVE_DIV	= 10;				// �Ȑ�������

const float	PLAYER_VECTOR_X	= -500.0f;		// ���@���@�ڐ��x�N�g���@�w����
const float	PLAYER_VECTOR_Y	= -500.0f;		// ���@���@�ڐ��x�N�g���@�x����
const float	ENEMY_VECTOR_X	=  300.0f;		// �G�@���@�ڐ��x�N�g���@�w����
const float	ENEMY_VECTOR_Y	=  200.0f;		// �G�@���@�ڐ��x�N�g���@�x����

//-------- �\���̒�`
struct LASER {
	int		status;							// �X�e�[�^�X
	int		start;							// ���[�U�[�J�n�C���f�b�N�X
	int		end;							// ���[�U�[�I���C���f�b�N�X
	POINT	curve_pt[CURVE_DIV + 1];		// �G���~�[�g�Ȑ���̍��W
};

//-------- �O���[�o���ϐ���`
HINSTANCE	g_hInst;						// �C���X�^���X�n���h��
HDC			g_hMemDC[BMP_QTY];				// �������c�b�i�a�l�o�ێ��p�j
int			player_x, player_y;				// ���@���W
int			enemy_x , enemy_y;				// �G�@���W

LASER		laser;							// ���[�U�[

//-------------------------------------------------------------------------------------------------
// �v���g�^�C�v�錾
//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//-------------------------------------------------------------------------------------------------
//	���C��
//-------------------------------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int iCmdShow)
{
	HWND	hWnd;		// �E�B���h�E�n���h��
	MSG		msg;		// ���b�Z�[�W���

	g_hInst = hInstance;

	// �E�B���h�E�N���X�̓o�^
	WNDCLASSEX wc = {
		sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WindowProc,
		0, 0, hInstance, LoadIcon(hInstance, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)GetStockObject(WHITE_BRUSH), NULL, CLASS_NAME, NULL};
	if (RegisterClassEx(&wc) == 0) return false;	// �E�B���h�E�N���X��o�^

	// �E�B���h�E���쐬
	hWnd = CreateWindowEx(
		WS_EX_TOPMOST, CLASS_NAME, CAPTION_NAME, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION,
		CW_USEDEFAULT, CW_USEDEFAULT,
		SCREEN_WIDTH  + GetSystemMetrics(SM_CXFIXEDFRAME) * 2,
		SCREEN_HEIGHT + GetSystemMetrics(SM_CYFIXEDFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION),
		NULL, NULL, hInstance, NULL);
	if (hWnd == NULL) return false;
	ShowWindow(hWnd, iCmdShow);					// �E�B���h�E�̕\����Ԃ�ݒ�
	UpdateWindow(hWnd);							// �E�B���h�E�̃N���C�A���g�̈���X�V

	// ���b�Z�[�W���[�v
	msg.message = WM_NULL;
	while (WM_QUIT != msg.message) {					// WM_QUIT ������܂Ń��[�v
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {	// ���b�Z�[�W���`�F�b�N
			TranslateMessage(&msg);						// ���z�L�[���b�Z�[�W�𕶎����b�Z�[�W�֕ϊ�
			DispatchMessage(&msg);						// �E�B���h�E�v���V�[�W���փ��b�Z�[�W�𑗏o
		}
	}
	return msg.wParam;
}

//-------------------------------------------------------------------------------------------------
//	�E�B���h�E�v���V�[�W��
//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC			hdc;			// �f�o�C�X�R���e�L�X�g�n���h��
	HBITMAP		hDstBmp;		// �r�b�g�}�b�v�n���h��
	PAINTSTRUCT	ps;
	HPEN		hOldPen;
	int			i;

	switch (uMsg) {						// ���b�Z�[�W���`�F�b�N
	case WM_CREATE:
		// �r�b�g�}�b�v�t�@�C�������[�h
		hdc = GetDC(hWnd);
		for (i = 0; i < BMP_QTY; i++) {
			hDstBmp = (HBITMAP)LoadImage(g_hInst, bmp_file[i], IMAGE_BITMAP, 0, 0,
				LR_CREATEDIBSECTION | LR_LOADFROMFILE);
			g_hMemDC[i] = CreateCompatibleDC(hdc);				// �������c�b���쐬
			SelectObject(g_hMemDC[i], hDstBmp);					// �r�b�g�}�b�v��I��
			DeleteObject(hDstBmp);								// ���[�h�����r�b�g�}�b�v��j��
		}
		ReleaseDC(hWnd, hdc);

		// �ϐ�������
		player_x = 120;
		player_y = 240;
		enemy_x  = 550;
		enemy_y  = 240;
		laser.status = 0;

		SetTimer(hWnd, 1, 20, NULL);		// �^�C�}�[���N��
		return 0;
	case WM_KEYDOWN:							// �L�[�{�[�h�������ꂽ
		switch (wParam) {							// �p�����[�^���`�F�b�N
		case VK_ESCAPE:								// [ESC]�L�[�������ꂽ
			DestroyWindow(hWnd);					// �E�B���h�E�j�����w��
			return 0;
		}
		break;
	case WM_TIMER:
		// ���@�𑀍�
		if(GetAsyncKeyState(VK_UP   )) player_y -= MOVE_SPEED;
		if(GetAsyncKeyState(VK_DOWN )) player_y += MOVE_SPEED;
		if(GetAsyncKeyState(VK_LEFT )) player_x -= MOVE_SPEED;
		if(GetAsyncKeyState(VK_RIGHT)) player_x += MOVE_SPEED;
		if(GetAsyncKeyState(VK_SPACE) && laser.status == 0) laser.status = 1;	// ���[�U�[�𔭎�
		// �ĕ`���v��
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		BitBlt(hdc, 0       , 0       , BG_W    , BG_H    , g_hMemDC[BMP_BG    ], 0, 0, SRCCOPY);	// �a�f�`��
		BitBlt(hdc, player_x, player_y, PLAYER_W, PLAYER_H, g_hMemDC[BMP_PLAYER], 0, 0, SRCCOPY);	// ���@�`��
		BitBlt(hdc, enemy_x , enemy_y , ENEMY_W , ENEMY_H , g_hMemDC[BMP_ENEMY ], 0, 0, SRCCOPY);	// �G�@�`��

		// ���[�U�[��`��
		hOldPen = (HPEN)SelectObject(hdc, CreatePen(PS_SOLID, 3, RGB(0, 255, 255)));	// ���[�U�[�p�y�����쐬

		//----------------------------------------------------------------
		//  �����ɃG���~�[�g�Ȑ����g��������������
		//  �Ȑ����[�U�[��`�悳����
		//----------------------------------------------------------------

		//****************************************************************
		
		// �ϐ���`
		float	h00, h01, h10, h11;
		float	t;

		// �G���~�[�g�Ȑ����v�Z
		for(i = 0; i <= CURVE_DIV; i++) {
			t = (float)i / CURVE_DIV;
			h00 = pow(t - 1.0f, 2) * (2 * t + 1);
			h01 = pow(t,2) * (3 - 2 * t);
			h10 = pow(1 - t, 2) * t;
			h11 = (t - 1) * pow(t,2);
			laser.curve_pt[i].x = (int)(h00 * player_x + h01 * enemy_x + h10 * PLAYER_VECTOR_X + h11 * ENEMY_VECTOR_X);
			laser.curve_pt[i].y = (int)(h00 * player_y + h01 * enemy_y + h10 * PLAYER_VECTOR_Y + h11 * ENEMY_VECTOR_Y);
		}

		// ���[�U�[�{�̂̐���
		switch(laser.status) {
		case 0:					// ���˂��Ă��Ȃ��i�ҋ@���j
			laser.start = 0;
			laser.end   = 0;
			break;
		case 1:					// ���[�U�[�I���C���f�b�N�X�����@����G�@��
			if(laser.end < CURVE_DIV) {
				laser.end++;			// �I���C���f�b�N�X��i�߂�
			} else {
				laser.status++;			// ���B�����玟��
			}
			break;
		case 2:					// ���[�U�[�J�n�C���f�b�N�X�����@����G�@��
			if(laser.start < CURVE_DIV) {
				laser.start++;			// �J�n�C���f�b�N�X��i�߂�
			} else {
				laser.status = 0;		// ���B������ҋ@����
				laser.start = 0;
				laser.end   = 0;
			}
			break;
		}

		// �G���~�[�g�Ȑ���`��
		for(i = laser.start; i < laser.end; i++) {
			MoveToEx(hdc, laser.curve_pt[i].x, laser.curve_pt[i].y, NULL);
			LineTo(hdc, laser.curve_pt[i+1].x, laser.curve_pt[i+1].y);
		}

		//****************************************************************

		DeleteObject(SelectObject(hdc, hOldPen));	// �쐬�����y�����폜

		EndPaint(hWnd, &ps);
		return 0;
	case WM_CLOSE:
		for(i = 0; i < BMP_QTY; i++) {
			DeleteDC(g_hMemDC[i]);				// �������c�b���폜
		}
		DestroyWindow(hWnd);
		return 0;
	case WM_DESTROY:							// �I���w��������
		KillTimer(hWnd, 1);							// �^�C�}�[���~
		PostQuitMessage(0);							// �V�X�e���ɃX���b�h�̏I����v��
		return 0;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);	// ����E�B���h�E�v���V�[�W�����ďo
}

//=================================================================================================
//	end
//=================================================================================================
