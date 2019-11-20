//=================================================================================================
//
//	  Alpha Blend Test Program
//
//=================================================================================================

//-------- �C���N���[�h�w�b�_
#define STRICT				// Windows�v���O�����ɑ΂���u�^�ۏ؁v������
#include <windows.h>		// Windows�v���O�����ɂ͂����t����
#include <tchar.h>			// �ėp�e�L�X�g �}�b�s���O
#include <stdio.h>

//-------- �萔��`
#define CLASS_NAME		_T("AlphaBlend")					// �E�B���h�E�N���X��
#define CAPTION_NAME	_T("Test Program")	// �L���v�V������
#define SCREEN_WIDTH	640									// �X�N���[�����r�b�g�}�b�v��
#define SCREEN_HEIGHT	480									// �X�N���[�����r�b�g�}�b�v����
#define ALPHA_VALUE		0.5									// �A���t�@�l
#define TILE_MAX		100									// �^�C������

//-------- �\���̒�`
typedef union {				// �s�N�Z���\��
	DWORD	rgba;
	BYTE	color[4];
} PIXEL;

typedef struct {			// �^�C���I�u�W�F�N�g
	int		status;				// �X�e�[�^�X
	int		px, py;				// ���W
	int		vx, vy;				// ���x
	int		w, h;				// �T�C�Y
	BYTE	r;				// �J���[
	BYTE	g;				// �J���[
	BYTE	b;				// �J���[
} OBJ_TILE;

//-------- �O���[�o���ϐ���`
HDC					g_hMemDC;				// �������c�b
HBITMAP				g_hMemBitmap;			// �������r�b�g�}�b�v�n���h��
bool				g_bDrawEnable;			// ��ʕ`�拖�t���O
DWORD				*g_pBitmap;				// �r�b�g�}�b�v�{�̂ւ̃|�C���^
int					g_LineLength;			// �P�s������̃o�C�g��
OBJ_TILE			g_ObjTile[TILE_MAX];	// �^�C���I�u�W�F�N�g

//-------------------------------------------------------------------------------------------------
// �v���g�^�C�v�錾
//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ExecTileObject();

//-------------------------------------------------------------------------------------------------
//	���C��
//-------------------------------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int iCmdShow)
{
	HWND	hWnd;		// �E�B���h�E�n���h��
	MSG		msg;		// ���b�Z�[�W���

	g_bDrawEnable = false;
	srand(GetTickCount());

	// �E�B���h�E�N���X�̓o�^
	WNDCLASSEX wc = {
		sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WindowProc,
		0, 0, hInstance, LoadIcon(hInstance, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)GetStockObject(WHITE_BRUSH), NULL, CLASS_NAME, NULL};
	if (RegisterClassEx(&wc) == 0) return false;	// �E�B���h�E�N���X��o�^

	// �E�B���h�E���쐬
	hWnd = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW, CLASS_NAME, CAPTION_NAME, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION,
		CW_USEDEFAULT, CW_USEDEFAULT,
		SCREEN_WIDTH  + (GetSystemMetrics(SM_CXFIXEDFRAME) + GetSystemMetrics(SM_CXEDGE)) * 2,
		SCREEN_HEIGHT + (GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYEDGE)) * 2 + GetSystemMetrics(SM_CYCAPTION),
		NULL, NULL, hInstance, NULL);
	if (hWnd == NULL) return false;
	ShowWindow(hWnd, iCmdShow);					// �E�B���h�E�̕\����Ԃ�ݒ�
	UpdateWindow(hWnd);							// �E�B���h�E�̃N���C�A���g�̈���X�V

	// ���b�Z�[�W���[�v
	msg.message = WM_NULL;
	while (WM_QUIT != msg.message) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {	// ���b�Z�[�W���`�F�b�N
			TranslateMessage(&msg);						// ���z�L�[���b�Z�[�W�𕶎����b�Z�[�W�֕ϊ�
			DispatchMessage(&msg);						// �E�B���h�E�v���V�[�W���փ��b�Z�[�W�𑗏o
		} else if (g_bDrawEnable) {
			ExecTileObject();							// �^�C���ړ�����������������
			InvalidateRect(hWnd, NULL, FALSE);			// ��ʂ̍ĕ`��
		}
		Sleep(1);
	}
	return msg.wParam;
}

//-------------------------------------------------------------------------------------------------
//	�E�B���h�E�v���V�[�W��
//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC			hdc;
	PAINTSTRUCT	ps;
	UINT		i;
	LPBYTE		biBits;
	BITMAP		bitmap;	// �������r�b�g�}�b�v
	static BITMAPINFOHEADER	bi;
	static HBITMAP	hbmOld;


	switch (uMsg) {						// ���b�Z�[�W���`�F�b�N
	case WM_CREATE:
		hdc = GetDC(hWnd);
		ZeroMemory(&bi, sizeof(bi));
		bi.biSize        = sizeof(bi);
		bi.biWidth       = SCREEN_WIDTH;
		bi.biHeight      = SCREEN_HEIGHT;
		bi.biPlanes      = 1;
		bi.biBitCount    = 32;
		bi.biCompression = BI_RGB;
		g_hMemBitmap = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, (VOID**)&biBits, NULL, 0);
		g_hMemDC     = CreateCompatibleDC(hdc);
		hbmOld = (HBITMAP)SelectObject(g_hMemDC, g_hMemBitmap);
		ReleaseDC(hWnd, hdc);

		// �r�b�g�}�b�v�����擾
		GetObject(g_hMemBitmap, sizeof(BITMAP), &bitmap);
		g_pBitmap = (DWORD*)bitmap.bmBits;				// �s�N�Z���̐擪�A�h���X���擾
		g_LineLength = bitmap.bmWidthBytes / 4;			// �P�s������̃s�N�Z���������߂�

		PatBlt(g_hMemDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x00000000);	// ���œh��Ԃ�

		for (i = 0; i < TILE_MAX; i++) g_ObjTile[i].status = 0;		// �^�C���I�u�W�F�N�g�̃X�e�[�^�X��������

		g_bDrawEnable = true;
		return 0;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return 0;
	case WM_DESTROY:							// �I���w��������
		g_bDrawEnable = false;
		SelectObject(g_hMemDC, hbmOld);
		DeleteObject(g_hMemBitmap);
		DeleteDC(g_hMemDC);
		PostQuitMessage(0);							// �V�X�e���ɃX���b�h�̏I����v��
		return 0;
	case WM_KEYDOWN:							// �L�[�{�[�h�������ꂽ
		switch (wParam) {							// �p�����[�^���`�F�b�N
		case VK_ESCAPE:								// [ESC]�L�[�������ꂽ
			DestroyWindow(hWnd);					// �E�B���h�E��j������	
			return 0;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, g_hMemDC, 0, 0, SRCCOPY);		// �������c�b��`��
		EndPaint(hWnd, &ps);
		return 0;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);	// ����E�B���h�E�v���V�[�W�����ďo
}

//-------------------------------------------------------------------------------------------------
// �^�C���I�u�W�F�N�g
//-------------------------------------------------------------------------------------------------
void ExecTileObject()
{
	int			i, x, y;

	// ���œh��Ԃ�
	PatBlt(g_hMemDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x00000000);
	
	for (i = 0; i < TILE_MAX; i++) {
		switch (g_ObjTile[i].status) {
		case 0:
			g_ObjTile[i].w  = rand() % 128 + 64;
			g_ObjTile[i].h  = rand() % 128 + 64;
			g_ObjTile[i].px = rand() % (SCREEN_WIDTH  - g_ObjTile[i].w);
			g_ObjTile[i].py = rand() % (SCREEN_HEIGHT - g_ObjTile[i].h);
			g_ObjTile[i].vx = rand() % 11 - 5;
			g_ObjTile[i].vy = rand() % 11 - 5;
			g_ObjTile[i].r = 16;
			g_ObjTile[i].g = 16;
			g_ObjTile[i].b = 16;
			g_ObjTile[i].status = 1;
			// THRU
		case 1:
			g_ObjTile[i].px += g_ObjTile[i].vx;
			g_ObjTile[i].py += g_ObjTile[i].vy;
			if (g_ObjTile[i].px < 0 || g_ObjTile[i].px >= (int)(SCREEN_WIDTH  - g_ObjTile[i].w)) {
				g_ObjTile[i].vx *= -1;
				g_ObjTile[i].px += g_ObjTile[i].vx;
			}
			if (g_ObjTile[i].py < 0 || g_ObjTile[i].py >= (int)(SCREEN_HEIGHT - g_ObjTile[i].h)) {
				g_ObjTile[i].vy *= -1;
				g_ObjTile[i].py += g_ObjTile[i].vy;
			}
			break;
		}

		// �s�N�Z���������[�v
		for (y = 0; y < g_ObjTile[i].h; y++) {
			for (x = 0; x < g_ObjTile[i].w; x++) {
				// �^�C���̍��W����擪�s�N�Z���̈ʒu�����߂�
				DWORD* buf = g_pBitmap + ((g_ObjTile[i].py + y) * g_LineLength + (g_ObjTile[i].px + x));

				PIXEL tmp;
				tmp.rgba = *buf;

				float r1 = tmp.color[2];
				float g1 = tmp.color[1];
				float b1 = tmp.color[0];

				float r2 = g_ObjTile[i].r;
				float g2 = g_ObjTile[i].g;
				float b2 = g_ObjTile[i].b;
					
				{
					float r3 = r1 + r2;
					float g3 = g1 + g2;
					float b3 = b1 + b2;

					if (r3 > 255.0f) r3 = 255.0f;
					if (g3 > 255.0f) g3 = 255.0f;
					if (b3 > 255.0f) b3 = 255.0f;

					tmp.color[2] = (BYTE)r3;
					tmp.color[1] = (BYTE)g3;
					tmp.color[0] = (BYTE)b3;
					tmp.color[3] = 0x00;
				}
				*buf = tmp.rgba;
			}
		}
	}
}

//=================================================================================================
//	end
//=================================================================================================
