//=================================================================================================
//    [ ���_�u�����f�B���O ] DirectX Graphics 3D sample
//    ���_�u�����f�B���O�T���v��
//
//    ����
//    �@�v�@�@�@�@���C���[�t���[���`��X�C�b�`
//    �@�X�y�[�X�@���[�V�����X�C�b�`
//
//=================================================================================================
//-------- �C���N���[�h�w�b�_
#define _CRT_SECURE_NO_WARNINGS
#define STRICT
#include <windows.h>		// Windows�v���O�����ɂ͂����t����
#include <tchar.h>			// �ėp�e�L�X�g �}�b�s���O
#include <d3d9.h>			// DirectX Graphics �֘A�̃w�b�_�[
#include <d3dx9.h>			// DirectX Graphics �֘A�̃w�b�_�[
#include <stdio.h>			// �����񏈗��Ŏg�p
#include <mmsystem.h>		// ���ԊǗ��Ŏg�p

//-------- ���C�u�����̃����N�i�����L�q���Ă����Εʓr�����N�ݒ肪�s�v�ƂȂ�j
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")

//-------- �}�N��
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x)	if(x){ x->Release(); x=NULL; }
#endif

//-------- �萔��`
#define CLASS_NAME		_T("VertexBlending")			// �E�B���h�E�N���X��
#define CAPTION_NAME	_T("[ ���_�u�����f�B���O ]")	// �L���v�V������

const float FRAME_RATE		= 1000.0f/60.0f;			// �t���[�����[�g
const int	SCREEN_WIDTH	= 640;						// �X�N���[���̕�
const int	SCREEN_HEIGHT	= 480;						// �X�N���[���̍���
const float	RAD				= D3DX_PI/180.0f;			// ���W�A��

//----- ���_�t�H�[�}�b�g��`
struct VERTEX {
	float		x, y, z;			// ���_���W�i���W�ϊ�����j
	float		blend;				// �u�����h�l
	D3DCOLOR	color;				// �f�B�t���[�Y�F
};
#define FVF_VERTEX (D3DFVF_XYZB1 | D3DFVF_DIFFUSE)

typedef struct {
	int			status;		// �X�e�[�^�X
	D3DXVECTOR3	position;	// ���W
	D3DXVECTOR3	rotation;	// ��]�p
	float		degree;		// �p�x�ω��T�C���J�[�u�p�p�x
} PRISM;

//----- �v���Y���ʃ��f����`
//�@�i���Ӂj�@���x�N�g���A�e�N�X�`�����W�͌v�Z�ŋ��߂�
VERTEX prism_vertex0[] = {
	//      x        y        z  blend  color
	{ -400.0f, -100.0f,    0.0f,  0.0f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// ���_�O
	{ -400.0f,  100.0f,    0.0f,  0.0f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// ���_�O
	{ -300.0f, -100.0f,    0.0f,  0.03f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// ���_�O
	{ -300.0f,  100.0f,    0.0f,  0.03f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// ���_�O
	{ -200.0f, -100.0f,    0.0f,  0.12f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// ���_�O
	{ -200.0f,  100.0f,    0.0f,  0.12f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// ���_�O
	{ -100.0f, -100.0f,    0.0f,  0.3f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// ���_�O
	{ -100.0f,  100.0f,    0.0f,  0.3f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// ���_�O
	{    0.0f, -100.0f,    0.0f,  0.5f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// ���_�O
	{    0.0f,  100.0f,    0.0f,  0.5f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// ���_�O
	{  100.0f, -100.0f,    0.0f,  0.7f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// ���_�O
	{  100.0f,  100.0f,    0.0f,  0.7f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// ���_�O
	{  200.0f, -100.0f,    0.0f,  0.88f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// ���_�O
	{  200.0f,  100.0f,    0.0f,  0.88f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// ���_�O
	{  300.0f, -100.0f,    0.0f,  0.97f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// ���_�O
	{  300.0f,  100.0f,    0.0f,  0.97f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// ���_�O
	{  400.0f, -100.0f,    0.0f,  1.0f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// ���_�O
	{  400.0f,  100.0f,    0.0f,  1.0f, D3DCOLOR_XRGB(0xff, 0xff, 0xff)},	// ���_�O
};

VERTEX prism_vertex1[] = {
	//      x        y        z  blend  color
	{ -400.0f, -100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// ���_�O
	{ -400.0f,  100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// ���_�O
	{ -300.0f, -100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// ���_�O
	{ -300.0f,  100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// ���_�O
	{ -200.0f, -100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// ���_�O
	{ -200.0f,  100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// ���_�O
	{ -100.0f, -100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// ���_�O
	{ -100.0f,  100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// ���_�O
	{    0.0f, -100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// ���_�O
	{    0.0f,  100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// ���_�O
	{  100.0f, -100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// ���_�O
	{  100.0f,  100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// ���_�O
	{  200.0f, -100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// ���_�O
	{  200.0f,  100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// ���_�O
	{  300.0f, -100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// ���_�O
	{  300.0f,  100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// ���_�O
	{  400.0f, -100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// ���_�O
	{  400.0f,  100.0f,    0.0f,  0.00f, D3DCOLOR_XRGB(0xff, 0x00, 0x00)},	// ���_�O
};

VERTEX prism_vertex2[] = {
	//      x        y        z  blend  color
	{ -400.0f, -100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// ���_�O
	{ -400.0f,  100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// ���_�O
	{ -300.0f, -100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// ���_�O
	{ -300.0f,  100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// ���_�O
	{ -200.0f, -100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// ���_�O
	{ -200.0f,  100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// ���_�O
	{ -100.0f, -100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// ���_�O
	{ -100.0f,  100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// ���_�O
	{    0.0f, -100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// ���_�O
	{    0.0f,  100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// ���_�O
	{  100.0f, -100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// ���_�O
	{  100.0f,  100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// ���_�O
	{  200.0f, -100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// ���_�O
	{  200.0f,  100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// ���_�O
	{  300.0f, -100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// ���_�O
	{  300.0f,  100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// ���_�O
	{  400.0f, -100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// ���_�O
	{  400.0f,  100.0f,    0.0f,  1.00f, D3DCOLOR_XRGB(0x00, 0xff, 0x00)},	// ���_�O
};

//----- ���_���
D3DXVECTOR3 view_pos(0, 0, -800);		// ���_�ʒu
D3DXVECTOR3 view_ref(0, 0,    0);		// �����ʒu
D3DXVECTOR3 view_up (0, 1,    0);		// �A�b�v�x�N�g��

//----- �O���[�o���ϐ�

// DirectX �֘A
LPDIRECT3D9				g_pD3D;				// Direct3D9 �I�u�W�F�N�g
LPDIRECT3DDEVICE9		g_pD3DDevice;		// Direct3DDevice9 �I�u�W�F�N�g
D3DPRESENT_PARAMETERS	g_D3Dpp;			// �v���[���e�[�V�����p�����[�^
LPD3DXFONT				g_pD3DXFont;		// D3DXFont �I�u�W�F�N�g
D3DXMATRIX				g_MatView;			// �r���[�}�g���b�N�X
D3DXMATRIX				g_MatProj;			// �v���W�F�N�V�����}�g���b�N�X

PRISM					g_Prism;			// �v���Y���I�u�W�F�N�g

TCHAR					g_szDebug[4096];	// �o�͕�����o�b�t�@
float					g_FPS;				// �t���[�����J�E���g�p
bool					g_Wireframe;		// ���C���[�t���[���`��X�C�b�`
bool					g_OnMove;			// ���[�V�����X�C�b�`

//----- �v���g�^�C�v�錾
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
long Initialize3DEnvironment(HWND hWnd, bool bWindow);
HRESULT Cleanup3DEnvironment(HWND hWnd);
HRESULT Render3DEnvironment();
void ExecBG();
void ExecPrism();
void DrawInfo();

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
	g_Prism.status = 0;
	g_Wireframe = false;
	g_OnMove = true;

	// ���C���E�C���h�E���[�v
    msg.message = WM_NULL;
	while (WM_QUIT != msg.message) {								// WM_QUIT ������܂Ń��[�v
		if (PeekMessage(&msg,NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			dwCurrentTime = timeGetTime();								// ���݂̃^�C�}�[�l���擾
			if (dwCurrentTime - dwFPSLastTime >= 500) {					// �O�D�T�b���ƂɌv��
				dt = (float)(dwCurrentTime - dwFPSLastTime) / 1000.0f;	// �t���[�������v�Z
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
	Cleanup3DEnvironment(hWnd);
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
		case VK_ESCAPE:	DestroyWindow(hWnd);	return 0;
		case 'W':		g_Wireframe = !g_Wireframe;	return 0;	// ���C���[�t���[���̐؂�ւ�
		case VK_SPACE:	g_OnMove = !g_OnMove;	return 0;		// ���[�V�����̐؂�ւ�
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
	// �f�o�C�X���e�[�u��
	struct TCreateDevice {
		D3DDEVTYPE type;			// �f�o�C�X�̎��
		DWORD      behavior;		// �f�o�C�X�̓���
	};
	const int c_nMaxDevice = 3;
	const TCreateDevice device[c_nMaxDevice] = {
	    { D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING },	// T&L HAL
	    { D3DDEVTYPE_HAL, D3DCREATE_SOFTWARE_VERTEXPROCESSING },	// HAL
	    { D3DDEVTYPE_REF, D3DCREATE_SOFTWARE_VERTEXPROCESSING },	// REF
	};
	int			nDev;				// �f�o�C�X��ʃJ�E���^

	// Direct3D�I�u�W�F�N�g�𐶐�
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION))) {
		MessageBox(hWnd, _T("Direct3D�I�u�W�F�N�g�̍쐬�Ɏ��s���܂���"), _T("error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// Direct3D �������p�����[�^�̐ݒ�
	ZeroMemory(&g_D3Dpp, sizeof(g_D3Dpp));
	if (bWindow) {
		// �E�B���h�E���[�h
		g_D3Dpp.Windowed			= TRUE;
		g_D3Dpp.BackBufferFormat	= D3DFMT_UNKNOWN;			// �o�b�N�o�b�t�@�t�H�[�}�b�g
	} else {
		// �t���X�N���[�����[�h
		ShowCursor(FALSE);										// �}�E�X�J�[�\����\�����Ȃ�
		g_D3Dpp.Windowed			= FALSE;
		g_D3Dpp.BackBufferFormat	= D3DFMT_X8R8G8B8;			// �o�b�N�o�b�t�@�t�H�[�}�b�g
	}
	g_D3Dpp.BackBufferWidth				= SCREEN_WIDTH;						// �Q�[����ʃT�C�Y
	g_D3Dpp.BackBufferHeight			= SCREEN_HEIGHT;
	g_D3Dpp.SwapEffect					= D3DSWAPEFFECT_DISCARD;			// �o�b�t�@�X���b�v����
	g_D3Dpp.EnableAutoDepthStencil		= TRUE;								// �y�o�b�t�@�L��
	g_D3Dpp.AutoDepthStencilFormat		= D3DFMT_D16;						// �y�o�b�t�@�t�H�[�}�b�g
	g_D3Dpp.FullScreen_RefreshRateInHz	= D3DPRESENT_RATE_DEFAULT;
	g_D3Dpp.PresentationInterval		= D3DPRESENT_INTERVAL_IMMEDIATE;	// VSYNC��҂��Ȃ�

	// �f�o�C�X�I�u�W�F�N�g���쐬
	for (nDev = 0; nDev < c_nMaxDevice; nDev++) {
		if (SUCCEEDED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, device[nDev].type, hWnd,	// �f�o�C�X���쐬
					device[nDev].behavior, &g_D3Dpp, &g_pD3DDevice))) break;			// �쐬�ł����� break
	}
	if (nDev >= c_nMaxDevice) {		// ���[�v����肫���Ă��܂�����f�o�C�X�쐬�͏o���Ȃ������Ɣ��f
		MessageBox(hWnd, _T("�f�o�C�X�̍쐬�Ɏ��s���܂���"), _T("error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// �t�H���g�Z�b�g�A�b�v
	D3DXCreateFont(g_pD3DDevice, 18, 0, 0, 0, FALSE, SHIFTJIS_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Terminal"), &g_pD3DXFont);

	// �r���[�}�g���b�N�X�ݒ�
	D3DXMatrixLookAtLH(&g_MatView, &view_pos, &view_ref, &view_up);
	g_pD3DDevice->SetTransform(D3DTS_VIEW, &g_MatView);		// �r���[�}�g���b�N�X���Z�b�g

	// �v���W�F�N�V�����}�g���b�N�X�ݒ�
	D3DXMatrixPerspectiveFovLH(&g_MatProj, 60.0f * D3DX_PI/180.0f, (float)SCREEN_WIDTH/SCREEN_HEIGHT, 100.0f, 10000.0f);
	g_pD3DDevice->SetTransform(D3DTS_PROJECTION, &g_MatProj);	// �v���W�F�N�V�����}�g���b�N�X���Z�b�g

	// �����_�����O�X�e�[�g��ݒ�
	g_pD3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE);					// �y�o�b�t�@������
	g_pD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);				// ���C�g������
	g_pD3DDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS);	// �E�F�C�g��ݒ�

	return S_OK;
}

//-------------------------------------------------------------------------------------------------
// �R�c���̃N���[���A�b�v
//-------------------------------------------------------------------------------------------------
HRESULT Cleanup3DEnvironment(HWND hWnd)
{
	SAFE_RELEASE(g_pD3DXFont);		// D3DXFont���
	SAFE_RELEASE(g_pD3DDevice);		// Direct3DDevice �I�u�W�F�N�g���
	SAFE_RELEASE(g_pD3D);			// Direct3D �I�u�W�F�N�g���
	return S_OK;
}

//-------------------------------------------------------------------------------------------------
// �R�c���̃����_�����O
//-------------------------------------------------------------------------------------------------
HRESULT Render3DEnvironment()
{
	g_szDebug[0] = _T('\0');		// �f�o�b�O�X�g�����O�o�b�t�@������

	// �o�b�N�o�b�t�@�Ƃy�o�b�t�@���N���A
	g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0x00, 0x60, 0x80), 1.0f, 0);

	if (SUCCEEDED(g_pD3DDevice->BeginScene())) {	// �V�[���J�n
		ExecPrism();					// �v���Y�����f�����s�A�`��
		DrawInfo();						// �C���t�H���[�V�����\��
		g_pD3DDevice->EndScene();		// �`��I��
	}

	// �o�b�N�o�b�t�@���v���C�}���o�b�t�@�ɃR�s�[
	if (FAILED(g_pD3DDevice->Present(NULL,NULL,NULL,NULL))) g_pD3DDevice->Reset(&g_D3Dpp);

	return S_OK;
}

//-------------------------------------------------------------------------------------------------
// �v���Y���I�u�W�F�N�g
//-------------------------------------------------------------------------------------------------
void ExecPrism()
{
	D3DXMATRIX	matRotateX;				// �w����]�}�g���b�N�X
	D3DXMATRIX	matRotateY;				// �x����]�}�g���b�N�X
	D3DXMATRIX	matRotateZ;				// �y����]�}�g���b�N�X
	D3DXMATRIX	matTrans;				// ���s�ړ��}�g���b�N�X
	D3DXMATRIX	matWorld;				// ���[���h�ϊ��}�g���b�N�X
	D3DXMATRIX	matWorld2;				// ���[���h�ϊ��}�g���b�N�X

	switch (g_Prism.status) {
	case 0:
		// ���f���p�x�A�ʒu�̏�����
		g_Prism.rotation.x = g_Prism.rotation.y = g_Prism.rotation.z = 0.0f;
		g_Prism.position.x = g_Prism.position.y = g_Prism.position.z = 0.0f;
		g_Prism.degree = 0.0f;
		g_Prism.status = 1;
		// THRU
	case 1:
		// �����I�ɉ�]
		if (g_OnMove) {
			g_Prism.rotation.z = (float)sin(g_Prism.degree);
			g_Prism.degree += 0.01f;
		}
		break;
	default:
		break;
	}
	// ���[���h�}�g���b�N�X������
	D3DXMatrixRotationX(&matRotateX, g_Prism.rotation.x);	// �w����]�}�g���b�N�X�𐶐�
	D3DXMatrixRotationY(&matRotateY, g_Prism.rotation.y);	// �x����]�}�g���b�N�X�𐶐�
	D3DXMatrixRotationZ(&matRotateZ, g_Prism.rotation.z);	// �y����]�}�g���b�N�X�𐶐�
	D3DXMatrixTranslation(&matTrans,
		g_Prism.position.x, g_Prism.position.y, g_Prism.position.z);	// ���s�ړ��}�g���b�N�X�𐶐�
	matWorld = matRotateX * matRotateY * matRotateZ * matTrans;			// ���[���h�}�g���b�N�X������

	if (g_Wireframe) {
		g_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);	// ���C���[�t���[���`��
	} else {
		g_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);		// �\���b�h�`��
	}

	D3DXMatrixIdentity(&matWorld2);
	g_pD3DDevice->SetFVF(FVF_VERTEX);									// ���_�t�H�[�}�b�g�ݒ�
	g_pD3DDevice->SetTransform(D3DTS_WORLDMATRIX(0), &matWorld);		// ���[���h�}�g���b�N�X���Z�b�g
	g_pD3DDevice->SetTransform(D3DTS_WORLDMATRIX(1), &matWorld2);		// ���[���h�}�g���b�N�X���Z�b�g

	// �}�g���b�N�X�P�ŕ`��
	g_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 16, &prism_vertex1[0], sizeof(VERTEX));

	// �u�����f�B���O���f����`��
	g_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 16, &prism_vertex2[0], sizeof(VERTEX));

	// �}�g���b�N�X�O�ŕ`��
	g_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 16, &prism_vertex0[0], sizeof(VERTEX));

}

//-------------------------------------------------------------------------------------------------
// �C���t�H���[�V�����\��
//-------------------------------------------------------------------------------------------------
void DrawInfo()
{
//	int			i;
	TCHAR		str[256];
	static RECT	rcStr = {0, 0, 640, 480};

	// �f�o�b�O�X�g�����O�`��
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
