#define _CRT_SECURE_NO_WARNINGS
//=================================================================================================
//    [ Shadow ] DirectX Graphics 3D sample
//    �V���h�E�{�����[���T���v��
//
//    ����
//    �@�J�[�\��		���_�ړ�
//
//=================================================================================================
//-------- �C���N���[�h�w�b�_
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


//----- ���f���A�e�N�X�`���p�t�@�C�����f�[�^
LPCTSTR	modelfile[]   = {_T("sozai/land.x")  , _T("sozai/donuts.x")  , };
LPCTSTR	texturefile[] = {_T("sozai/land.bmp"), _T("sozai/donuts.bmp"), };
#define MODEL_QTY	2		// ���f������
enum MODEL_SERIAL {
	MN_LAND = 0,
	MN_DONUTS,
};

//-------- �萔��`
#define CLASS_NAME		_T("Shadow")							// �E�B���h�E�N���X��
#define CAPTION_NAME	_T("[ Shadow ] DirectX Sample Program")	// �L���v�V������

const float FRAME_RATE		= 1000.0f/60.0f;					// �t���[�����[�g
const int	SCREEN_WIDTH	= 640;								// �X�N���[���̕�
const int	SCREEN_HEIGHT	= 480;								// �X�N���[���̍���
const float	RAD				= D3DX_PI/180.0f;					// ���W�A��
const int	DONUTS_QTY		= 16;								// �h�[�i�c�̌�
const int	LIGHT_DETAIL	= 32;								// �_�����̃f�B�e�[�����x��

//----- ���_�t�H�[�}�b�g��`
struct VERTEX {
	float		x, y, z;			// ���_���W�i���W�ϊ�����j
	float		nx, ny, nz;			// �@���x�N�g��
	float		tu, tv;				// �e�N�X�`�����W
};
#define FVF_VERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)

struct TLVERTEX {
	float		x, y, z;			// ���_���W�i���W�ϊ�����j
	DWORD		diffuse;			// �f�B�t���[�Y
};
#define	FVF_TLVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE)

//----- �\���̒�`
struct LAND {								//----- �n��
	int			status;							// �X�e�[�^�X
	D3DXMATRIX	matrix;							// ���[�J�����[���h�ϊ��s��
};

struct DONUTS {								//----- �h�[�i�c
	int			status;							// �X�e�[�^�X
	D3DXVECTOR3	position;						// ���W
	D3DXVECTOR3	rotation;						// ��]��
	D3DXVECTOR3	rot_speed;						// ��]���x
	D3DXMATRIX	matrix;							// ���[�J�����[���h�ϊ��s��
	D3DXMATRIX	shadow_matrix;					// �V���h�E���[���h�ϊ��s��
};

struct POINTLIGHT {							//----- �_����
	int			status;							// �X�e�[�^�X
	D3DXVECTOR3	position;						// ���W
	D3DXVECTOR3	base_cycle;						// ���S�ړ��p��]�l
	D3DXVECTOR3	base_speed;						// ���S�ړ��p��]���x�l
	TLVERTEX	vertex[LIGHT_DETAIL + 2];		// ���_�f�[�^
	float		cycle[LIGHT_DETAIL + 2];		// ���_�ω��T�C���J�[�u�p�x�l
 	float		speed[LIGHT_DETAIL + 2];		// ���_�ω��T�C���J�[�u�p���x�l
 	float		range1, range2;					// ���_�ω��T�C���J�[�u�ω��l
};

struct MODELINFO {							//----- ���f��
	LPD3DXMESH				lpD3DXMesh;			// D3DXMESH �C���^�[�t�F�C�X
	LPDIRECT3DVERTEXBUFFER9	lpD3DVertexBuffer;	// Direct3DVertexBuffer8 �I�u�W�F�N�g
	LPDIRECT3DINDEXBUFFER9	lpD3DIndexBuffer;	// Direct3DIndexBuffer8 �I�u�W�F�N�g
	int						iNumVertex;			// ���_��
	int						iNumIndex;			// �C���f�b�N�X��
	LPDIRECT3DTEXTURE9		lpD3DTexture;		// Direct3DTexture8 �I�u�W�F�N�g
};

//----- �O���[�o���ϐ�

// DirectX �֘A
LPDIRECT3D9				g_pD3D;						// Direct3D8 �I�u�W�F�N�g
LPDIRECT3DDEVICE9		g_pD3DDevice;				// Direct3DDevice8 �I�u�W�F�N�g
D3DPRESENT_PARAMETERS	g_D3DPresentParam;			// �v���[���e�[�V�����p�����[�^
LPD3DXFONT				g_pD3DXFont;				// D3DXFont �I�u�W�F�N�g
D3DLIGHT9				g_Light[2];					// D3DLIGHT8 �I�u�W�F�N�g

LAND					g_Land;						// �n�ʕ\���I�u�W�F�N�g
DONUTS					g_Donuts[DONUTS_QTY];		// �h�[�i�c�\���I�u�W�F�N�g
POINTLIGHT				g_PointLight;				// �_�����I�u�W�F�N�g
TLVERTEX				g_GuideLine[2];				// �K�C�h���C�����_�f�[�^
MODELINFO				g_ModelInfo[MODEL_QTY];		// �\�����f���I�u�W�F�N�g

D3DXMATRIX				g_MatView;					// �r���[�}�g���b�N�X
float					g_fRotation;				// �r���[��]�p
float					g_fHeight;					// �r���[���x
TCHAR					g_szDebug[4096];			// �o�͕�����o�b�t�@
float					g_FPS;						// �t���[�����J�E���g�p

//----- �v���g�^�C�v�錾
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
long Initialize3DEnvironment(HWND hWnd, bool bWindow);
HRESULT Cleanup3DEnvironment(HWND hWnd);
HRESULT Render3DEnvironment();
int SetModelData(HWND hWnd);
void ExecLand();
void DrawLand();
void DrawGuideLine();
void ExecPointLight();
void DrawPointLight();
void ExecDonuts();
void DrawDonuts();

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
	int		i;

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
	if (hWnd == NULL) return false;

	bWindow = false;
	if (IDYES == MessageBox(hWnd, _T("�E�B���h�E���[�h�Ŏ��s���܂����H"), _T("��ʃ��[�h"), MB_YESNO))
		bWindow = true;
	if (FAILED(Initialize3DEnvironment(hWnd, bWindow))) return 0;	// �R�c���̏�����

	// ���f���f�[�^�̏���
	SetModelData(hWnd);

	// �ϐ�������
	timeBeginPeriod(1);									// �V�X�e���^�C�}�[�̕���\���P�����ɐݒ�
	dwFPSLastTime = dwExecLastTime = timeGetTime();		// ���݂̃V�X�e���^�C�}�[���擾
	dwFrameCount = 0;
	g_fRotation = (float)RAD*(-90);
	g_fHeight = 400.0f;
	// �I�u�W�F�N�g�X�e�[�^�X��������
	g_Land.status = 0;
	for (i = 0; i < DONUTS_QTY; i++) {
		g_Donuts[i].status = 0;
	}
	g_PointLight.status = 0;

	// ���C���E�C���h�E���[�v
    msg.message = WM_NULL;
	while (WM_QUIT != msg.message) {								// WM_QUIT ������܂Ń��[�v
		if (PeekMessage(&msg,NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			dwCurrentTime = timeGetTime();								// ���݂̃^�C�}�[�l���擾
			if (dwCurrentTime - dwFPSLastTime >= 500) {					// �O�D�T�b���ƂɌv��
				dt = (dwCurrentTime - dwFPSLastTime) / 1000.0f;			// �t���[�������v�Z
				g_FPS = (float)dwFrameCount / dt;
				dwFPSLastTime = dwCurrentTime;							// �^�C�}�[�l���X�V
				dwFrameCount = 0;										// �t���[���J�E���^�����Z�b�g
			}
			// ���̕ӂŎ��ԊǗ�
			if (dwCurrentTime - dwExecLastTime >= FRAME_RATE) {			// ��莞�Ԃ��o�߂�����E�E�E
				dwExecLastTime = dwCurrentTime;							// �^�C�}�[�l���X�V
				ExecLand();												// �n�ʏ���
				ExecDonuts();											// �h�[�i�c����
				ExecPointLight();										// �_��������
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
	D3DDISPLAYMODE	disp;				// �f�B�X�v���C���[�h���L�q
	int				nDev;				// �f�o�C�X��ʃJ�E���^
	D3DXMATRIX		matProj;			// �v���W�F�N�V�����}�g���b�N�X

	// Direct3D �I�u�W�F�N�g���쐬
	g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!g_pD3D) {
		MessageBox(NULL, _T("Direct3D �I�u�W�F�N�g�̍쐬�Ɏ��s���܂����B"), _T("Error"), MB_OK | MB_ICONSTOP);
		return E_FAIL;
	}
	// Direct3D �������p�����[�^��ݒ�
	ZeroMemory(&g_D3DPresentParam, sizeof(D3DPRESENT_PARAMETERS));
	if (bWindow)	{
		// �E�B���h�E���[�h
		g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &disp);	// ���݂̉�ʃ��[�h���擾
		g_D3DPresentParam.BackBufferFormat = disp.Format;
		g_D3DPresentParam.Windowed = 1;
		g_D3DPresentParam.FullScreen_RefreshRateInHz = 0;
		g_D3DPresentParam.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	} else {
		// �t���X�N���[�����[�h
		ShowCursor(FALSE);
		g_D3DPresentParam.BackBufferFormat = D3DFMT_R5G6B5;
		g_D3DPresentParam.Windowed = 0;
		g_D3DPresentParam.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
		g_D3DPresentParam.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;	// VSYNC��҂��Ȃ�
	}
	// �\���̈�T�C�Y�̐ݒ�
	g_D3DPresentParam.BackBufferWidth  = SCREEN_WIDTH;
	g_D3DPresentParam.BackBufferHeight = SCREEN_HEIGHT;
	g_D3DPresentParam.SwapEffect       = D3DSWAPEFFECT_DISCARD;
	// �y�o�b�t�@�̎����쐬
	g_D3DPresentParam.EnableAutoDepthStencil = 1;
	g_D3DPresentParam.AutoDepthStencilFormat = D3DFMT_D16;

	// �f�o�C�X�I�u�W�F�N�g���쐬
	for (nDev = 0; nDev < c_nMaxDevice; nDev++) {
		if (SUCCEEDED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, device[nDev].type, hWnd,	// �f�o�C�X���쐬
			device[nDev].behavior, &g_D3DPresentParam, &g_pD3DDevice))) break;
	}
	if (nDev >= c_nMaxDevice) {
		MessageBox(hWnd, _T("�f�o�C�X�̍쐬�Ɏ��s���܂���"), _T("error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// �t�H���g�Z�b�g�A�b�v
	D3DXCreateFont(g_pD3DDevice, 18, 0, 0, 0, FALSE, SHIFTJIS_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Terminal"), &g_pD3DXFont);

	// �v���W�F�N�V�����}�g���b�N�X�ݒ�
	D3DXMatrixPerspectiveFovLH(&matProj, 60.0f * D3DX_PI/180.0f, (float)SCREEN_WIDTH/SCREEN_HEIGHT, 100.0f, 10000.0f);
	g_pD3DDevice->SetTransform(D3DTS_PROJECTION, &matProj);	// �v���W�F�N�V�����}�g���b�N�X���Z�b�g

	// �}�e���A���̐ݒ�
	D3DMATERIAL9 mtrl;
	ZeroMemory(&mtrl, sizeof(mtrl));
	mtrl.Diffuse.r  = 1.0f;	mtrl.Diffuse.g  = 1.0f;	mtrl.Diffuse.b  = 1.0f;
	mtrl.Ambient.r  = 0.2f;	mtrl.Ambient.g  = 0.2f;	mtrl.Ambient.b  = 0.2f;
	g_pD3DDevice->SetMaterial(&mtrl);

	// ���C�g�̐ݒ�
	ZeroMemory(&g_Light[0], sizeof(D3DLIGHT9));			// [0] ���F��
	g_Light[0].Type       = D3DLIGHT_DIRECTIONAL;		// ���s����
	g_Light[0].Diffuse.r  = 1.0f;	g_Light[0].Diffuse.g  = 1.0f;	g_Light[0].Diffuse.b  = 1.0f;
	g_Light[0].Ambient.r  = 0.4f;	g_Light[0].Ambient.g  = 0.4f;	g_Light[0].Ambient.b  = 0.4f;
	ZeroMemory(&g_Light[1], sizeof(D3DLIGHT9));			// [1] ���F���i�H�j
	g_Light[1].Type       = D3DLIGHT_DIRECTIONAL;		// ���s����
	g_Light[1].Diffuse.r  = 0.0f;	g_Light[1].Diffuse.g  = 0.0f;	g_Light[1].Diffuse.b  = 0.0f;
	g_Light[1].Ambient.r  = 0.0f;	g_Light[1].Ambient.g  = 0.0f;	g_Light[1].Ambient.b  = 0.0f;

	return S_OK;
}

//-------------------------------------------------------------------------------------------------
// �R�c���̃N���[���A�b�v
//-------------------------------------------------------------------------------------------------
HRESULT Cleanup3DEnvironment(HWND hWnd)
{
	int			nModel;
	MODELINFO	*pt;

	SAFE_RELEASE(g_pD3DXFont);					// D3DXFont���
	for (nModel = 0; nModel < MODEL_QTY; nModel++) {
		pt = &g_ModelInfo[nModel];
		SAFE_RELEASE(pt->lpD3DVertexBuffer);	// ���_�o�b�t�@���
		SAFE_RELEASE(pt->lpD3DIndexBuffer);		// �C���f�b�N�X�o�b�t�@���
		SAFE_RELEASE(pt->lpD3DXMesh);			// ���b�V�����
		SAFE_RELEASE(pt->lpD3DTexture);			// �e�N�X�`�����
	}
	SAFE_RELEASE(g_pD3DDevice);					// Direct3DDevice �I�u�W�F�N�g���
	SAFE_RELEASE(g_pD3D);						// Direct3D �I�u�W�F�N�g���
	return S_OK;
}

//-------------------------------------------------------------------------------------------------
// �R�c���̃����_�����O
//-------------------------------------------------------------------------------------------------
HRESULT Render3DEnvironment()
{
	D3DXMATRIX	matWorld;		// ���[���h�}�g���b�N�X
	float		xx, zz;
	TCHAR		str[256];
	static RECT	strRect = {0, 0, 640, 480};
	
	g_szDebug[0] = _T('\0');	// �f�o�b�O�X�g�����O�o�b�t�@������
	// �L�[�ɂ�鎋�_�ʒu�̈ړ�
	if (GetAsyncKeyState(VK_UP     ) & 0x8000) g_fHeight   += 20.00f;	// �J�[�\���i��j
	if (GetAsyncKeyState(VK_DOWN   ) & 0x8000) g_fHeight   -= 20.00f;	// �J�[�\���i���j
	if (GetAsyncKeyState(VK_LEFT   ) & 0x8000) g_fRotation += 0.05f;	// �J�[�\���i���j
	if (GetAsyncKeyState(VK_RIGHT  ) & 0x8000) g_fRotation -= 0.05f;	// �J�[�\���i�E�j
	D3DXVECTOR3	vecDir = -1.0f * g_PointLight.position;
	D3DXVec3Normalize((D3DXVECTOR3*)&g_Light[0].Direction, &vecDir);
	D3DXVec3Normalize((D3DXVECTOR3*)&g_Light[1].Direction, &vecDir);

	// �o�b�N�o�b�t�@�Ƃy�o�b�t�@���N���A
	g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0x00, 0x90, 0xc0), 1.0f, 0);

	// �r���[�}�g���b�N�X�ݒ�
	xx = cosf(g_fRotation) * 1400.0f;
	zz = sinf(g_fRotation) * 1400.0f;
	D3DXMatrixLookAtLH(&g_MatView, &D3DXVECTOR3(xx, g_fHeight, zz), &D3DXVECTOR3(0, 500.0f, 0), &D3DXVECTOR3(0, 1, 0));
	g_pD3DDevice->SetTransform(D3DTS_VIEW, &g_MatView);

	if (SUCCEEDED(g_pD3DDevice->BeginScene())) {	// �V�[���J�n
		DrawLand();						// �n�ʕ`��
		DrawDonuts();					// �h�[�i�c�`��
		DrawGuideLine();				// �K�C�h���C���`��
		DrawPointLight();				// �_�����`��

		// �f�o�b�O�X�g�����O�`��
		_stprintf(str, _T("FPS : %4.1f\n"), g_FPS);		strcat(g_szDebug, str);
		g_pD3DXFont->DrawText(NULL, g_szDebug, -1, &strRect, DT_LEFT, D3DCOLOR_ARGB(0xff, 0xff, 0xff, 0xff));

		g_pD3DDevice->EndScene();		// �`��I��
	}

	// �o�b�N�o�b�t�@���v���C�}���o�b�t�@�ɃR�s�[
	if (FAILED(g_pD3DDevice->Present(NULL,NULL,NULL,NULL))) g_pD3DDevice->Reset(&g_D3DPresentParam);

	return S_OK;
}

//-------------------------------------------------------------------------------------------------
// ���f���f�[�^�Z�b�g�A�b�v
//-------------------------------------------------------------------------------------------------
int SetModelData(HWND hWnd)
{
	DWORD			dwNumMaterials = 0L;
	LPD3DXBUFFER	pD3DXMtrlBuffer, pD3DXEffectInst;
	MODELINFO		*pt;
	int				nModel;

	pt = &g_ModelInfo[0];
	for (nModel = 0; nModel < MODEL_QTY; nModel++, pt++) {
		// �w�t�@�C�����烁�b�V���f�[�^��ǂݍ���
		D3DXLoadMeshFromX(modelfile[nModel], D3DXMESH_SYSTEMMEM, g_pD3DDevice, NULL,
							&pD3DXMtrlBuffer, &pD3DXEffectInst, &dwNumMaterials, &pt->lpD3DXMesh);
		pt->iNumVertex = pt->lpD3DXMesh->GetNumVertices();			// ���_�����Q�b�g
		pt->lpD3DXMesh->GetVertexBuffer(&pt->lpD3DVertexBuffer);	// ���_�o�b�t�@�I�u�W�F�N�g�ւ̃|�C���^���Q�b�g
		pt->iNumIndex = pt->lpD3DXMesh->GetNumFaces();				// �ʐ����Q�b�g
		pt->lpD3DXMesh->GetIndexBuffer(&pt->lpD3DIndexBuffer);		// �C���f�b�N�X�o�b�t�@�I�u�W�F�N�g�ւ̃|�C���^���Q�b�g

		// �e�N�X�`���ǂݍ���
		D3DXCreateTextureFromFileEx(g_pD3DDevice, texturefile[nModel], 0, 0, 0, 0,
			D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_FILTER_NONE,
			0xff000000, NULL, NULL, &pt->lpD3DTexture);				// �J���[�L�[��L���ɂ��Ă���
	}
	return 0;
}

//-----------------------------------------------------------------------------
// �n�ʃI�u�W�F�N�g
//-----------------------------------------------------------------------------
void ExecLand()
{
	LAND* gpt = &g_Land;
	switch (gpt->status) {
	case 0:		// ������
		// ���[���h�}�g���N�X��������
		D3DXMatrixIdentity(&gpt->matrix);		// �P�ʍs��𐶐�
		gpt->status = 1;
		// THRU
	case 1:		// ���C��
		break;
	default:
		break;
	}
}

//----- �n�ʕ`��
void DrawLand()
{
	MODELINFO* mpt = &g_ModelInfo[MN_LAND];
	g_pD3DDevice->SetTransform(D3DTS_WORLD, &g_Land.matrix);						// ���[���h�}�g���N�X�ݒ�
	g_pD3DDevice->SetFVF(FVF_VERTEX);												// ���_�t�H�[�}�b�g�ݒ�
	g_pD3DDevice->SetTexture(0, mpt->lpD3DTexture);									// �e�N�X�`���ݒ�
	g_pD3DDevice->SetStreamSource(0, mpt->lpD3DVertexBuffer, 0, sizeof(VERTEX));	// ���_�o�b�t�@�ݒ�
	g_pD3DDevice->SetIndices(mpt->lpD3DIndexBuffer);								// �C���f�b�N�X�o�b�t�@�ݒ�
	g_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);					// ���u�����f�B���O�ݒ�
	g_pD3DDevice->SetRenderState(D3DRS_ZENABLE, TRUE);								// �y�o�b�t�@�L����
	g_pD3DDevice->SetRenderState(D3DRS_LIGHTING, TRUE);								// ���C�g�L����
	g_pD3DDevice->SetLight(0, &g_Light[0]);
	g_pD3DDevice->LightEnable(0, TRUE);
	g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mpt->iNumVertex, 0, mpt->iNumIndex);		// �`��
}

//-----------------------------------------------------------------------------
// �K�C�h���C���`��
//-----------------------------------------------------------------------------
void DrawGuideLine()
{
	D3DXMATRIX mtrx;
	int i;

	D3DXMatrixIdentity(&mtrx);											// �P�ʍs��𐶐�
	g_pD3DDevice->SetTransform(D3DTS_WORLD, &mtrx);						// ���[���h�}�g���N�X�ݒ�
	g_pD3DDevice->SetFVF(FVF_TLVERTEX);									// ���_�t�H�[�}�b�g�ݒ�
	g_pD3DDevice->SetTexture(0, NULL);									// �e�N�X�`���ݒ�
	g_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);		// ���u�����f�B���O�ݒ�
	g_pD3DDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	g_pD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	for (i = 0; i < DONUTS_QTY; i++) {
		// ���C���[�_�̒��_��ݒ�
		g_GuideLine[0].x       = g_PointLight.position.x;
		g_GuideLine[0].y       = g_PointLight.position.y;
		g_GuideLine[0].z       = g_PointLight.position.z;
		g_GuideLine[0].diffuse = D3DCOLOR_XRGB(0,255,0);
		g_GuideLine[1].x       = g_Donuts[i].position.x + (g_PointLight.position.x - g_Donuts[i].position.x) * -500.0f;
		g_GuideLine[1].y       = g_Donuts[i].position.y + (g_PointLight.position.y - g_Donuts[i].position.y) * -500.0f;
		g_GuideLine[1].z       = g_Donuts[i].position.z + (g_PointLight.position.z - g_Donuts[i].position.z) * -500.0f;
		g_GuideLine[1].diffuse = D3DCOLOR_XRGB(0,255,0);
		g_pD3DDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, &g_GuideLine[0], sizeof(TLVERTEX));		// �`��
	}
}

//-----------------------------------------------------------------------------
// �_�����I�u�W�F�N�g
//-----------------------------------------------------------------------------
//----- ���C��
void ExecPointLight()
{
	POINTLIGHT*	pt;
	int			i;
	float		x;
	BYTE		col_r, col_g, col_b;

	pt = &g_PointLight;
	switch (pt->status) {
	case 0:			// ������
		// ���_�f�[�^��ݒ�
		pt->base_cycle.x = (float)rand() / 10;			// ����W�̉�]�l
		pt->base_cycle.y = (float)rand() / 10;			// ����W�̉�]�l
		pt->base_cycle.z = (float)rand() / 10;			// ����W�̉�]�l
		pt->base_speed.x = (float)rand() / 4000000;		// ����W�̉�]���x�l
		pt->base_speed.y = (float)rand() / 4000000;		// ����W�̉�]���x�l
		pt->base_speed.z = (float)rand() / 4000000;		// ����W�̉�]���x�l
		pt->vertex[0].x = 0;		// �ŏ��̒��_�i�����S�j�̍��W
		pt->vertex[0].y = 0;
		pt->vertex[0].z = 0;
		pt->vertex[0].diffuse = D3DCOLOR_RGBA(0xff, 0xff, 0xff, 0xff);	// ���S�́u���E�s�����v
		col_r = rand() & 0xff;		// �����S�̂��J���[���w��
		col_g = rand() & 0xff;
		col_b = rand() & 0xff;
		for (i = 0; i <= LIGHT_DETAIL; i++) {
			pt->vertex[i + 1].z       = pt->vertex[0].z;
			pt->vertex[i + 1].diffuse = D3DCOLOR_RGBA(col_r, col_g, col_b, 0x00);
			pt->cycle[i + 1] = (float)rand() / 1000;
			pt->speed[i + 1] = (float)rand() / 1000000;
			pt->range1       = (float)(150 + (rand() % 100));
			pt->range2       = (float)( 50 + (rand() %  50));
		}
		pt->cycle[i] = pt->cycle[1];	// �Ō�̒��_�͍ŏ��̒��_�Ɠ���
		pt->speed[i] = pt->speed[1];
		pt->status = 1;
		// THRU
	case 1:			// ���C��
		for (i = 0; i <= LIGHT_DETAIL; i++) {
			x = cosf(pt->cycle[i + 1]);
			pt->vertex[i + 1].x = pt->vertex[0].x
								+ sinf((i * 360 / LIGHT_DETAIL) * RAD) * (pt->range1 + x * pt->range2);
			pt->vertex[i + 1].y = pt->vertex[0].y
								+ cosf((i * 360 / LIGHT_DETAIL) * RAD) * (pt->range1 + x * pt->range2);
			pt->cycle[i + 1] += pt->speed[i + 1];
		}
		pt->position.x = sinf(pt->base_cycle[0]) * 1500.0f;
		pt->position.y = sinf(pt->base_cycle[1]) * 200.0f + 1500.0f;
		pt->position.z = sinf(pt->base_cycle[2]) * 1500.0f;
		pt->base_cycle += pt->base_speed;		// ����W���ړ��i��]�j������
		break;
	default:
		break;
	}
}

//----- �`��
void DrawPointLight()
{
	D3DXMATRIX	matWorld;										// ���[���h�}�g���b�N�X
	g_pD3DDevice->SetFVF(FVF_TLVERTEX);							// ���_�t�H�[�}�b�g�ݒ�
	g_pD3DDevice->SetTexture(0, NULL);							// �e�N�X�`���ݒ�
	// �r���{�[�h�p�ɉ�]�����̓r���[�}�g���b�N�X�̋t�s���ݒ�i�]�u�s��łn�j�j
	matWorld._11 = g_MatView._11;		matWorld._12 = g_MatView._21;		matWorld._13 = g_MatView._31;
	matWorld._21 = g_MatView._12;		matWorld._22 = g_MatView._22;		matWorld._23 = g_MatView._32;
	matWorld._31 = g_MatView._13;		matWorld._32 = g_MatView._23;		matWorld._33 = g_MatView._33;
	matWorld._14 = matWorld._24 = matWorld._34 = 0.0f;
	matWorld._41 = g_PointLight.position.x;
	matWorld._42 = g_PointLight.position.y;
	matWorld._43 = g_PointLight.position.z;
	matWorld._44 = 1.0f;
	g_pD3DDevice->SetTransform(D3DTS_WORLD, &matWorld);						// ���[���h�}�g���b�N�X���Z�b�g
	g_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);				// ���u�����f�B���O�ݒ�
	g_pD3DDevice->SetRenderState(D3DRS_SRCBLEND , D3DBLEND_SRCALPHA);
	g_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	// �`��
	g_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, LIGHT_DETAIL, &g_PointLight.vertex[0], sizeof(TLVERTEX));
}

//-----------------------------------------------------------------------------
// �h�[�i�c�I�u�W�F�N�g
//-----------------------------------------------------------------------------
void ExecDonuts()
{
	DONUTS*		gpt;
	D3DXMATRIX	rot_x, rot_y, rot_z, trans, shadow;
	D3DXVECTOR3	light;
	int			i;

	for (i = 0; i < DONUTS_QTY; i++) {
		gpt = &g_Donuts[i];
		switch (gpt->status) {
		case 0:		// ������
			gpt->position.x = (float)(((rand() % 40) - 20) * 100);			// ���W
			gpt->position.y = (float)(((rand() % 20) - 10) *  30) + 500.0f;
			gpt->position.z = (float)(((rand() % 40) - 20) * 100);
			gpt->rotation.x = gpt->rotation.y = gpt->rotation.z = 0.0f;
			gpt->rot_speed.x = (float)((rand() % 100) - 50) / 1000.0f;		// ��]���x
			gpt->rot_speed.y = (float)((rand() % 100) - 50) / 1000.0f;
			gpt->rot_speed.z = (float)((rand() % 100) - 50) / 1000.0f;
			D3DXMatrixIdentity(&gpt->shadow_matrix);
			gpt->shadow_matrix._11 = 0;
			gpt->shadow_matrix._22 = 0;
			gpt->shadow_matrix._33 = 0;
			gpt->shadow_matrix._44 = 0;
			gpt->status = 1;
			// THRU
		case 1:		// ���C��
			// ��]������
			gpt->rotation += gpt->rot_speed;
			// ���[���h�}�g���N�X��ݒ�
			D3DXMatrixRotationX(&rot_x, gpt->rotation.x);
			D3DXMatrixRotationY(&rot_y, gpt->rotation.y);
			D3DXMatrixRotationZ(&rot_z, gpt->rotation.z);
			D3DXMatrixTranslation(&trans, gpt->position.x, gpt->position.y, gpt->position.z);
			gpt->matrix = rot_x * rot_y * rot_z * trans;		// �}�g���b�N�X������

			//***************************************************************************
			//
			//�@�����ɃV���h�E�{�����[�������߂ĕϐ��Ɋi�[����v���O������ǉ�����
			//�@�i�P�j���̂̍��W�ig_Donuts[i].position�j��������ig_PointLight.position�j
			//�@�@�@�@�ւ̃x�N�g�������߂�
			//�@�i�Q�j�i�P�j�ŋ��߂��x�N�g������V���h�E�{�����[�����쐬����
			//�@�i�R�j���̂������[���h�}�g���b�N�X�igpt->matrix�j�ɃV���h�E�{�����[��
			//�@�@�@�@����Z���A�V���h�E�{�����[���p�ϐ��igpt->shadow_matrix�j�Ɋi�[����
			//
			//***************************************************************************
			// �i�P�j
			light = g_PointLight.position - g_Donuts[i].position;
			// �i�Q�j
			D3DXMatrixIdentity(&shadow);
			shadow._21 = -light.x / light.y;
			shadow._22 = 0.0f;
			shadow._23 = -light.z / light.y;
			shadow._42 = 0.5f;	// Z�t�@�C�e�B���O�΍�
			// �i�R�j
			gpt->shadow_matrix =  gpt->matrix * shadow;
			//***************************************************************************

			break;
		default:
			break;
		}
	}
}

//----- �h�[�i�c���e��`��
void DrawDonuts()
{
	DONUTS*		gpt;
	MODELINFO*	mpt = &g_ModelInfo[MN_DONUTS];
	int i;

	g_pD3DDevice->SetFVF(FVF_VERTEX);												// ���_�t�H�[�}�b�g�ݒ�
	g_pD3DDevice->SetStreamSource(0, mpt->lpD3DVertexBuffer, 0, sizeof(VERTEX));	// ���_�o�b�t�@�ݒ�
	g_pD3DDevice->SetIndices(mpt->lpD3DIndexBuffer);								// �C���f�b�N�X�o�b�t�@�ݒ�
	g_pD3DDevice->SetRenderState(D3DRS_ZENABLE, TRUE);								// �y�o�b�t�@�L����
	g_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);					// ���u�����f�B���O�ݒ�
	g_pD3DDevice->SetRenderState(D3DRS_LIGHTING, TRUE);								// ���C�g�L����
	g_pD3DDevice->LightEnable(0, TRUE);

	// �h�[�i�c��`��
	g_pD3DDevice->SetTexture(0, mpt->lpD3DTexture);									// �e�N�X�`���ݒ�
	g_pD3DDevice->SetLight(0, &g_Light[0]);
	for (i = 0; i < DONUTS_QTY; i++) {
		gpt = &g_Donuts[i];
		g_pD3DDevice->SetTransform(D3DTS_WORLD, &gpt->matrix);						// ���[���h�}�g���N�X�ݒ�
		g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
									0, 0, mpt->iNumVertex, 0, mpt->iNumIndex);		// �`��
	}

	// �e��`��
	g_pD3DDevice->SetTexture(0, NULL);												// �e�N�X�`���ݒ�
	g_pD3DDevice->SetLight(0, &g_Light[1]);
	for (i = 0; i < DONUTS_QTY; i++) {
		gpt = &g_Donuts[i];
		g_pD3DDevice->SetTransform(D3DTS_WORLD, &gpt->shadow_matrix);				// �V���h�E�}�g���N�X�ݒ�
		g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
									0, 0, mpt->iNumVertex, 0, mpt->iNumIndex);		// �`��
	}
}

//=================================================================================================
//	end of file
//=================================================================================================
