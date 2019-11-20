//=======================================================================================
//
//	[ C_Ball ] : Collision of Balls  ���̂̏Փ�
//
//=======================================================================================

//----- �C���N���[�h�t�@�C��
#include <windows.h>
#include <math.h>
#include <mmsystem.h>
#include <tchar.h>			// �ėp�e�L�X�g �}�b�s���O

//----- ���C�u�����t�@�C��
#pragma comment(lib, "winmm.lib")

//----- �\����
struct BALL {
	float	x, y;			// �ʒu���W
	float	vx, vy;			// ���x
	float	m;				// ����
	float	r;				// ���a
};

//----- �萔��`
#define	BALL_QTY		2		// �����{�[����
#define FRAME_TIME		10		// �t���[�������莞��

#define SCREEN_WIDTH	800		// �X�N���[���@��
#define SCREEN_HEIGHT	600		// �X�N���[���@����

#define BOUND_CONST		1.0f	// �͂˂�����W��

BALL initBallData[BALL_QTY] = {
	//     x       y      vx      vy       m      r
	{ 100.0f, 100.0f, 150.0f,  80.0f, 200.0f, 50.0f },		// �{�[���O
	{ 100.0f, 500.0f, 150.0f, -80.0f, 200.0f, 50.0f },		// �{�[���P
};

//----- �O���[�o���ϐ�
const TCHAR szClassName[] = _T("C_Ball");
const TCHAR szAppName[]   = _T("Collision of Balls");

BALL		g_Ball[BALL_QTY];	// �{�[���{��

HWND		g_hWndApp;				// �E�B���h�E�n���h��
int			g_iClientWidth;			// �N���C�A���g�̈�T�C�Y
int			g_iClientHeight;		// �N���C�A���g�̈�T�C�Y
RECT		g_ClientRect;			// �N���C�A���g�̈�
HDC			g_hMemDC;				// ����ʂc�b
HBITMAP		g_hBitmap;				// ����ʃr�b�g�}�b�v

//----- �v���g�^�C�v�錾
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void MainModule();

//-----------------------------------------------------------------------------
// ���C��
//-----------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR args, int cmdShow)
{
	MSG		msg;
	int		time, oldtime;
	int		i;

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
	oldtime = timeGetTime();

	// �{�[���̏�����
	for (i = 0; i < BALL_QTY; i++) {
		g_Ball[i] = initBallData[i];
	}

	// ���C�����[�v
	timeBeginPeriod(1);
	msg.message = WM_NULL;
	while (WM_QUIT != msg.message) {				// WM_QUIT ������܂Ń��[�v
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		time = timeGetTime();
		if (time - oldtime > FRAME_TIME) {
			oldtime = time;
			MainModule();		// ���C���̓R�R
		}
	}
	timeEndPeriod(1);
	return (int)msg.wParam;
}

//-----------------------------------------------------------------------------
// ���b�Z�[�W����
//-----------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HBITMAP	oldbmp;
	static HFONT	oldfont;
	static HPEN		oldpen;
	static HBRUSH	oldbrush;
	HDC				hdc;
	PAINTSTRUCT		ps;

	switch (uMsg) {
	case WM_CREATE:
		hdc = GetDC(hWnd);
		g_hMemDC = CreateCompatibleDC(hdc);		// ����ʂ̍쐬
		g_hBitmap = CreateCompatibleBitmap(hdc, SCREEN_WIDTH, SCREEN_HEIGHT);
		oldbmp   = (HBITMAP)SelectObject(g_hMemDC, g_hBitmap);
		SetBkMode(g_hMemDC, TRANSPARENT);
		SetTextColor(g_hMemDC, RGB(0, 0, 0));
		oldfont  = (HFONT)SelectObject(g_hMemDC, GetStockObject(SYSTEM_FIXED_FONT));
		oldpen   = (HPEN)SelectObject(g_hMemDC, CreatePen(PS_SOLID, 1, RGB(0, 0, 255)));
		oldbrush = (HBRUSH)SelectObject(g_hMemDC, CreateSolidBrush(RGB(0, 0, 255)));
		ReleaseDC(hWnd, hdc);
		return 0;
	case WM_KEYDOWN:			// �L�[�������ꂽ
		switch (wParam) {
		case VK_ESCAPE:
			DestroyWindow(hWnd);				// �I��
			return 0;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, g_hMemDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		if (g_hMemDC) {
			DeleteObject(SelectObject(g_hMemDC, oldbrush));
			DeleteObject(SelectObject(g_hMemDC, oldpen));
			SelectObject(g_hMemDC, oldfont);
			SelectObject(g_hMemDC, oldbmp);
			DeleteDC(g_hMemDC);
		}
		if (g_hBitmap) DeleteObject(g_hBitmap);
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// ���C�����W���[��
//-----------------------------------------------------------------------------
void MainModule()
{
	static float	before_length = 0.0f;		// �O��̃{�[���Ԃ̎��ۂ̋���
	float	length;								// �{�[���Ԃ̎��ۂ̋���
	float	dt;									// �������ԃ���
	float	dx, dy;								// ���W����
	float	e;									// �͂˂�����W��
	float	vx0_new, vy0_new, vx1_new, vy1_new;	// �Փˌ�̑��x
	int		i;
	int		out;

	// �������ɂ���c�b���N���A
	PatBlt(g_hMemDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITENESS);

	// �{�[���̈ړ�
	dt = 0.01f;
	for (i = out = 0; i < BALL_QTY; i++) {
		// �ړ�
		g_Ball[i].x += g_Ball[i].vx * dt;
		g_Ball[i].y += g_Ball[i].vy * dt;
		if (g_Ball[i].x > SCREEN_WIDTH + g_Ball[i].r ||
			g_Ball[i].y > SCREEN_HEIGHT + g_Ball[i].r) {
			out++;
		}
	}
	if (out >= BALL_QTY) {
		for (i = 0; i < BALL_QTY; i++) {
			g_Ball[i] = initBallData[i];
		}
	}
	// �{�[�����m�̃q�b�g�`�F�b�N
	dx = g_Ball[1].x - g_Ball[0].x;				// �{�[���Ԃ̂w��������
	dy = g_Ball[1].y - g_Ball[0].y;				// �{�[���Ԃ̂x��������
	length = sqrtf(dx * dx + dy * dy);			// �{�[���Ԃ̋������v�Z
	if (before_length > length && length < g_Ball[0].r + g_Ball[1].r) {
		// �������Z���Ȃ��Ă���A�����a�̍��v���������̕����Z���̂œ��������Ɣ���
		e = BOUND_CONST;		// �͂˂�����W����
		// �͂˂�����̖@�� e=-(v1'-v2')/(v1-v2) �Ɖ^���ʕۑ��� m1v1'+m2v2' = m1v1+m2v2 ��A��������
		vx0_new = ((g_Ball[0].m - e * g_Ball[1].m) * g_Ball[0].vx + (1 + e) * g_Ball[1].m * g_Ball[1].vx) / (g_Ball[0].m + g_Ball[1].m);
		vy0_new = ((g_Ball[0].m - e * g_Ball[1].m) * g_Ball[0].vy + (1 + e) * g_Ball[1].m * g_Ball[1].vy) / (g_Ball[0].m + g_Ball[1].m);
		vx1_new = ((g_Ball[1].m - e * g_Ball[0].m) * g_Ball[1].vx + (1 + e) * g_Ball[0].m * g_Ball[0].vx) / (g_Ball[0].m + g_Ball[1].m);
		vy1_new = ((g_Ball[1].m - e * g_Ball[0].m) * g_Ball[1].vy + (1 + e) * g_Ball[0].m * g_Ball[0].vy) / (g_Ball[0].m + g_Ball[1].m);
		g_Ball[0].vx = vx0_new;
		g_Ball[0].vy = vy0_new;
		g_Ball[1].vx = vx1_new;
		g_Ball[1].vy = vy1_new;
	}
	before_length = length;						// ������ۑ�
	// �{�[����`��
	for (i = 0; i < BALL_QTY; i++) {
		Ellipse(g_hMemDC,
			(int)(g_Ball[i].x - g_Ball[i].r), (int)(g_Ball[i].y - g_Ball[i].r),
			(int)(g_Ball[i].x + g_Ball[i].r), (int)(g_Ball[i].y + g_Ball[i].r));
	}
	InvalidateRect(g_hWndApp, NULL, TRUE);		// �`��v��
}

//=======================================================================================
//	end
//=======================================================================================
