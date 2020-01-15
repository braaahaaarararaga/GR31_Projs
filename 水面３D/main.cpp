//=======================================================================================
//
//	[ ���ʁi�R�c�j ] : ���ʂ̋������Č�����i�R�c�o�[�W�����j
//
//=======================================================================================

//----- �C���N���[�h�t�@�C��
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>

//----- �\����
struct VECTOR {
	float	x, y, z;		// �R�c�̊e����
};

struct WAVE {
	int		status;				// �X�e�[�^�X
	VECTOR	position;			// �ʒu���W (Position)
	float	amplitude;			// �U�� (Amplitude)
	float	time;				// ����
};

//----- �萔��`
#define FRAME_TIME				20						// �t���[�������莞��

#define SCREEN_WIDTH			800						// �X�N���[���@��
#define SCREEN_HEIGHT			600						// �X�N���[���@����
#define AXIS_X_OFFSET			SCREEN_WIDTH/2			// �w���W�I�t�Z�b�g
#define AXIS_Y_OFFSET			SCREEN_HEIGHT/2			// �x���W�I�t�Z�b�g

#define WAVE_PITCH				10						// �E�F�[�u�|�C���g�̊Ԋu
#define WAVE_QTY				(40+1)					// �E�F�[�u�|�C���g�̑���
#define WAVE_AMPLITUDE			20.0f					// �E�F�[�u�|�C���g�̐U��
#define WAVE_LENGTH				20.0f					// �E�F�[�u�|�C���g�̔g��
#define WAVE_CYCLE				1.0f					// �E�F�[�u�|�C���g�̎���

//----- �O���[�o���ϐ�
const TCHAR szClassName[] = _T("Water Surface");
const TCHAR szAppName[]   = _T("[ ���ʁi�R�c�j ] Water Surface");

WAVE		g_Wave[WAVE_QTY][WAVE_QTY];		// �E�F�[�u�|�C���g

HWND		g_hWndApp;					// �E�B���h�E�n���h��
int			g_iClientWidth;				// �N���C�A���g�̈�T�C�Y
int			g_iClientHeight;			// �N���C�A���g�̈�T�C�Y
RECT		g_ClientRect;				// �N���C�A���g�̈�
HDC			g_hMemDC;					// ����ʂc�b
HBITMAP		g_hBitmap;					// ����ʃr�b�g�}�b�v
HPEN		g_PenGrid0, g_PenGrid1;		// �O���b�h�̃y�� 
HPEN		g_PenWave;					// �E�F�[�u�|�C���g�̃y��
HBRUSH		g_BrushWave;				// �E�F�[�u�|�C���g�̃u���V

//----- �v���g�^�C�v�錾
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void MainModule();
void ExecWavePoint();
void DispInfo();
void DrawWaveLine();

//---------------------------------------------------------------------------------------
// ���C��
//---------------------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR args, int cmdShow)
{
	MSG		msg;
	int		x, z;

	// �E�B���h�E�N���X��o�^
	WNDCLASS wndClass = {
		0, WndProc, 0, 0, hInst,
		LoadIcon(hInst, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
		0, NULL, szClassName
	};
	if (RegisterClass(&wndClass) == 0) return false;

	// �E�C���h�E���쐬
	g_hWndApp = CreateWindow(
		szClassName, szAppName,
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		SCREEN_WIDTH  + GetSystemMetrics(SM_CXFIXEDFRAME) * 2,
		SCREEN_HEIGHT + GetSystemMetrics(SM_CYFIXEDFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION),
		NULL, NULL, hInst, NULL);
	if (!g_hWndApp) return false;
	ShowWindow(g_hWndApp, cmdShow);
	UpdateWindow(g_hWndApp);

	// �ϐ�������
	for (z = 0; z < WAVE_QTY; z++) {
		for (x = 0; x < WAVE_QTY; x++) {
			g_Wave[z][x].status = 0;		// �E�F�[�u�|�C���g�̏�����
		}
	}

	// ���C�����[�v
	msg.message = WM_NULL;
	while (WM_QUIT != msg.message) {					// WM_QUIT ������܂Ń��[�v
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
// ���b�Z�[�W����
//-----------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HGDIOBJ	oldfont;
	static HPEN		oldpen;
	static HBRUSH	oldbrush;
	HDC				hdc;
	PAINTSTRUCT		ps;

	switch (uMsg) {
	case WM_CREATE:
		hdc = GetDC(hWnd);
		g_hMemDC = CreateCompatibleDC(hdc);		// ����ʂ̍쐬
		g_hBitmap = CreateCompatibleBitmap(hdc, SCREEN_WIDTH, SCREEN_HEIGHT);
		SelectObject(g_hMemDC, g_hBitmap);
		SetBkMode(g_hMemDC, TRANSPARENT);
		SetTextColor(g_hMemDC, RGB(0, 0, 0));
		oldfont  = SelectObject(g_hMemDC, GetStockObject(SYSTEM_FIXED_FONT));
		g_PenGrid0     = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
		g_PenGrid1     = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
		g_PenWave      = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
		g_BrushWave    = CreateSolidBrush(RGB(0, 0, 255));
		ReleaseDC(hWnd, hdc);
		SetTimer(hWnd, 0, FRAME_TIME, NULL);
		return 0;
	case WM_KEYDOWN:			// �L�[�������ꂽ
		switch (wParam) {
		case VK_ESCAPE:	DestroyWindow(hWnd);	return 0;			// �I��
		}
		break;
	case WM_PAINT:				// �`��
		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, g_hMemDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		return 0;
	case WM_TIMER:
		MainModule();		// ���C���̓R�R
		return 0;
	case WM_DESTROY:
		KillTimer(hWnd, 0);
		if (g_hBitmap) DeleteObject(g_hBitmap);
		if (g_hMemDC) {
			DeleteObject(SelectObject(g_hMemDC, g_BrushWave));
			DeleteObject(SelectObject(g_hMemDC, g_PenWave));
			DeleteObject(SelectObject(g_hMemDC, g_PenGrid1));
			DeleteObject(SelectObject(g_hMemDC, g_PenGrid0));
			DeleteObject(SelectObject(g_hMemDC, oldfont));
			DeleteDC(g_hMemDC);
		}
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//---------------------------------------------------------------------------------------
// ���C�����W���[��
//---------------------------------------------------------------------------------------
void MainModule()
{
	// �������ɂ���c�b���N���A
	PatBlt(g_hMemDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITENESS);

	ExecWavePoint();			// �E�F�[�u�|�C���g�\���������s
	DrawWaveLine();				// �E�F�[�u�|�C���g�Ԃ����C���Őڑ�

	// ���l�\��
	TCHAR	str[256];
	int		nLen;
	nLen = _stprintf(str, _T("����   (%8.3f)"), g_Wave[0][0].time);
	TextOut(g_hMemDC, 0, 0, str, nLen);

	InvalidateRect(g_hWndApp, NULL, TRUE);		// �`��v��
}

//---------------------------------------------------------------------------------------
// �E�F�[�u�|�C���g���W���[��
//---------------------------------------------------------------------------------------
void ExecWavePoint()
{
	int		x, z;
	float	dx, dz, length;
	float	px, py;
	HPEN	oldPen;
	HBRUSH	oldBrush;

	oldPen   = (HPEN)SelectObject(g_hMemDC, g_PenWave);
	oldBrush = (HBRUSH)SelectObject(g_hMemDC, g_BrushWave);
	for (z = 0; z < WAVE_QTY; z++) {
		for (x = 0; x < WAVE_QTY; x++) {
			switch (g_Wave[z][x].status) {
			case 0:		// ������
				g_Wave[z][x].position.x = (float)((x - WAVE_QTY / 2) * WAVE_PITCH);	// �w���W
				g_Wave[z][x].position.y = 0.0f;										// �x���W
				g_Wave[z][x].position.z = (float)((z - WAVE_QTY / 2) * WAVE_PITCH);	// �y���W

				g_Wave[z][x].amplitude = WAVE_AMPLITUDE;	// �U��
				g_Wave[z][x].time      = 0.0f;
				g_Wave[z][x].status    = 1;
				// THRU
			case 1:		// ���C������

				//===============================================================================
				//
				//    �����Ɍv�Z�������ăE�F�[�u�|�C���g�𓮂���
				//
				//===============================================================================

				// �U�����_����̋��������߂�
				dx = g_Wave[z][x].position.x - g_Wave[WAVE_QTY / 2][WAVE_QTY / 2].position.x;
				dz = g_Wave[z][x].position.z - g_Wave[WAVE_QTY / 2][WAVE_QTY / 2].position.z;
				length = sqrtf(dx * dx + dz * dz);

				// ���������ɐU���̎�����ψʂ����߂�
				g_Wave[z][x].position.y = g_Wave[z][x].??? * ???(2.0f * (float)M_PI * ((??? / WAVE_LENGTH) - (g_Wave[z][x].??? / WAVE_CYCLE)));

				// ���Ԃ�i�߂�
				g_Wave[z][x].time += 0.01f;

				break;
			}
			// �E�F�[�u�|�C���g��`��
			px = AXIS_X_OFFSET + g_Wave[z][x].position.x + g_Wave[z][x].position.z;
			py = AXIS_Y_OFFSET - g_Wave[z][x].position.y - g_Wave[z][x].position.z;
			Ellipse(g_hMemDC, (int)(px - 2.0f), (int)(py - 2.0f), (int)(px + 2.0f), (int)(py + 2.0f));
		}
	}
	SelectObject(g_hMemDC, oldPen);
	SelectObject(g_hMemDC, oldBrush);
}

//---------------------------------------------------------------------------------------
// �|�C���g�Ԃ����C���ŕ`��
//---------------------------------------------------------------------------------------
void DrawWaveLine()
{
	int		x, z;
	float	px, py, px2, py2;
	HPEN	oldPen;
	
	oldPen   = (HPEN)SelectObject(g_hMemDC, g_PenWave  );
	for (z = 0; z < WAVE_QTY; z++) {
		for (x = 0; x < WAVE_QTY; x++) {
			if (x < WAVE_QTY - 1) {
				px  = AXIS_X_OFFSET + g_Wave[z][x].position.x + g_Wave[z][x].position.z;
				py  = AXIS_Y_OFFSET - g_Wave[z][x].position.y - g_Wave[z][x].position.z;
				px2 = AXIS_X_OFFSET + g_Wave[z][x + 1].position.x + g_Wave[z][x + 1].position.z;
				py2 = AXIS_Y_OFFSET - g_Wave[z][x + 1].position.y - g_Wave[z][x + 1].position.z;
				MoveToEx(g_hMemDC, (int)px, (int)py, NULL);
				LineTo(g_hMemDC, (int)px2, (int)py2);
			}
			if (z < WAVE_QTY - 1) {
				px  = AXIS_X_OFFSET + g_Wave[z][x].position.x + g_Wave[z][x].position.z;
				py  = AXIS_Y_OFFSET - g_Wave[z][x].position.y - g_Wave[z][x].position.z;
				px2 = AXIS_X_OFFSET + g_Wave[z + 1][x].position.x + g_Wave[z + 1][x].position.z;
				py2 = AXIS_Y_OFFSET - g_Wave[z + 1][x].position.y - g_Wave[z + 1][x].position.z;
				MoveToEx(g_hMemDC, (int)px, (int)py, NULL);
				LineTo(g_hMemDC, (int)px2, (int)py2);
			}
		}
	}
	SelectObject(g_hMemDC, oldPen);
}

//=======================================================================================
//	end
//=======================================================================================
