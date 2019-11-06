//=======================================================================================
//
//	  [ �z ] : �o�l��A�����ĕz�����
//
//    �i����j
//    �J�[�\���L�[�F���̋�����ω������܂�
//    [ W ] : �z�ɗ^���镗�̂n�m�^�n�e�e�؂�ւ�
//    �}�E�X�œ_���h���b�O����ƈړ����܂��i����̓_�͈ړ��セ�̏�ɌŒ�j
//
//=======================================================================================

#define _CRT_SECURE_NO_WARNINGS
//----- �C���N���[�h�t�@�C��
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <math.h>
#include <mmsystem.h>

//----- ���C�u�����t�@�C��
#pragma comment(lib, "winmm.lib")

//----- �}�N����`
#define F_ZERO(v)			v.x=v.y=v.z=0.0f
#define F_SET(v,px,py,pz)	v.x=px;v.y=py;v.z=pz

//----- �萔��`
#define FRAME_TIME				16				// �t���[�������莞��

#define SCREEN_WIDTH			800				// �X�N���[���@��
#define SCREEN_HEIGHT			600				// �X�N���[���@����
#define AXIS_X_OFFSET			SCREEN_WIDTH/2	// �w���W�I�t�Z�b�g
#define AXIS_Y_OFFSET			SCREEN_HEIGHT/2	// �x���W�I�t�Z�b�g

#define PARTICLE_HORIZONTAL		10				// ���q�@�c��������
#define PARTICLE_VERTICAL		10				// ���q�@����������

#define SPRING_NUMS				(PARTICLE_HORIZONTAL*(PARTICLE_VERTICAL+1) + \
								 PARTICLE_VERTICAL*(PARTICLE_HORIZONTAL+1) + \
								 PARTICLE_VERTICAL*PARTICLE_HORIZONTAL*2)	// �X�v�����O����

#define SPRING_DEFAULT_LENGTH	40.0f			// �o�l���R��
#define SPRING_COEFFICIENT		40.0f			// �o�l�W��
#define SPRING_MASS				2.0f			// �o�l����

#define ATT_COEFFICIENT			0.5f			// �����W��(Attenuation)
#define DELTA_TIME				0.1f			// �ϕ��P�ʎ���

#define WIND_FORCE_DEF_X		40.0f			// ���͂w����
#define WIND_FORCE_DEF_Y		20.0f			// ���͂x����
#define WIND_FORCE_DEF_Z		30.0f			// ���͂y����

//----- �\����
struct VECTOR {
	float	x, y, z;			// �e����
};

struct WINDFORCE {
	int		status;				// �X�e�[�^�X
	VECTOR	force;				// ���̓x�N�g��
};

struct PARTICLE {
	bool	onLock;				// �Œ�t���O
	bool	onDrag;				// �}�E�X�h���b�O������t���O
	VECTOR	position;			// �ʒu���W (Position)
	VECTOR	velocity;			// ���x (Velocity)
	VECTOR	acceleration;		// �����x (Acceleration)
	VECTOR	resultant;			// ���� (Resultant)
	VECTOR	gravity;			// �d�� (Gravity)
};

struct P_REF {
	int		horz;				// �������C���f�b�N�X
	int		vert;				// �c�����C���f�b�N�X
};

struct SPRING {
	P_REF	p1;					// �ڑ�����Ă��闱�q�P
	P_REF	p2;					// �ڑ�����Ă��闱�q�Q
	float	length;				// ���R��
};

struct CLOTH {
	int			status;														// �X�e�[�^�X
	PARTICLE	Particle[PARTICLE_VERTICAL+1][PARTICLE_HORIZONTAL+1];		// �\�����q
	SPRING		Spring[SPRING_NUMS];										// ���q�Ԃ̃o�l
};

//----- �O���[�o���ϐ�
const TCHAR szClassName[] = _T("Cloth");
const TCHAR szAppName[]   = _T("[ �z ] Cloth");

CLOTH		g_Cloth;					// �z�I�u�W�F�N�g
WINDFORCE	g_WindForce;				// ���̓I�u�W�F�N�g

HWND		g_hWndApp;					// �E�B���h�E�n���h��
int			g_iClientWidth;				// �N���C�A���g�̈�T�C�Y
int			g_iClientHeight;			// �N���C�A���g�̈�T�C�Y
RECT		g_ClientRect;				// �N���C�A���g�̈�
HDC			g_hMemDC;					// ����ʂc�b
HBITMAP		g_hBitmap;					// ����ʃr�b�g�}�b�v
bool		g_bOnButton;				// �}�E�X�{�^���N���b�N�t���O
bool		g_bOnInfo;					// �x�N�g���E���l���\���X�C�b�`
bool		g_bOnWind;					// ���̓X�C�b�`

//----- �v���g�^�C�v�錾
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void MainModule();
void ExecGrid();
void ExecWindForce();
void ExecCloth();
void DispInfo();

//---------------------------------------------------------------------------------------
// ���C��
//---------------------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR args, int cmdShow)
{
	MSG		msg;
	int		time, oldtime;

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
	g_bOnButton = false;		// �}�E�X�{�^���t���O���N���A
	g_bOnInfo   = true;			// ���\���X�C�b�`���n�m
	g_bOnWind   = true;			// ���X�C�b�`���n�m
	g_WindForce.status = 0;		// ���̓X�e�[�^�X�̏�����
	g_Cloth.status     = 0;		// �z�X�e�[�^�X�̏�����

	// ���C�����[�v
	timeBeginPeriod(1);
	msg.message = WM_NULL;
	while (WM_QUIT != msg.message) {					// WM_QUIT ������܂Ń��[�v
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		time = timeGetTime();
		if (time - oldtime > FRAME_TIME) {
			oldtime = time;
			MainModule();		// ���C���̓R�R
		}
		Sleep(1);
	}
	timeEndPeriod(1);
	return 0;
}

//-----------------------------------------------------------------------------
// ���b�Z�[�W����
//-----------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC				hdc;
	PAINTSTRUCT		ps;
	float			mx = 0.0f, my = 0.0f;
	int				i, j;

	switch (uMsg) {
	case WM_CREATE:
		hdc = GetDC(hWnd);
		g_hMemDC = CreateCompatibleDC(hdc);		// ����ʂ̍쐬
		g_hBitmap = CreateCompatibleBitmap(hdc, SCREEN_WIDTH, SCREEN_HEIGHT);
		SelectObject(g_hMemDC, g_hBitmap);
		SetBkMode(g_hMemDC, TRANSPARENT);
		SetTextColor(g_hMemDC, RGB(0, 0, 0));
		ReleaseDC(hWnd, hdc);
		return 0;
	case WM_KEYDOWN:			// �L�[�������ꂽ
		switch (wParam) {
		case VK_ESCAPE:	DestroyWindow(hWnd);	return 0;	// �I��
		case 'I':	g_bOnInfo = !g_bOnInfo;		return 0;	// ���\���X�C�b�`�̐؂�ւ�
		case 'W':	g_bOnWind = !g_bOnWind;		return 0;	// ���X�C�b�`�̐؂�ւ�
		}
		break;
	case WM_MOUSEMOVE:			// �}�E�X��������
		if (g_bOnButton) {
			mx = (float)( (short)LOWORD(lParam) - AXIS_X_OFFSET);
			my = (float)(((short)HIWORD(lParam) - AXIS_Y_OFFSET) * -1);
			for (j = 0; j <= PARTICLE_VERTICAL; j++) {
				for (i = 0; i <= PARTICLE_HORIZONTAL; i++) {
					if (g_Cloth.Particle[j][i].onDrag) {
						g_Cloth.Particle[j][i].position.x = mx;		// �h���b�O���̓_������Έړ�
						g_Cloth.Particle[j][i].position.y = my;
					}
				}
			}
		}
		break;
	case WM_LBUTTONDOWN:		// �}�E�X���{�^���������ꂽ
		g_bOnButton = true;
		mx = (float)( (short)LOWORD(lParam) - AXIS_X_OFFSET);
		my = (float)(((short)HIWORD(lParam) - AXIS_Y_OFFSET) * -1);
		for (j = 0; j <= PARTICLE_VERTICAL; j++) {
			for (i = 0; i <= PARTICLE_HORIZONTAL; i++) {
				// �_������͈͓��Ȃ�N���b�N�����Ƃ݂Ȃ�
				if (g_Cloth.Particle[j][i].position.x - 6.0f <= mx && g_Cloth.Particle[j][i].position.x + 6.0f >= mx &&
					g_Cloth.Particle[j][i].position.y - 6.0f <= my && g_Cloth.Particle[j][i].position.y + 6.0f >= my) {
					g_Cloth.Particle[j][i].position.x = mx;
					g_Cloth.Particle[j][i].position.y = my;
					g_Cloth.Particle[j][i].onDrag = true;		// �h���b�O��
				}
			}
		}
		break;
	case WM_LBUTTONUP:			// �}�E�X���{�^���������ꂽ
		g_bOnButton = false;
		for (j = 0; j <= PARTICLE_VERTICAL; j++) {
			for (i = 0; i <= PARTICLE_HORIZONTAL; i++) {
				if (g_Cloth.Particle[j][i].onDrag) g_Cloth.Particle[j][i].onDrag = false;	// �h���b�O�t���O������
			}
		}
		break;
	case WM_PAINT:				// �`��
		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, g_hMemDC, 0, 0, SRCCOPY);	// ����ʂc�b���R�s�[
		EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		DeleteObject(g_hBitmap);
		DeleteDC(g_hMemDC);
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

	ExecGrid();									// �O���b�h�\���������s
	ExecWindForce();							// ���͏������s
	ExecCloth();								// �z�������s
	if (g_bOnInfo) DispInfo();					// ���\��

	InvalidateRect(g_hWndApp, NULL, TRUE);		// �`��v��
}


//---------------------------------------------------------------------------------------
// �O���b�h�\�����W���[��
//---------------------------------------------------------------------------------------
void ExecGrid()
{
	// �O���b�h�̃y�� 
	HPEN PenGrid0 = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
	HPEN PenGrid1 = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));

	// �g��`��
	HPEN hpenOld = (HPEN)SelectObject(g_hMemDC, PenGrid0);
	for (int x = 0; x < SCREEN_WIDTH; x += 20) {
		MoveToEx(g_hMemDC, x, 0, NULL);
		LineTo(g_hMemDC, x, SCREEN_HEIGHT);
	}
	for (int y = 0; y < SCREEN_HEIGHT; y += 20) {
		MoveToEx(g_hMemDC, 0, y, NULL);
		LineTo(g_hMemDC, SCREEN_WIDTH, y);
	}
	SelectObject(g_hMemDC, PenGrid1);
	MoveToEx(g_hMemDC, 0, AXIS_Y_OFFSET, NULL);
	LineTo(g_hMemDC, SCREEN_WIDTH, AXIS_Y_OFFSET);
	MoveToEx(g_hMemDC, AXIS_X_OFFSET, 0, NULL);
	LineTo(g_hMemDC, AXIS_X_OFFSET, SCREEN_HEIGHT);
	SelectObject(g_hMemDC, hpenOld);

	DeleteObject(PenGrid1);
	DeleteObject(PenGrid0);
}

//---------------------------------------------------------------------------------------
// �z���W���[��
//---------------------------------------------------------------------------------------
void ExecWindForce()
{
	switch (g_WindForce.status) {
	case 0:
		g_WindForce.force.x = WIND_FORCE_DEF_X;		// ���͂̏����l��ݒ�
		g_WindForce.force.y = WIND_FORCE_DEF_Y;
		g_WindForce.force.z = WIND_FORCE_DEF_Z;
		g_WindForce.status = 1;
		// THRU
	case 1:
		if (GetAsyncKeyState(VK_UP   ) & 0x8000) g_WindForce.force.y += 1.0f;	// �J�[�\���L�[�ŕ��͂�ς���
		if (GetAsyncKeyState(VK_DOWN ) & 0x8000) g_WindForce.force.y -= 1.0f;
		if (GetAsyncKeyState(VK_LEFT ) & 0x8000) g_WindForce.force.x -= 1.0f;
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000) g_WindForce.force.x += 1.0f;
		if (GetAsyncKeyState(VK_PRIOR) & 0x8000) g_WindForce.force.z -= 1.0f;
		if (GetAsyncKeyState(VK_NEXT ) & 0x8000) g_WindForce.force.z += 1.0f;
		if (GetAsyncKeyState(VK_HOME ) & 0x0001) g_WindForce.status = 0;
		break;
	}
}

//---------------------------------------------------------------------------------------
// �z���W���[��
//---------------------------------------------------------------------------------------
void ExecCloth()
{
	VECTOR	vec_spr;				// ���q�Q�_�Ԃ̃x�N�g��
	VECTOR	resistance;				// ��R�̓x�N�g��
	VECTOR	windforce;				// ���̓x�N�g��
	float	f1, f2;					// �o�l�����q�ɗ^����́i�Q��j
	int		h1, v1, h2, v2;			// �o�l�[�ɂ��闱�q�̃C���f�b�N�X�i���A�c�ʂQ���j
	float	f1x, f1y, f1z;			// ��������o�l�ɔ��������
	float	f2x, f2y, f2z;
	float	length;					// ���q�Ԃ̋���
	float	xx, yy, zz;				// ���q�Ԃ̋����i�������j
	int		count;					// �o�l�J�E���g�p
	int		i, j;					// ���[�v�J�E���^

	switch (g_Cloth.status) {
	case 0:			// ������
		// ���q�̏�����
		for (j = 0; j <= PARTICLE_VERTICAL; j++) {
			for (i = 0; i <= PARTICLE_HORIZONTAL; i++) {
				// ���W��ݒ�
				g_Cloth.Particle[j][i].position.x =  SPRING_DEFAULT_LENGTH * (i - PARTICLE_HORIZONTAL/2);
				g_Cloth.Particle[j][i].position.y = -SPRING_DEFAULT_LENGTH * (j - PARTICLE_VERTICAL/2);
				g_Cloth.Particle[j][i].position.z = 0.0f;
				F_ZERO(g_Cloth.Particle[j][i].velocity);									// ���x������
				F_ZERO(g_Cloth.Particle[j][i].acceleration);								// �����x������
				F_ZERO(g_Cloth.Particle[j][i].resultant);									// ���͏�����
				F_SET(g_Cloth.Particle[j][i].gravity, 0.0f, SPRING_MASS * -9.8f, 0.0f);		// �d�͏�����
				// ����̗��q�̓��b�N��Ԃ�
				g_Cloth.Particle[j][i].onLock = (i == 0 && (j == 0 || j == PARTICLE_VERTICAL));	// ���b�N
				g_Cloth.Particle[j][i].onDrag = false;					// �h���b�O����ĂȂ�
			}
		}

		// �o�l�̏�����
		count = 0;
		for (j = 0; j <= PARTICLE_VERTICAL; j++) {
			for (i = 0; i <= PARTICLE_HORIZONTAL; i++) {
				// �o�l�Q�ƃC���f�b�N�X�̐ݒ�
				if (i < PARTICLE_HORIZONTAL) {
					g_Cloth.Spring[count].p1.horz = i;		// �P�ڂ̎Q�Ɨ��q�C���f�b�N�X��ݒ�
					g_Cloth.Spring[count].p1.vert = j;
					g_Cloth.Spring[count].p2.horz = i+1;	// �Q�ڂ̎Q�Ɨ��q�C���f�b�N�X��ݒ�
					g_Cloth.Spring[count].p2.vert = j;
					// �Q�̗��q�Ԃ̋��������߁A�o�l�̎��R���Ƃ���
					xx = g_Cloth.Particle[j][i].position.x - g_Cloth.Particle[j][i+1].position.x;
					yy = g_Cloth.Particle[j][i].position.y - g_Cloth.Particle[j][i+1].position.y;
					zz = g_Cloth.Particle[j][i].position.z - g_Cloth.Particle[j][i+1].position.z;
					g_Cloth.Spring[count].length = sqrtf(xx*xx + yy*yy + zz*zz);
					count++;
				}
				if (j < PARTICLE_VERTICAL) {
					g_Cloth.Spring[count].p1.horz = i;
					g_Cloth.Spring[count].p1.vert = j;
					g_Cloth.Spring[count].p2.horz = i;
					g_Cloth.Spring[count].p2.vert = j+1;
					xx = g_Cloth.Particle[j][i].position.x - g_Cloth.Particle[j+1][i].position.x;
					yy = g_Cloth.Particle[j][i].position.y - g_Cloth.Particle[j+1][i].position.y;
					zz = g_Cloth.Particle[j][i].position.z - g_Cloth.Particle[j+1][i].position.z;
					g_Cloth.Spring[count].length = sqrtf(xx*xx + yy*yy + zz*zz);
					count++;
				}
				if (i < PARTICLE_HORIZONTAL && j < PARTICLE_VERTICAL) {
					g_Cloth.Spring[count].p1.horz = i;
					g_Cloth.Spring[count].p1.vert = j;
					g_Cloth.Spring[count].p2.horz = i+1;
					g_Cloth.Spring[count].p2.vert = j+1;
					xx = g_Cloth.Particle[j][i].position.x - g_Cloth.Particle[j+1][i+1].position.x;
					yy = g_Cloth.Particle[j][i].position.y - g_Cloth.Particle[j+1][i+1].position.y;
					zz = g_Cloth.Particle[j][i].position.z - g_Cloth.Particle[j+1][i+1].position.z;
					g_Cloth.Spring[count].length = sqrtf(xx*xx + yy*yy + zz*zz);
					count++;
				}
				if (i > 0 && j < PARTICLE_VERTICAL) {
					g_Cloth.Spring[count].p1.horz = i;
					g_Cloth.Spring[count].p1.vert = j;
					g_Cloth.Spring[count].p2.horz = i-1;
					g_Cloth.Spring[count].p2.vert = j+1;
					xx = g_Cloth.Particle[j][i].position.x - g_Cloth.Particle[j+1][i-1].position.x;
					yy = g_Cloth.Particle[j][i].position.y - g_Cloth.Particle[j+1][i-1].position.y;
					zz = g_Cloth.Particle[j][i].position.z - g_Cloth.Particle[j+1][i-1].position.z;
					g_Cloth.Spring[count].length = sqrtf(xx*xx + yy*yy + zz*zz);
					count++;
				}
			}
		}
		g_Cloth.status = 1;
		break;
	case 1:
		// ���q�̏���
		for (j = 0; j <= PARTICLE_VERTICAL; j++) {
			for (i = 0; i <= PARTICLE_HORIZONTAL; i++) {
				// ���͂��O�ɂ���
				g_Cloth.Particle[j][i].resultant.x = 0;
				g_Cloth.Particle[j][i].resultant.y = 0;
				g_Cloth.Particle[j][i].resultant.z = 0;

				if (!g_Cloth.Particle[j][i].onLock && !g_Cloth.Particle[j][i].onDrag) {
					// �d�͂����߂�
					F_SET(g_Cloth.Particle[j][i].gravity, 0.0f, SPRING_MASS * -9.8f, 0.0f);	// �d��

					// ��R�͂����߂�
					resistance.x = -g_Cloth.Particle[j][i].velocity.x * ATT_COEFFICIENT;
					resistance.y = -g_Cloth.Particle[j][i].velocity.y * ATT_COEFFICIENT;
					resistance.z = -g_Cloth.Particle[j][i].velocity.z * ATT_COEFFICIENT;

					// ���͂����߂�
					if (g_bOnWind) {
						windforce.x = g_WindForce.force.x + (float)(rand() % 30 - 15);		// �w����
						windforce.y = g_WindForce.force.y + (float)(rand() % 30 - 15);		// �x����
						windforce.z = g_WindForce.force.z + (float)(rand() % 30 - 15);		// �y����
					} else {
						windforce.x = 0;
						windforce.y = 0;
						windforce.z = 0;
					}

					// ���́��d�́{��R�́{����
					g_Cloth.Particle[j][i].resultant.x = g_Cloth.Particle[j][i].gravity.x + resistance.x + windforce.x;
					g_Cloth.Particle[j][i].resultant.y = g_Cloth.Particle[j][i].gravity.y + resistance.y + windforce.y;
					g_Cloth.Particle[j][i].resultant.z = g_Cloth.Particle[j][i].gravity.z + resistance.z + windforce.z;
				}
			}
		}

		// �o�l�̏���
		for (i = 0; i < SPRING_NUMS; i++) {
			// �o�l�̎Q�Ɨ��q�i�Q�j���擾
			h1 = g_Cloth.Spring[i].p1.horz;
			v1 = g_Cloth.Spring[i].p1.vert;
			h2 = g_Cloth.Spring[i].p2.horz;
			v2 = g_Cloth.Spring[i].p2.vert;
			// �Q�̗��q�Ԃ̃x�N�g�������߂�
			vec_spr.x = g_Cloth.Particle[v1][h1].position.x - g_Cloth.Particle[v2][h2].position.x;
			vec_spr.y = g_Cloth.Particle[v1][h1].position.y - g_Cloth.Particle[v2][h2].position.y;
			vec_spr.z = g_Cloth.Particle[v1][h1].position.z - g_Cloth.Particle[v2][h2].position.z;
			// ���q�Ԃ̋��������߂�
			length = sqrtf(vec_spr.x * vec_spr.x + vec_spr.y * vec_spr.y + vec_spr.z * vec_spr.z);
			// �����A���R���A�o�l�W�����炩����͂����߂�i�Q�ڂ͋t�����j
			f1 = -SPRING_COEFFICIENT * (length - g_Cloth.Spring[i].length);
			f2 = -f1;
			// �x�N�g���̐����ɗ͂�������
			f1x = f1 * (vec_spr.x / length);
			f1y = f1 * (vec_spr.y / length);
			f1z = f1 * (vec_spr.z / length);
			f2x = f2 * (vec_spr.x / length);
			f2y = f2 * (vec_spr.y / length);
			f2z = f2 * (vec_spr.z / length);
			// ���߂��͂����͂ɉ�����
			g_Cloth.Particle[v1][h1].resultant.x += f1x;
			g_Cloth.Particle[v1][h1].resultant.y += f1y;
			g_Cloth.Particle[v1][h1].resultant.z += f1z;
			g_Cloth.Particle[v2][h2].resultant.x += f2x;
			g_Cloth.Particle[v2][h2].resultant.y += f2y;
			g_Cloth.Particle[v2][h2].resultant.z += f2z;
		}

		for (j = 0; j <= PARTICLE_VERTICAL; j++) {
			for (i = 0; i <= PARTICLE_HORIZONTAL; i++) {
				// �Œ肳��Ă���΃X�L�b�v
				if (g_Cloth.Particle[j][i].onLock || g_Cloth.Particle[j][i].onDrag) continue;

				// ���͂Ǝ��ʂ�������x�����߂�
				g_Cloth.Particle[j][i].acceleration.x = g_Cloth.Particle[j][i].resultant.x / SPRING_MASS;
				g_Cloth.Particle[j][i].acceleration.y = g_Cloth.Particle[j][i].resultant.y / SPRING_MASS;
				g_Cloth.Particle[j][i].acceleration.z = g_Cloth.Particle[j][i].resultant.z / SPRING_MASS;

				// ���x�ɉ����x��������
				g_Cloth.Particle[j][i].velocity.x += g_Cloth.Particle[j][i].acceleration.x * DELTA_TIME;
				g_Cloth.Particle[j][i].velocity.y += g_Cloth.Particle[j][i].acceleration.y * DELTA_TIME;
				g_Cloth.Particle[j][i].velocity.z += g_Cloth.Particle[j][i].acceleration.z * DELTA_TIME;

				// ���x������W���ړ�
				g_Cloth.Particle[j][i].position.x += g_Cloth.Particle[j][i].velocity.x * DELTA_TIME;
				g_Cloth.Particle[j][i].position.y += g_Cloth.Particle[j][i].velocity.y * DELTA_TIME;
				g_Cloth.Particle[j][i].position.z += g_Cloth.Particle[j][i].velocity.z * DELTA_TIME;
			}
		}
		break;
	}

	// �z��`��
	HPEN PenCloth = CreatePen(PS_SOLID, 2, RGB(0, 128, 0));
	HPEN hpenOld = (HPEN)SelectObject(g_hMemDC, PenCloth);
	for (i = 0; i < SPRING_NUMS; i++) {
		// �o�l�̎Q�Ɨ��q�i�Q�j���擾
		h1 = g_Cloth.Spring[i].p1.horz;
		v1 = g_Cloth.Spring[i].p1.vert;
		h2 = g_Cloth.Spring[i].p2.horz;
		v2 = g_Cloth.Spring[i].p2.vert;
		// �o�l��`��
		MoveToEx(g_hMemDC, AXIS_X_OFFSET + (int)g_Cloth.Particle[v1][h1].position.x, AXIS_Y_OFFSET - (int)g_Cloth.Particle[v1][h1].position.y, NULL);
		LineTo(g_hMemDC, AXIS_X_OFFSET + (int)g_Cloth.Particle[v2][h2].position.x, AXIS_Y_OFFSET - (int)g_Cloth.Particle[v2][h2].position.y);
	}
	for (j = 0; j <= PARTICLE_VERTICAL; j++) {
		for (i = 0; i <= PARTICLE_HORIZONTAL; i++) {
			// �_��`��
			Ellipse(g_hMemDC,
				AXIS_X_OFFSET + (int)g_Cloth.Particle[j][i].position.x - 4, AXIS_Y_OFFSET - (int)g_Cloth.Particle[j][i].position.y - 4,
				AXIS_X_OFFSET + (int)g_Cloth.Particle[j][i].position.x + 4, AXIS_Y_OFFSET - (int)g_Cloth.Particle[j][i].position.y + 4);
		}
	}
	SelectObject(g_hMemDC, hpenOld);
	DeleteObject(PenCloth);
}

//---------------------------------------------------------------------------------------
// ���\�����W���[��
//---------------------------------------------------------------------------------------
void DispInfo()
{
	TCHAR	str[256];
	int		nLen;

	HFONT hfontOld = (HFONT)SelectObject(g_hMemDC, GetStockObject(SYSTEM_FIXED_FONT));
	// ���͕\��
	nLen = _stprintf(str, _T("Wind (%8.3f,%8.3f,%8.3f)"), g_WindForce.force.x, g_WindForce.force.y, g_WindForce.force.z);
	TextOut(g_hMemDC, 0, 0, str, nLen);
	if (g_bOnWind)	nLen = _stprintf(str, _T("< Wind On  >"));
	else			nLen = _stprintf(str, _T("< Wind Off >"));
	TextOut(g_hMemDC, 0, 20, str, nLen);
	SelectObject(g_hMemDC, hfontOld);
}

//=======================================================================================
//	end
//=======================================================================================
