//=======================================================================================
//
//	  [ ���[�t�B���O ] : DirectX Sample Program
//		�w�t�@�C�����Q�ǂݍ���Ń��[�t�B���O�ŕό`
//
//�@�@������@
//�@�@�m�P�n�F���f���O��`��
//�@�@�m�Q�n�F���f���P��`��
//�@�@�m�R�n�F���[�t�B���O�����f����`��
//
//=======================================================================================
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
#define SAFE_RELEASE(x)	if(x){x->Release();x=NULL;}
#endif
#ifndef SAFE_DELETE
#define SAFE_DELETE(x)	if(x){delete x;x=NULL;}
#endif

//-------- �萔��`
#define CLASS_NAME		_T("Morphing")								// �E�B���h�E�N���X��
#define CAPTION_NAME	_T("[ ���[�t�B���O ] (����r���̗�)")		// �L���v�V������

const float FRAME_RATE		= 1000.0f/60.0f;						// �t���[�����[�g
const int	SCREEN_WIDTH	= 640;									// �X�N���[���̕�
const int	SCREEN_HEIGHT	= 480;									// �X�N���[���̍���
const float	ASPECT_RATIO	= (float)SCREEN_WIDTH/SCREEN_HEIGHT;	// �A�X�y�N�g��
const int	FONT_SIZE		= 14;									// �f�o�b�O�����t�H���g�T�C�Y
LPCTSTR		FONT_TYPE		= _T("�l�r �o�S�V�b�N");				// �f�o�b�O�����t�H���g���
const float	NEAR_CLIP		= 100.0f;								// �j�A�N���b�v���s������
const float	FAR_CLIP		= 30000.0f;								// �t�@�[�N���b�v���s������
const float	RAD				= D3DX_PI/180.0f;						// ���W�A��
const float ROTATE_SPEED	= 0.04f;								// ��]���x

#define MODEL_MAX	2

//----- ���_�t�H�[�}�b�g��`
struct VERTEX {
	float		x, y, z;			// ���_���W�i���W�ϊ�����j
	float		nx, ny, nz;			// �@���x�N�g��
	float		tu, tv;				// �e�N�X�`�����W
};
#define FVF_VERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)

//----- �\���̒�`
struct OBJECT {			//--- �I�u�W�F�N�g�p�\����
	int			status;				// �X�e�[�^�X
	float		x, y, z;			// ���W
	float		rotx, roty, rotz;	// �p�x
	D3DXMATRIX	matrix;				// �ϊ��}�g���b�N�X
	int			morph_count;		// ���[�t�B���O�v�Z�p���ԃJ�E���g�l
	int			morph_dir;			// ���[�t�B���O�̕���
	int			morph_time[2];		// ���[�t�B���O�ɗv���鎞��
};

//=======================================================================================
// �O���[�o���ϐ���`
//=======================================================================================
LPDIRECT3D9					g_pD3D;					// Direct3D8 �I�u�W�F�N�g
LPDIRECT3DDEVICE9			g_pD3DDevice;			// Direct3DDevice8 �I�u�W�F�N�g
D3DPRESENT_PARAMETERS		g_D3DPresentParam;		// PRESENT PARAMETERS
LPD3DXFONT					g_pD3DXFont;			// D3DXFont �I�u�W�F�N�g
float						g_FPS;					// �t���[�����J�E���g�p
LPDIRECT3DTEXTURE9			g_pD3DTexture;			// Direct3DTexture8 �I�u�W�F�N�g
OBJECT						g_Object;				// �I�u�W�F�N�g���

VERTEX						*pt_vertex[MODEL_MAX];	// ���_�f�[�^�ւ̃|�C���^
WORD						*pt_index[MODEL_MAX];	// �C���f�b�N�X�f�[�^�ւ̃|�C���^
int							num_vertex[MODEL_MAX];	// ���_��
int							num_face[MODEL_MAX];	// �ʐ�
VERTEX						*morph_vertex;			// ���[�t�B���O�p���_���[�N�ւ̃|�C���^
WORD						*morph_index;			// ���[�t�B���O�p�C���f�b�N�X���[�N�ւ̃|�C���^

int							g_DrawMode;				// �`�惂�f���I��
bool						g_Wireframe;			// ���C���[�t���[���`��X�C�b�`

//=======================================================================================
// �v���g�^�C�v�錾
//=======================================================================================
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT InitializeGraphics(HWND hWnd, bool bWindow);
HRESULT CleanupGraphics();
HRESULT RenderGraphics();
int SetModelData(HWND hWnd);
void ExecPat();
void DrawPat();

//---------------------------------------------------------------------------------------
//	���C��
//---------------------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int iCmdShow)
{
	HWND	hWnd;
	MSG		msg;
	DWORD	dwExecLastTime, dwFPSLastTime, dwCurrentTime, dwFrameCount;
	float	dt;
	bool	bWindow;

	// �E�B���h�E�N���X�̓o�^
	WNDCLASSEX wc = {
		sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WindowProc,
		0, 0, hInstance, LoadIcon(hInstance, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)GetStockObject(WHITE_BRUSH), NULL, CLASS_NAME, NULL};
	if (RegisterClassEx(&wc) == 0) return false;	// �E�B���h�E�N���X��o�^

	// �E�B���h�E���쐬
	hWnd = CreateWindow(
		CLASS_NAME, CAPTION_NAME, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION, 100, 100,
		SCREEN_WIDTH  + GetSystemMetrics(SM_CXFIXEDFRAME) * 2,
		SCREEN_HEIGHT + GetSystemMetrics(SM_CYFIXEDFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION),
		NULL, NULL, hInstance, NULL);
	if (hWnd == NULL) return false;
	ShowWindow(hWnd, iCmdShow);		// �E�B���h�E��\��
	UpdateWindow(hWnd);				// �E�B���h�E���A�b�v�f�[�g

	// �O���t�B�b�N���̏�����
	bWindow = false;
	if (IDYES == MessageBox(hWnd, _T("�E�B���h�E���[�h�Ŏ��s���܂����H"), _T("��ʃ��[�h"), MB_YESNO))
		bWindow = true;
	if (FAILED(InitializeGraphics(hWnd, bWindow))) return false;	// �����ŏ�����

	// �e�평����
	srand(timeGetTime());
	dwFPSLastTime = dwExecLastTime = timeGetTime();		// ���݂̃V�X�e���^�C�}�[���擾
	dwFrameCount = 0;
	SetModelData(hWnd);					// ���f���f�[�^�̏���
	g_Object.status = 0;				// �X�e�[�^�X���O�i���������j�ɐݒ�
	g_DrawMode  = 0;
	g_Wireframe = false;
	// ���b�Z�[�W���[�v
	timeBeginPeriod(1);					// �V�X�e���^�C�}�[�̕���\���P�����ɐݒ�
    msg.message = WM_NULL;
	while (WM_QUIT != msg.message) {						// WM_QUIT ������܂Ń��[�v
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {		// ���b�Z�[�W���`�F�b�N
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			// ���̕ӂŃt���[�����J�E���g
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
				ExecPat();												// �p�^�[���ړ����s
				RenderGraphics();										// �����_�����O
				dwFrameCount++;											// �t���[���J�E���g���{�P
			}
		}
		Sleep(1);
	}
	CleanupGraphics();		// �O���t�B�b�N���̃N���[���A�b�v
	timeEndPeriod(1);		// �V�X�e���^�C�}�[�̕���\�����ɖ߂�
	return msg.wParam;
}

//---------------------------------------------------------------------------------------
//	�E�B���h�E�v���V�[�W��
//---------------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_DESTROY:				//----- �I���w��������
		PostQuitMessage(0);				// �V�X�e���ɃX���b�h�̏I����v��
		return 0;
	case WM_KEYDOWN:				//----- �L�[�{�[�h�������ꂽ
		switch (wParam) {
		case '1':		g_DrawMode = 0;				break;		// �P�ڂ̃��f����`��
		case '2':		g_DrawMode = 1;				break;		// �Q�ڂ̃��f����`��
		case '3':		g_DrawMode = 2;				break;		// ���[�t�B���O���f����`��
		case 'W':		g_Wireframe = !g_Wireframe;	break;		// ���C���[�t���[���̐؂�ւ�
		case VK_ESCAPE:	DestroyWindow(hWnd);		break;		// [ESC]�L�[�������ꂽ
		}
		return 0;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//=======================================================================================
//    DirectX Graphics �֘A
//=======================================================================================

//---------------------------------------------------------------------------------------
// �O���t�B�b�N���̏�����
//---------------------------------------------------------------------------------------
HRESULT InitializeGraphics(HWND hWnd, bool bWindow)
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
	D3DXMATRIX		matView, matProj;	// �r���[�}�g���b�N�X�A�v���W�F�N�V�����}�g���b�N�X
	D3DMATERIAL9	mtrl;				// �}�e���A���I�u�W�F�N�g
	D3DXVECTOR3		vecDir;				// �������w�肷��x�N�g��
	D3DLIGHT9		light;				// ���C�g�I�u�W�F�N�g

	// Direct3D�I�u�W�F�N�g�𐶐�
	if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL) {
		MessageBox(hWnd, _T("Direct3D�I�u�W�F�N�g�̍쐬�Ɏ��s���܂���"), _T("error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}
	// ���݂̉�ʃ��[�h���擾
	if (FAILED(g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &disp))) {
		MessageBox(hWnd, _T("�f�B�X�v���C���[�h�̎擾�Ɏ��s���܂���"), _T("error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}
	// Direct3D �������p�����[�^�̐ݒ�
	ZeroMemory(&g_D3DPresentParam, sizeof(D3DPRESENT_PARAMETERS));
	g_D3DPresentParam.BackBufferWidth        = SCREEN_WIDTH;			// �Q�[����ʃT�C�Y
	g_D3DPresentParam.BackBufferHeight       = SCREEN_HEIGHT;
	g_D3DPresentParam.Windowed               = bWindow;					// �E�B���h�E���[�h��?
	g_D3DPresentParam.BackBufferFormat       = (bWindow) ? disp.Format : D3DFMT_R5G6B5;	// �o�b�N�o�b�t�@
	g_D3DPresentParam.SwapEffect             = D3DSWAPEFFECT_DISCARD;
	g_D3DPresentParam.EnableAutoDepthStencil = true;					// �y�o�b�t�@�L��
	g_D3DPresentParam.AutoDepthStencilFormat = D3DFMT_D16;				// �y�o�b�t�@�t�H�[�}�b�g

	// �f�o�C�X�I�u�W�F�N�g���쐬
	for (nDev = 0; nDev < c_nMaxDevice; nDev++) {
		if (SUCCEEDED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, device[nDev].type, hWnd,	// �f�o�C�X���쐬
					device[nDev].behavior, &g_D3DPresentParam, &g_pD3DDevice))) break;
	}
	if (nDev >= c_nMaxDevice) {
		MessageBox(hWnd, _T("�f�o�C�X�̍쐬�Ɏ��s���܂���"), _T("error"), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// ���\���p�t�H���g��ݒ�
	D3DXCreateFont(g_pD3DDevice, FONT_SIZE, 0, 0, 0, FALSE, SHIFTJIS_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, FONT_TYPE, &g_pD3DXFont);

	// �����_�����O�X�e�[�g��ݒ�
	g_pD3DDevice->SetRenderState(D3DRS_ZENABLE, TRUE);			// �y�o�b�t�@�L����
	g_pD3DDevice->SetRenderState(D3DRS_AMBIENT, 0x00303030);	// �����̐ݒ�

	//-------- ��������R�c�֘A
	// �r���[�}�g���b�N�X�ݒ�
	D3DXMatrixLookAtLH(&matView,
						&D3DXVECTOR3(  0.0,  0.0,-1000.0),		// ���_���W
						&D3DXVECTOR3(  0.0,  0.0,    0.0),		// �����_���W
						&D3DXVECTOR3(  0.0,  1.0,    0.0));		// �A�b�v�x�N�g��
	g_pD3DDevice->SetTransform(D3DTS_VIEW, &matView);

	// �v���W�F�N�V�����}�g���b�N�X�ݒ�
	D3DXMatrixPerspectiveFovLH(&matProj, 30 * RAD, ASPECT_RATIO, NEAR_CLIP, FAR_CLIP);
	g_pD3DDevice->SetTransform(D3DTS_PROJECTION, &matProj);

	// �}�e���A���̐ݒ�
	ZeroMemory(&mtrl, sizeof(mtrl));			// ��������[���ŃN���A
	mtrl.Diffuse.r = mtrl.Diffuse.g = mtrl.Diffuse.b = mtrl.Diffuse.a = 1.0;
	mtrl.Ambient.r = mtrl.Ambient.g = mtrl.Ambient.b = mtrl.Ambient.a = 1.0;
	g_pD3DDevice->SetMaterial(&mtrl);

	// ���C�g�̐ݒ�
	ZeroMemory(&light, sizeof(light));
	light.Type = D3DLIGHT_DIRECTIONAL;			// ���s����
	light.Diffuse.r = light.Diffuse.g = light.Diffuse.b = light.Diffuse.a = 1.0;
	vecDir = D3DXVECTOR3(1.0, -1.0, 1.0);							// ���̌��������߂�
	D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, &vecDir);		// ���K�������x�N�g����Ԃ�
	g_pD3DDevice->SetLight(0, &light);
	g_pD3DDevice->LightEnable(0, TRUE);			// ���C�g�O��L��

	return S_OK;
}

//---------------------------------------------------------------------------------------
// �O���t�B�b�N���̃N���[���A�b�v
//---------------------------------------------------------------------------------------
HRESULT CleanupGraphics()
{
	int		i;

	for (i = 0; i < MODEL_MAX; i++) {
		SAFE_DELETE(pt_vertex[i]);	// ���_�f�[�^�̗̈�����
		SAFE_DELETE(pt_index[i]);	// �C���f�b�N�X�f�[�^�̗̈�����
	}
	SAFE_DELETE(morph_vertex);		// ���[�t�B���O�p���_�f�[�^�̗̈�����
	SAFE_DELETE(morph_index);		// ���[�t�B���O�p�C���f�b�N�X�f�[�^�̗̈�����

	SAFE_RELEASE(g_pD3DXFont);		// D3DXFont �I�u�W�F�N�g�����
	SAFE_RELEASE(g_pD3DTexture);	// �e�N�X�`���I�u�W�F�N�g�����
	SAFE_RELEASE(g_pD3DDevice);		// Direct3DDevice �I�u�W�F�N�g�����
	SAFE_RELEASE(g_pD3D);			// Direct3D �I�u�W�F�N�g�����
	return S_OK;
}

//---------------------------------------------------------------------------------------
// �O���t�B�b�N���̃����_�����O
//---------------------------------------------------------------------------------------
HRESULT RenderGraphics()
{
	RECT	rcStr;
	TCHAR	str[1024], tmp[256];
	
	// �r���[�|�[�g���N���A
	g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0x00, 0x40, 0x80), 1.0, 0);

	if (SUCCEEDED(g_pD3DDevice->BeginScene())) {		// �V�[���J�n

		DrawPat();		// �p�^�[����`��

		// �f�o�b�O�X�g�����O�`��
		str[0] = _T('\0');
		_stprintf(tmp, _T("FPS = %6.2f\n\n"), g_FPS);	lstrcat(str, tmp);			// �e�o�r
		_stprintf(tmp, _T("DrawMode = %d\n\n"), g_DrawMode);	lstrcat(str, tmp);	// �`�惂�[�h
		_stprintf(tmp, _T("Time = %d\n\n"), g_Object.morph_count);	lstrcat(str, tmp);	// ����
		_stprintf(tmp, _T("Rotate X(%6.2f), Y(%6.2f), Z(%6.2f)\n"),
						g_Object.rotx, g_Object.roty, g_Object.rotz);	lstrcat(str, tmp);	// �p�x
		SetRect(&rcStr, 0, 0, 640, 480);
		g_pD3DXFont->DrawText(NULL, str, -1, &rcStr, 0, D3DCOLOR_ARGB(0xff, 0xff, 0xff, 0xff));

		g_pD3DDevice->EndScene();					// �V�[���I��
	}

	// �o�b�N�o�b�t�@�ɕ`�悵�����e��\��
	if (FAILED(g_pD3DDevice->Present(NULL, NULL, NULL, NULL))) {
		g_pD3DDevice->Reset(&g_D3DPresentParam);
	}
	return S_OK;
}

//---------------------------------------------------------------------------------------
// ���f���f�[�^�Z�b�g�A�b�v
//---------------------------------------------------------------------------------------
int SetModelData(HWND hWnd)
{
	LPD3DXMESH				mesh;			// ���b�V���I�u�W�F�N�g
	LPDIRECT3DVERTEXBUFFER9	vertex_buffer;	// ���_�o�b�t�@
	LPDIRECT3DINDEXBUFFER9	index_buffer;	// �C���f�b�N�X�o�b�t�@

	DWORD			dwNumMaterials = 0L;
    LPD3DXBUFFER	pD3DXMtrlBuffer, pD3DXEffectInst;
	VERTEX*		pVertices;		// ���_�f�[�^�ւ̃|�C���^
	WORD*		pIndices;		// �|���S���\���i���_�����N�j�f�[�^�ւ̃|�C���^
	VERTEX*		pVertices_2;
	WORD*		pIndices_2;

	static LPCTSTR filename[] = {
		_T("ishi.x"), _T("ishi2.x")		// ���f���i�w�t�@�C���j�̃t�@�C�����i�Q�j
	};
	int			i;

	for (i = 0; i < MODEL_MAX; i++) {
	    // �w�t�@�C�����烁�b�V���f�[�^��ǂݍ���
		D3DXLoadMeshFromX(filename[i], D3DXMESH_SYSTEMMEM, g_pD3DDevice, NULL,	// �����łw�t�@�C�����w��
							&pD3DXMtrlBuffer, &pD3DXEffectInst, &dwNumMaterials, &mesh);
		mesh->GetVertexBuffer(&vertex_buffer);	// ���_�o�b�t�@�I�u�W�F�N�g�ւ̃|�C���^���Q�b�g
		mesh->GetIndexBuffer(&index_buffer);	// �C���f�b�N�X�o�b�t�@�I�u�W�F�N�g�ւ̃|�C���^���Q�b�g
		num_vertex[i] = mesh->GetNumVertices();	// ���_�����Q�b�g
		num_face[i]  = mesh->GetNumFaces();		// �ʐ����Q�b�g

		// ���_�f�[�^�A�C���f�b�N�X�f�[�^���������ɃR�s�[
		pt_vertex[i] = new VERTEX[num_vertex[i]];						// ���_���[�N�̈���m��
		pt_index[i]  = new WORD[num_face[i]*3];							// �C���f�b�N�X���[�N�̈���m��
		
		pVertices_2 = pt_vertex[i];
		vertex_buffer->Lock(0, 0, (void**)&pVertices, 0);				// ���_�o�b�t�@�����b�N
		memcpy(pVertices_2, pVertices, sizeof(VERTEX)*num_vertex[i]);	// ���_�f�[�^�����[�N�ɃR�s�[
		vertex_buffer->Unlock();										// ���_�o�b�t�@���A�����b�N

		pIndices_2 = pt_index[i];
		index_buffer ->Lock(0, 0, (void**)&pIndices , 0);				// �C���f�b�N�X�o�b�t�@�����b�N
		memcpy(pIndices_2, pIndices, sizeof(WORD)*num_face[i]*3);		// �C���f�b�N�X�f�[�^�����[�N�ɃR�s�[
		index_buffer ->Unlock();										// �C���f�b�N�X�o�b�t�@���A�����b�N
	}

	// ���[�t�B���O�p�ɒ��_�o�b�t�@�A�C���f�b�N�X�o�b�t�@���쐬
	morph_vertex = new VERTEX[num_vertex[0]];						// ���[�t�B���O�p���_���[�N�̈���m��
	morph_index  = new WORD[num_face[0]*3];							// ���[�t�B���O�p�C���f�b�N�X���[�N�̈���m��

	vertex_buffer->Lock(0, 0, (void**)&pVertices, 0);				// ���_�o�b�t�@�����b�N
	memcpy(morph_vertex, pVertices, sizeof(VERTEX)*num_vertex[0]);	// ���_�f�[�^�����[�N�ɃR�s�[
	vertex_buffer->Unlock();										// ���_�o�b�t�@���A�����b�N
	index_buffer ->Lock(0, 0, (void**)&pIndices , 0);				// �C���f�b�N�X�o�b�t�@�����b�N
	memcpy(morph_index, pIndices, sizeof(WORD)*num_face[0]*3);		// �C���f�b�N�X�f�[�^�����[�N�ɃR�s�[
	index_buffer ->Unlock();										// �C���f�b�N�X�o�b�t�@���A�����b�N

	// �e�N�X�`���t�@�C����ǂݍ���
	D3DXCreateTextureFromFile(g_pD3DDevice, _T("ishi.bmp"), &g_pD3DTexture);
	return 0;
}

//---------------------------------------------------------------------------------------
//�@�p�^�[������
//---------------------------------------------------------------------------------------
void ExecPat()
{
	D3DXMATRIX	matRotateX, matRotateY, matRotateZ;	// ��]�}�g���b�N�X�i�R��j
	D3DXMATRIX	matTrans;							// ���s�ړ��}�g���b�N�X
	OBJECT*		pt;									// �I�u�W�F�N�g�ւ̃|�C���^
	float		ratio;								// ���Ԃ��狁�߂��ό`�̊���
	VERTEX		*spt0, *spt1, *dpt;					// �e���_�f�[�^�ւ̃|�C���^
	int			i;

	//----- �����{��
	pt = &g_Object;
	switch (pt->status) {
	case 0:		// Initialize
		// �ʒu�̏�����
		pt->x  = 0.0;		pt->y  = 0.0;		pt->z  = 0.0;
		// ��]�p�̏�����
		pt->rotx = 0.0;		pt->roty = 0.0;		pt->rotz = 0.0;
		// ���[�t�B���O�p�ϐ��̏�����
		pt->morph_dir = 0;
		pt->morph_count = 0;
		pt->morph_time[0] = 0;
		pt->morph_time[1] = 30;	// �����ŕό`���Ԃ����܂�
		pt->status = 1;
		// THRU
	case 1:		// Move
		// �L�[���͂ɂ���]
		if (GetAsyncKeyState(VK_UP   ) & 0x8000) pt->rotx += ROTATE_SPEED;	// �J�[�\���i��j
		if (GetAsyncKeyState(VK_DOWN ) & 0x8000) pt->rotx -= ROTATE_SPEED;	// �J�[�\���i���j
		if (GetAsyncKeyState(VK_LEFT ) & 0x8000) pt->roty -= ROTATE_SPEED;	// �J�[�\���i���j
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000) pt->roty += ROTATE_SPEED;	// �J�[�\���i�E�j
		if (GetAsyncKeyState(VK_PRIOR) & 0x8000) pt->rotz += ROTATE_SPEED;	// Page Up
		if (GetAsyncKeyState(VK_NEXT ) & 0x8000) pt->rotz -= ROTATE_SPEED;	// Page Down
		// �}�g���b�N�X������
		D3DXMatrixRotationX(&matRotateX, pt->rotx);							// �w����]�}�g���b�N�X�𐶐�
		D3DXMatrixRotationY(&matRotateY, pt->roty);							// �x����]�}�g���b�N�X�𐶐�
		D3DXMatrixRotationZ(&matRotateZ, pt->rotz);							// �y����]�}�g���b�N�X�𐶐�
		D3DXMatrixTranslation(&matTrans, pt->x, pt->y, pt->z);				// ���s�ړ��}�g���b�N�X�𐶐�
		pt->matrix = matRotateX * matRotateY * matRotateZ * matTrans;		// �}�g���b�N�X�������i�����ɒ��ӁI�j

		//**************************************************************************
		//
		//�@�����Ƀ��[�t�B���O�̌v�Z������ǉ�����B
		//
		//�@�i�P�j�ό`�O���f���̒��_�ւ̃|�C���^�ipt_vertex[0]�j
		//�@�@�@�@�ό`�ヂ�f���̒��_�ւ̃|�C���^�ipt_vertex[1]�j
		//�@�@�@�@�v�Z�ヂ�f���̒��_�ւ̃|�C���^�imorph_vertex�j�����[�N�ϐ��ɃZ�b�g
		//�@�i�Q�j�ό`�O�̎��Ԃƕό`��̎��Ԃƌ��݂̎��Ԃ��犄�������߂�
		//�@�i�R�j�����ƕό`�O��̍��W�̍�������ό`�ʂ����߂�
		//�@      �v�Z�ヂ�f���̒��_���[�N�Ɋi�[
		//�@      �|�C���^�����ꂼ��{�P
		//�@      �ȏ�̏����𒸓_���������[�v���đS�Ă̒��_����������
		//
		//�@���⑫
		//�@�@�ό`�O�̎���    g_Object.morph_time[0]
		//�@�@�ό`��̎���    g_Object.morph_time[1]
		//�@�@���݂̎���      g_Object.morph_count
		//
		//**************************************************************************
		//�i�P�j
		spt0 = pt_vertex[0];
		spt1 = pt_vertex[1];
		dpt  = morph_vertex;

		//�i�Q�j
		ratio = (float)(g_Object.morph_count - g_Object.morph_time[0]) / (float)(g_Object.morph_time[1] - g_Object.morph_time[0]);

		//�i�R�j
		for (i = 0; i < num_vertex[0]; i++) {
			dpt->x = (spt1->x - spt0->x) * ratio + spt0->x;
			dpt->y = (spt1->y - spt0->y) * ratio + spt0->y;
			dpt->z = (spt1->z - spt0->z) * ratio + spt0->z;
			spt0++;
			spt1++;
			dpt++;
		}
		//**************************************************************************

		switch (pt->morph_dir) {
		case 0:			// 0 > 1
			if(++(pt->morph_count) >= pt->morph_time[1]) pt->morph_dir = 1;	// �����𔽓]
			break;
		case 1:			// 1 > 0
			if(--(pt->morph_count) <= 0                ) pt->morph_dir = 0;	// �����𔽓]
			break;
		}
		break;
	default:
		break;
	}
}

//----- �`��
void DrawPat()
{
	g_pD3DDevice->SetFVF(FVF_VERTEX);							// ���_�t�H�[�}�b�g�w��
	g_pD3DDevice->SetTransform(D3DTS_WORLD, &g_Object.matrix);	// ���[���h�}�g���N�X���Z�b�g

	if (g_Wireframe) {
		g_pD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);				// ���C�g������
		g_pD3DDevice->SetTexture(0, NULL);									// �e�N�X�`������
		g_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);	// ���C���[�t���[���`��
	} else {
		g_pD3DDevice->SetRenderState(D3DRS_LIGHTING, TRUE);					// ���C�g�L����
		g_pD3DDevice->SetTexture(0, g_pD3DTexture);							// �e�N�X�`���ݒ�
		g_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);		// �\���b�h�`��
	}

	switch (g_DrawMode) {
	case 0:
		g_pD3DDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,		// ���f���O��`��
						0, num_vertex[0], num_face[0], pt_index[0],
						D3DFMT_INDEX16, pt_vertex[0], sizeof(VERTEX));
		break;
	case 1:
		g_pD3DDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,		// ���f���P��`��
						0, num_vertex[1], num_face[1], pt_index[1],
						D3DFMT_INDEX16, pt_vertex[1], sizeof(VERTEX));
		break;
	case 2:
		g_pD3DDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,		// ���[�t�B���O���f����`��
						0, num_vertex[0], num_face[0], morph_index,
						D3DFMT_INDEX16, morph_vertex, sizeof(VERTEX));
		break;
	}
}

//=======================================================================================
//	end of files
//=======================================================================================
