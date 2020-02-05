//=================================================================================================
//    [ ���}�b�s���O�i�X�t�B�A�j ] OpenGL sample
//    ���}�b�s���O�i�X�t�B�A�j�T���v��
//
//    ����
//    �@�J�[�\���@���ʃ��f����]
//
//=================================================================================================
//-------- �C���N���[�h�w�b�_
#define _CRT_SECURE_NO_WARNINGS
#define STRICT
#include <windows.h>		// Windows�v���O�����ɂ͂����t����
#include <tchar.h>			// �ėp�e�L�X�g �}�b�s���O
#include <GL/gl.h>			// OpenGL �֘A�̃w�b�_�[
#include <GL/glu.h>			// OpenGL �֘A�̃w�b�_�[
#include <stdio.h>			// �����񏈗��Ŏg�p
#include <mmsystem.h>		// ���ԊǗ��Ŏg�p
#define _USE_MATH_DEFINES
#include <math.h>			// �~�����ŕK�v

#include "tga.h"
#include "matrix.h"

//-------- ���C�u�����̃����N�i�����L�q���Ă����Εʓr�����N�ݒ肪�s�v�ƂȂ�j
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "winmm.lib")

//-------- �萔��`
#define CLASS_NAME		_T("Sphere")								// �E�B���h�E�N���X��
#define CAPTION_NAME	_T("[ ���}�b�s���O�i�X�t�B�A�j ������ ]")		// �L���v�V������

const float FRAME_RATE		= 1000/60;								// �t���[�����[�g
const int	SCREEN_WIDTH	= 640;									// �X�N���[���̕�
const int	SCREEN_HEIGHT	= 480;									// �X�N���[���̍���

const int	FONT_WIDTH		= 10;
const int	FONT_HEIGHT		= 16;

//----- ���_�t�H�[�}�b�g��`
struct VERTEX {
	// GL_T2F_N3F_V3F
	float		tu, tv;				// �e�N�X�`�����W
	float		nx, ny, nz;			// �@���x�N�g��
	float		x, y, z;			// ���_���W�i���W�ϊ�����j
};

struct VERTEX2 {
	// GL_T2F_V3F
	float		tu, tv;				// �e�N�X�`�����W
	float		x, y, z;			// ���_���W
};

typedef struct {
	int			status;				// �X�e�[�^�X
	VECTOR		position;			// ���W
	VECTOR		rotation;			// ��]�p
} MIRROR;

//----- ���ʃ��f����`
VERTEX mirror_vertex[] = {
	//  tu    tv     nx     ny       nz        x       y     z
	{ 0.0f, 0.0f, -0.3f,  0.3f, -0.905f, -256.0f, 256.0f, 0.0f},	// ���_�O
	{ 0.0f, 0.0f,  0.3f,  0.3f, -0.905f,  256.0f, 256.0f, 0.0f},	// ���_�P
	{ 0.0f, 0.0f,  0.3f, -0.3f, -0.905f,  256.0f,-256.0f, 0.0f},	// ���_�Q
	{ 0.0f, 0.0f, -0.3f, -0.3f, -0.905f, -256.0f,-256.0f, 0.0f},	// ���_�R
};

//----- �O���[�o���ϐ�

// OpenGL �֘A
HDC						g_hDC;						// �f�o�C�X�R���e�L�X�g
HGLRC					g_hRC;						// �J�����g�R���e�L�X�g
GLuint					g_uFontTexID;				// �t�H���g�e�N�X�`�� ID
GLuint					g_uTextureID;				// �e�N�X�`�� ID
MATRIX					g_MatView;					// �r���[�}�g���b�N�X

MIRROR					g_Mirror;					// ���I�u�W�F�N�g

TCHAR					g_szDebug[4096];			// �o�͕�����o�b�t�@
float					g_FPS;						// �t���[�����J�E���g�p

//----- �v���g�^�C�v�錾
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
// ���C��
//-------------------------------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int iCmdShow)
{
	HWND	hWnd;
	MSG		msg;
	DWORD	dwExecLastTime, dwFPSLastTime, dwCurrentTime, dwFrameCount;
	float	dt;
	bool	bWindow;

	// �E�B���h�E�N���X��o�^
	WNDCLASS wndClass = {
		CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hInst, LoadIcon(hInst, IDI_APPLICATION),
		LoadCursor(NULL, IDC_ARROW), (HBRUSH)GetStockObject(WHITE_BRUSH), NULL, CLASS_NAME
	};
	if (RegisterClass(&wndClass) == 0) return false;

	// �E�C���h�E���쐬
	hWnd = CreateWindow(
		CLASS_NAME, CAPTION_NAME, WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		SCREEN_WIDTH  + GetSystemMetrics(SM_CXFIXEDFRAME) * 2,
		SCREEN_HEIGHT + GetSystemMetrics(SM_CYFIXEDFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION),
		NULL, NULL, hInst, NULL);
	if (hWnd == 0) return false;

	bWindow = false;
	if (IDYES == MessageBox(hWnd, _T("�E�B���h�E���[�h�Ŏ��s���܂����H"), _T("��ʃ��[�h"), MB_YESNO))
		bWindow = true;
	if (FAILED(Initialize3DEnvironment(hWnd, bWindow))) return 0;	// �R�c���̏�����

	// �ϐ�������
	timeBeginPeriod(1);									// �V�X�e���^�C�}�[�̕���\���P�����ɐݒ�
	dwFPSLastTime = dwExecLastTime = timeGetTime();		// ���݂̃V�X�e���^�C�}�[���擾
	dwFrameCount = 0;
	g_Mirror.status = 0;

	// ���C���E�C���h�E���[�v
	msg.message = WM_NULL;
	while (WM_QUIT != msg.message) {									// WM_QUIT ������܂Ń��[�v
		if (PeekMessage(&msg,NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			dwCurrentTime = timeGetTime();								// ���݂̃^�C�}�[�l���擾
			if (dwCurrentTime - dwFPSLastTime >= 500) {					// �O�D�T�b���ƂɌv��
				dt = (float)(dwCurrentTime - dwFPSLastTime) / 1000;		// �t���[�������v�Z
				g_FPS = (float)dwFrameCount / dt;
				dwFPSLastTime = dwCurrentTime;							// �^�C�}�[�l���X�V
				dwFrameCount = 0;										// �t���[���J�E���^�����Z�b�g
			}
			// ���̕ӂŎ��ԊǗ�
			if (dwCurrentTime - dwExecLastTime >= FRAME_RATE) {			// ��莞�Ԃ��o�߂�����E�E�E
				dwExecLastTime = dwCurrentTime;							// �^�C�}�[�l���X�V
				Render3DEnvironment();									// �`�揈��
				dwFrameCount++;											// �t���[���J�E���g���{�P
			}
		}
		Sleep(1);
	}
	timeEndPeriod(1);						// �V�X�e���^�C�}�[�̕���\�����ɖ߂�
	Cleanup3DEnvironment(hWnd, bWindow);
	return msg.wParam;
}

//-------------------------------------------------------------------------------------------------
// ���b�Z�[�W�n���h��
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
// �R�c���̏�����
//-------------------------------------------------------------------------------------------------
HRESULT Initialize3DEnvironment(HWND hWnd, bool bWindow)
{
	// �s�N�Z���t�H�[�}�b�g
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

	// �E�B���h�E�^�t���X�N���[���؂�ւ�
	if (!bWindow) {
		DEVMODE	devmode;
		devmode.dmFields		= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		devmode.dmBitsPerPel	= 32;
		devmode.dmPelsWidth		= SCREEN_WIDTH;
		devmode.dmPelsHeight	= SCREEN_HEIGHT;
		devmode.dmSize			= sizeof devmode;
		if (ChangeDisplaySettings(&devmode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
			MessageBox(hWnd, _T("�t���X�N���[���؂�ւ��Ɏ��s���܂���"),
				_T("error"), MB_OK | MB_ICONERROR);
			return E_FAIL;
		}
		SetWindowLong(hWnd, GWL_STYLE, WS_VISIBLE);
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0,
			SCREEN_WIDTH, SCREEN_HEIGHT, SWP_SHOWWINDOW);
	}

	// �E�B���h�E�R���e�L�X�g�̎擾
	g_hDC = GetDC(hWnd);

	// ���v����ł��߂��s�N�Z���t�H�[�}�b�g�̑I��
	int nPixelFormat = ChoosePixelFormat(g_hDC, &pfd);
	if (!nPixelFormat) {
		MessageBox(hWnd, _T("�s�N�Z���t�H�[�}�b�g�̑I���Ɏ��s���܂���"), _T("error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// �s�N�Z���t�H�[�}�b�g�̐ݒ�
	if (!SetPixelFormat(g_hDC, nPixelFormat, &pfd)) {
		MessageBox(hWnd, _T("�s�N�Z���t�H�[�}�b�g�̐ݒ�Ɏ��s���܂���"), _T("error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// OpenGL�����_�����O�R���e�L�X�g���쐬
	g_hRC = wglCreateContext(g_hDC);
	if (!g_hRC) {
		MessageBox(hWnd, _T("�����_�����O�R���e�L�X�g�̐����Ɏ��s���܂���"), _T("error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// OpenGL�����_�����O�R���e�L�X�g�����C�N
	if (!wglMakeCurrent(g_hDC, g_hRC)) {
		MessageBox(hWnd, _T("�����_�����O�R���e�L�X�g�̃��C�N�Ɏ��s���܂���"), _T("error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// �r���[�|�[�g�ݒ�
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// �t�H���g�Z�b�g�A�b�v
	g_uFontTexID = LoadTexture(hWnd, "font.tga");

	// �e�N�X�`���ǂݍ���
	g_uTextureID = LoadTexture(hWnd, "spheremap.bmp");

	// �����_�����O�X�e�[�g��ݒ�
	glDisable(GL_LIGHTING);					// ���C�g������
	glEnable(GL_TEXTURE_2D);				// �e�N�X�`����L���ɂ���

	return S_OK;
}

//-------------------------------------------------------------------------------------------------
// �R�c���̃N���[���A�b�v
//-------------------------------------------------------------------------------------------------
HRESULT Cleanup3DEnvironment(HWND hWnd, bool bWindow)
{
	ReleaseTexture(&g_uTextureID);		// �e�N�X�`�����
	ReleaseTexture(&g_uFontTexID);		// �t�H���g�p�e�N�X�`�����

	wglMakeCurrent(NULL, NULL);
	if (g_hRC) {						// �J�����g�R���e�L�X�g�����
		wglDeleteContext(g_hRC);
		g_hRC = NULL;
	}
	if (g_hDC) {						// �f�o�C�X�R���e�L�X�g�����
		ReleaseDC(hWnd, g_hDC);
		g_hDC = NULL;
	}
	if (!bWindow)
		ChangeDisplaySettings(NULL, 0);
	return S_OK;
}

//-------------------------------------------------------------------------------------------------
// �R�c���̃����_�����O
//-------------------------------------------------------------------------------------------------
HRESULT Render3DEnvironment()
{
	g_szDebug[0] = _T('\0');		// �f�o�b�O�X�g�����O�o�b�t�@������

	// �o�b�N�o�b�t�@�Ƃy�o�b�t�@���N���A
	glClearColor(0.0f, 0.375f, 0.5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Init3D();
	ExecMirror();					// ���ʃ��f�����s�A�`��

	Init2D();
	DrawInfo();						// �C���t�H���[�V�����\��

	// �o�b�N�o�b�t�@���v���C�}���o�b�t�@�ɃR�s�[
	SwapBuffers(g_hDC);

	return S_OK;
}

//---------------------------------------------------------------------------------------
// 2D�p�����_�����O�X�e�[�g������
//---------------------------------------------------------------------------------------
void Init2D()
{
	// �`��ݒ�
	glDisable(GL_CULL_FACE);	// �J�����O�͍s��Ȃ�

	// ���ߐF(�����F)���\�ɂ���
	glEnable(GL_BLEND);			// ���u�����h��
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// ���ߐF(�����F)�̕�����Z�o�b�t�@�ɏ������܂Ȃ�
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.0);

	// Z�o�b�t�@�[�x�𖳌���
	glDisable(GL_DEPTH_TEST);

	// ���ˉe�}�g���b�N�X���v���W�F�N�V�����}�g���b�N�X�Ƃ��Đݒ�
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-SCREEN_WIDTH/2.0, SCREEN_WIDTH/2,
		-SCREEN_HEIGHT/2.0, SCREEN_HEIGHT/2.0, 0.0, 1.0);

	// �r���[�}�g���b�N�X�ݒ�
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

//---------------------------------------------------------------------------------------
// 3D�p�����_�����O�X�e�[�g������
//---------------------------------------------------------------------------------------
void Init3D()
{
	// �|���S���̕Жʂ����`���L����
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// ���ߐF(�����F)�𖳌��ɂ���
	glDisable(GL_BLEND);

	// ���e�X�g�𖳌��ɂ���
	glDisable(GL_ALPHA_TEST);

	// Z�o�b�t�@�[�x��L����
	glEnable(GL_DEPTH_TEST);

	// �v���W�F�N�V�����}�g���b�N�X�ݒ�
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, (float)SCREEN_WIDTH/SCREEN_HEIGHT, 100.0f, 10000.0f);

	// �r���[�}�g���b�N�X�ݒ�
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, -800.0f,
			  0.0f, 0.0f, 0.0f,
			  0.0f, 1.0f, 0.0f);
	glGetFloatv(GL_MODELVIEW_MATRIX, g_MatView[0]);
}

//-------------------------------------------------------------------------------------------------
// ���ʃI�u�W�F�N�g
//-------------------------------------------------------------------------------------------------
void ExecMirror()
{
	MATRIX	matRotateX, matRotateY, matRotateZ;		// ��]�}�g���b�N�X�i�R��j
	MATRIX	matTrans;								// ���s�ړ��}�g���b�N�X
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
		// �L�[�ɂ�鎋�_�ʒu����ы��ʃ��f���̈ړ�
		if (GetAsyncKeyState(VK_UP     ) & 0x8000) g_Mirror.rotation[0] += 0.02f;		// �J�[�\���i��j
		if (GetAsyncKeyState(VK_DOWN   ) & 0x8000) g_Mirror.rotation[0] -= 0.02f;		// �J�[�\���i���j
		if (GetAsyncKeyState(VK_LEFT   ) & 0x8000) g_Mirror.rotation[1] -= 0.02f;		// �J�[�\���i���j
		if (GetAsyncKeyState(VK_RIGHT  ) & 0x8000) g_Mirror.rotation[1] += 0.02f;		// �J�[�\���i�E�j
		break;
	default:
		break;
	}
	// ���[���h�}�g���b�N�X������
	MatrixRotationX(matRotateX, g_Mirror.rotation[0]);							// �w����]�}�g���b�N�X�𐶐�
	MatrixRotationY(matRotateY, g_Mirror.rotation[1]);							// �x����]�}�g���b�N�X�𐶐�
	MatrixRotationZ(matRotateZ, g_Mirror.rotation[2]);							// �y����]�}�g���b�N�X�𐶐�
	MatrixTranslation(matTrans, g_Mirror.position[0], g_Mirror.position[1], g_Mirror.position[2]);	// ���s�ړ��}�g���b�N�X�𐶐�
	MatrixMultiply(matWorld, matTrans, matRotateZ);
	MatrixMultiply(matWorld, matWorld, matRotateY);
	MatrixMultiply(matWorld, matWorld, matRotateX);

	// ���݂̃��[���h�A�r���[�}�g���b�N�X����r���[��Ԃւ̕ϊ��}�g���b�N�X�𐶐�
	MatrixMultiply(matWV, g_MatView, matWorld);

	//*************************************************************************
	//
	//	�����ŃX�t�B�A�}�b�v�ɂ��e�N�X�`�����W�v�Z�����������Ȃ�
	//
	//*************************************************************************
	for (int i = 0; i < 4; i++) {		// ���ʃ��f���͂S���_
		// ���ʃ��f���̒��_�ɂ���@���x�N�g�����r���[��Ԃɉ�]
		v_norm[0] = mirror_vertex[i].nx;
		v_norm[1] = mirror_vertex[i].ny;
		v_norm[2] = mirror_vertex[i].nz;
		v_norm[3] = 1.0f;

		VectorTransformCoord(v_norm2, matWV, v_norm);

		// �X�t�B�A�}�b�v�̃e�N�X�`�����W�����蓖�Ă�
		mirror_vertex[i].tu = 0.5f + 0.5f * v_norm2[0];
		mirror_vertex[i].tv = 0.5f + 0.5f * v_norm2[1];
	}
	//*************************************************************************

	// �`��
	glMultMatrixf(matWorld[0]);
	glInterleavedArrays(GL_T2F_N3F_V3F, 0, mirror_vertex);						// ���_�t�H�[�}�b�g�ݒ�
	glBindTexture(GL_TEXTURE_2D, g_uTextureID);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

//-------------------------------------------------------------------------------------------------
// �C���t�H���[�V�����\��
//-------------------------------------------------------------------------------------------------
void DrawInfo()
{
	static VERTEX2 m_info_vertex[] = {
		{ 0.0f, 0.0f, (float)SCREEN_WIDTH / 2 - 256.0f, (float)-SCREEN_HEIGHT / 2         , 0.0f},	// ���_�O
		{ 1.0f, 0.0f, (float)SCREEN_WIDTH / 2         , (float)-SCREEN_HEIGHT / 2         , 0.0f},	// ���_�P
		{ 1.0f, 1.0f, (float)SCREEN_WIDTH / 2         , (float)-SCREEN_HEIGHT / 2 + 256.0f, 0.0f},	// ���_�Q
		{ 0.0f, 1.0f, (float)SCREEN_WIDTH / 2 - 256.0f, (float)-SCREEN_HEIGHT / 2 + 256.0f, 0.0f},	// ���_�R
	};

	static float	frame_vertex[2 * 5];
	int			i;
	TCHAR		str[256];

	// �X�t�B�A�}�b�v�@�e�N�X�`���\��
	glBindTexture(GL_TEXTURE_2D, g_uTextureID);
	glInterleavedArrays(GL_T2F_V3F, 0, m_info_vertex);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// �X�t�B�A�}�b�v�@�|�W�V�����\��
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

	// �f�o�b�O�X�g�����O�`��
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
// BMP�Ǎ�
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
		return NULL;	// �������m�ۃG���[
	}
	fp = fopen(pszFName, "rb");
	if (fp == NULL) {
		free(pBmp);
		return NULL;	// �t�@�C����������Ȃ�
	}
	if (fread(&bmfh, sizeof(bmfh), 1, fp) != 1) {
		fclose(fp);
		free(pBmp);
		return NULL;	// �t�@�C�����ǂݍ��߂Ȃ�
	}
	if (bmfh.bfType != 'MB') {
		fclose(fp);
		free(pBmp);
		return NULL;	// �r�b�g�}�b�v�`���łȂ�
	}
	if (fread(&bmih, sizeof(bmih), 1, fp) != 1) {
		fclose(fp);
		free(pBmp);
		return NULL;	// �t�@�C�����ǂݍ��߂Ȃ�
	}
	if (bmih.biBitCount != 24 && bmih.biBitCount != 32) {
		fclose(fp);
		free(pBmp);
		return NULL;	// �Ή��`���łȂ�
	}
	if (bmih.biCompression != BI_RGB) {
		fclose(fp);
		free(pBmp);
		return NULL;	// �Ή��`���łȂ�
	}
	uSize = bmfh.bfSize - bmfh.bfOffBits;
	pBmp->data = (unsigned char *)malloc(uSize);
	if (pBmp->data == NULL) {
		fclose(fp);
		free(pBmp);
		return NULL;	// �������m�ۃG���[
	}
	fseek(fp, bmfh.bfOffBits, SEEK_SET);
	if (fread(pBmp->data, uSize, 1, fp) != 1) {
		fclose(fp);
		free(pBmp->data);
		free(pBmp);
		return NULL;	// �t�@�C�����ǂݍ��߂Ȃ�
	}
	pBmp->alphaDepth = 0;
	pBmp->pixelDepth = bmih.biBitCount;
	pBmp->sizeX = bmih.biWidth;
	pBmp->sizeY = bmih.biHeight;
	pBmp->type = bmih.biBitCount / 8;
	// BGR��RGB�ɕ��ъ���
	for (pBGR = pBmp->data; pBGR + pBmp->type - 1 < pBmp->data + uSize; pBGR += pBmp->type) {
		bTmp = pBGR[0];
		pBGR[0] = pBGR[2];
		pBGR[2] = bTmp;
	}
	fclose(fp);
	return pBmp;
}

//---------------------------------------------------------------------------------------
// �e�N�X�`�����[�h
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
		_stprintf(szMsg, _T("ERROR(%hs):�e�N�X�`���̓ǂݍ��݂Ɏ��s���܂����B"), fname);
		MessageBox(hWnd, szMsg, _T("LoadTexture"), MB_OK | MB_ICONSTOP);
		return 0;
	}

	if (bIsTGA) {
		if (pTextureImage->type != 3 && pTextureImage->type != 4) {
			if (pTextureImage->data)
				free(pTextureImage->data);
			free(pTextureImage);
			_stprintf(szMsg, _T("ERROR(%hs):�T�|�[�g���Ă��Ȃ��`���ł��B"), fname);
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
// �e�N�X�`�����
//---------------------------------------------------------------------------------------
void ReleaseTexture(GLuint* puTextureID)
{
	if (puTextureID && *puTextureID) {
		glDeleteTextures(1, puTextureID);	// �e�N�X�`���I�u�W�F�N�g�����
		*puTextureID = 0;
	}
}

//---------------------------------------------------------------------------------------
// 1�����`��
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

	// �r���[�}�g���b�N�X�ݒ�
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBindTexture(GL_TEXTURE_2D, g_uFontTexID);
	glInterleavedArrays(GL_T2F_V3F, 0, vtxQuad);
	glDrawArrays(GL_QUADS, 0, 4);
}

//---------------------------------------------------------------------------------------
// ������`��
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
