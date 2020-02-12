//=================================================================================================
//
//	  [ EdgeDetect ] �G�b�W���o�v���O����
//    �X�y�[�X�L�[�Łu���摜�v�u������摜�v��؂�ւ��ĕ\������
//
//=================================================================================================

//-------- �C���N���[�h�w�b�_
#define STRICT				// Windows�v���O�����ɑ΂���u�^�ۏ؁v������
#include <windows.h>		// Windows�v���O�����ɂ͂����t����
#include <tchar.h>			// �ėp�e�L�X�g �}�b�s���O
#include <math.h>

//-------- �萔��`
#define CLASS_NAME		_T("EdgeDetect")							// �E�B���h�E�N���X��
#define CAPTION_NAME	_T("[ EdgeDetect ] �G�b�W���o�v���O�����@�������`��")	// �L���v�V������
const UINT	SCREEN_WIDTH	= 500;									// �X�N���[�����r�b�g�}�b�v��
const UINT	SCREEN_HEIGHT	= 500;									// �X�N���[�����r�b�g�}�b�v����
const UINT	PIXEL_QTY		= SCREEN_WIDTH * SCREEN_HEIGHT;			// �s�N�Z������

#define FILE_NAME		_T("sample.bmp")

//-------- �\���̒�`
typedef union {
	DWORD	dword;
	BYTE	byte[4];
} PIXEL;

//-------- �O���[�o���ϐ���`
HINSTANCE	g_hInst;			// �C���X�^���X
HDC			g_hMemDC[2];		// �������c�b

//-------------------------------------------------------------------------------------------------
// �v���g�^�C�v�錾
//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ExecEdgeDetect(DWORD* srcpt, DWORD* dstpt);

//-------------------------------------------------------------------------------------------------
//	���C��
//-------------------------------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int iCmdShow)
{
	HWND	hWnd;		// �E�B���h�E�n���h��
	MSG		msg;		// ���b�Z�[�W���
	BOOL	bRet;

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
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {	// WM_QUIT ���b�Z�[�W������܂Ń��[�v
		if (bRet == -1)									// �G���[������
			break;
		TranslateMessage(&msg);							// ���z�L�[���b�Z�[�W�𕶎����b�Z�[�W�֕ϊ�
		DispatchMessage(&msg);							// �E�B���h�E�v���V�[�W���փ��b�Z�[�W�𑗏o
	}
	return msg.wParam;
}

//-------------------------------------------------------------------------------------------------
//	�E�B���h�E�v���V�[�W��
//-------------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC			hdc;
	HBITMAP		hSrcBmp, hDstBmp;
	BITMAP		SrcBmpInfo, DstBmpInfo;
	PAINTSTRUCT	ps;
	DWORD		*dstpt, *srcpt;
	static int	draw_sw;
	static LPTSTR	strdata[] = { LPTSTR("���摜"), LPTSTR("������摜")};

	switch (uMsg) {						// ���b�Z�[�W���`�F�b�N
	case WM_CREATE:
		draw_sw = 0;			// ���摜
		hdc = GetDC(hWnd);
		// �r�b�g�}�b�v�t�@�C�������[�h
		// �i���Ӂj�g�p����a�l�o�t�@�C���� 640x480 32bit�J���[�łȂ��Ƃ����Ȃ�
		hDstBmp = (HBITMAP)LoadImage(g_hInst, FILE_NAME, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
		hSrcBmp = (HBITMAP)LoadImage(g_hInst, FILE_NAME, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
		// �r�b�g�}�b�v�����擾
		GetObject(hDstBmp, sizeof(BITMAP), &DstBmpInfo);
		GetObject(hSrcBmp, sizeof(BITMAP), &SrcBmpInfo);
		// �s�N�Z���̐擪�A�h���X���擾
		dstpt = (DWORD*)DstBmpInfo.bmBits;
		srcpt = (DWORD*)SrcBmpInfo.bmBits;

		ExecEdgeDetect(srcpt, dstpt);	// �G�b�W���o�֐�

		g_hMemDC[0] = CreateCompatibleDC(hdc);
		g_hMemDC[1] = CreateCompatibleDC(hdc);
		SelectObject(g_hMemDC[0], hSrcBmp);
		SelectObject(g_hMemDC[1], hDstBmp);
		DeleteObject(hDstBmp);
		DeleteObject(hSrcBmp);
		ReleaseDC(hWnd, hdc);
		return 0;
	case WM_CLOSE:
		DeleteDC(g_hMemDC[0]);
		DeleteDC(g_hMemDC[1]);
		DestroyWindow(hWnd);
		return 0;
	case WM_DESTROY:							// �I���w��������
		PostQuitMessage(0);							// �V�X�e���ɃX���b�h�̏I����v��
		return 0;
	case WM_KEYDOWN:							// �L�[�{�[�h�������ꂽ
		switch (wParam) {							// �p�����[�^���`�F�b�N
		case VK_SPACE:								// �X�y�[�X�ŉ摜�`�F���W
			draw_sw ^= 1;
			InvalidateRect(hWnd, NULL, false);
			return 0;
		case VK_ESCAPE:								// [ESC]�L�[�������ꂽ
			PostMessage(hWnd, WM_CLOSE, 0, 0);		// �I���̃��b�Z�[�W�𔭐�������
			return 0;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// �������c�b��`��
		BitBlt(hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, g_hMemDC[draw_sw], 0, 0, SRCCOPY);
		// �e�L�X�g�\��
		TextOut(hdc, 0, 0, strdata[draw_sw], lstrlen(strdata[draw_sw]));
		EndPaint(hWnd, &ps);
		return 0;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);	// ����E�B���h�E�v���V�[�W�����ďo
}

//-------------------------------------------------------------------------------------------------
// �G�b�W���o�֐�
//-------------------------------------------------------------------------------------------------
void ExecEdgeDetect(DWORD* srcpt, DWORD* dstpt)
{
	int		i, j, x, y;					// ���[�v�J�E���^
	int		tmp_v, tmp_h, tmp;			// �e���|����
	PIXEL	src_pixel[9], ans_pixel;	// ���摜�s�N�Z���f�[�^�A�v�Z��s�N�Z���f�[�^

	// ��������Sobel�t�B���^
	DWORD		sobel_h[9];
	sobel_h[0] = 1;		sobel_h[1] = 2;		sobel_h[2] = 1;
	sobel_h[3] = 0;		sobel_h[4] = 0;		sobel_h[5] = 0;
	sobel_h[6] = -1;	sobel_h[7] = -2;	sobel_h[8] = -1;

	// ��������Sobel�t�B���^
	DWORD		sobel_v[9];
	sobel_v[0] = -1;	sobel_v[1] = 0;	sobel_v[2] = 1;
	sobel_v[3] = -2;	sobel_v[4] = 0;	sobel_v[5] = 2;
	sobel_v[6] = -1;	sobel_v[7] = 0;	sobel_v[8] = 1;

	// �x�����F�s�N�Z������(=SCREEN_HEIGHT)�������[�v
	for (y = 0; y < SCREEN_HEIGHT; y++) {
		// �w�����F�s�N�Z������(=SCREEN_WIDTH)�������[�v
		for (x = 0; x < SCREEN_WIDTH; x++) {
			// srcpt �𒆐S�Ƃ����s�N�Z���Ǝ��͂W�s�N�Z���̒l�A���v�X�̒l���擾����
			// src_pixel �Ɋi�[�i���͂̃s�N�Z�����u�͈͊O�v�̏ꍇ�͒l���O�Ƃ���j
			src_pixel[0].dword = ((y <= 0                ) || (x <= 0               ))? 0: *(srcpt + (y - 1) * SCREEN_WIDTH + (x - 1));
			src_pixel[1].dword = ((y <= 0                )                           )? 0: *(srcpt + (y - 1) * SCREEN_WIDTH + (x));
			src_pixel[2].dword = ((y <= 0                ) || (x >= SCREEN_WIDTH - 1))? 0: *(srcpt + (y - 1) * SCREEN_WIDTH + (x + 1));
			src_pixel[3].dword = (                            (x <= 0               ))? 0: *(srcpt + (y    ) * SCREEN_WIDTH + (x - 1));
			src_pixel[4].dword =                                                           *(srcpt + (y    ) * SCREEN_WIDTH + (x));
			src_pixel[5].dword = (                            (x >= SCREEN_WIDTH - 1))? 0: *(srcpt + (y    ) * SCREEN_WIDTH + (x + 1));
			src_pixel[6].dword = ((y >= SCREEN_HEIGHT - 1) || (x <= 0               ))? 0: *(srcpt + (y + 1) * SCREEN_WIDTH + (x - 1));
			src_pixel[7].dword = ((y >= SCREEN_HEIGHT - 1)                           )? 0: *(srcpt + (y + 1) * SCREEN_WIDTH + (x));
			src_pixel[8].dword = ((y >= SCREEN_HEIGHT - 1) || (x >= SCREEN_WIDTH - 1))? 0: *(srcpt + (y + 1) * SCREEN_WIDTH + (x + 1));
			// �q�f�a�̂��߂R�񃋁[�v�Ōv�Z���s��
			for (j = 0; j < 3; j++) {
				// ���������A���������̃e���|�������O�ɃN���A
				tmp_v = tmp_h = 0;
				// �X�s�N�Z�������[�v
				for (i = 0; i < 9; i++) {
					// �s�N�Z���f�[�^�ɐ��������t�B���^�������A���ʂ�����
					tmp_v += (src_pixel[i].byte[j] * sobel_v[i]);
					// �s�N�Z���f�[�^�ɐ��������t�B���^�������A���ʂ�����
					tmp_h += (src_pixel[i].byte[j] * sobel_h[i]);
				}
				// �e���|��������ŏI�s�N�Z���l�����߂�
				tmp = (int)sqrt((double)tmp_v * tmp_v + tmp_h * tmp_h);
				// �l��臒l���I�[�o�[���Ă��Ȃ����`�F�b�N���A�v�Z��s�N�Z���f�[�^���m��
				if (tmp > 128) {
					ans_pixel.byte[0] = 255;
					ans_pixel.byte[1] = 255;
					ans_pixel.byte[2] = 255;
				}
				else {
					ans_pixel.byte[j] = tmp;
				}
			}
			// �m�肵���s�N�Z���f�[�^���o�̓o�b�t�@�̃|�C���^���w�������A�h���X�֏�������
			*dstpt++ = ans_pixel.dword;
		}
	}
}

//=================================================================================================
//	end
//=================================================================================================
