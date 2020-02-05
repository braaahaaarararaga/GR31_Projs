//=======================================================================================
//
//	  TGA ロードモジュール
//
//=======================================================================================
//-------- インクルードヘッダ
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tga.h"

//---------------------------------------------------------------------------------------
// TGA ファイル ロード
//---------------------------------------------------------------------------------------
TGAImage *LoadTGA(const char *pszFName)
{
	FILE			*fp;
	TGAImage		*pImage;
	unsigned char	rle;				// Run-length encoded
	unsigned char	truecolor;			// True-color Image
	unsigned char	buf[1000];			// ファイル読み込みバッファ
	unsigned char	IDLength;			// Image ID フィールド長
	unsigned char	ColorMapType;		// カラーマップ有無
	unsigned char	ImageType;			// イメージ形式
	unsigned char	ImageDescriptor;	// Image Origin & Alpha Channel Bits
	unsigned char	PacketHeader;		// Packet Type (1bit) & Pixel Count (7bits)
	unsigned char	PixelCount;			// パケット毎のピクセル数
	unsigned		widthBytes;			// ピクセル構成バイト数
	unsigned		i, j;
	unsigned		TGAError;

	fp = fopen(pszFName, "rb");
	if (fp == NULL)
		return NULL;

	pImage = NULL;
	rle = truecolor = 0;
	TGAError = 1;
	do {
		if (fread(&IDLength, 1, 1, fp) < 1)
			break;
		if (fread(&ColorMapType, 1, 1, fp) < 1)
			break;
		if (ColorMapType == 1)
			break;

		pImage = (TGAImage *)malloc(sizeof(TGAImage));
		if (pImage == NULL)
			break;
		memset(pImage, 0, sizeof(*pImage));

		if (fread(&ImageType, 1, 1, fp) < 1)
			break;
		switch (ImageType) {
		case 2:		// Uncompressed, True-color Image
			truecolor = 1;
			break;
		case 3:		// Uncompressed, Black-and-white Image
			pImage->type = 1;
			break;
		case 10:	// Run-length encoded, True-color Image
			rle = 1;
			truecolor = 1;
			break;
		case 11:	// Run-length encoded, Black-and-white Image
			rle = 1;
			pImage->type = 1;
			break;
		default:	// 0, 1, 9 はサポートしない
			free(pImage);
			pImage = NULL;
			break;
		}
		if (pImage == NULL)
			break;

		if (fseek(fp, 5, SEEK_CUR))		// Skip "Color Map Specification"
			break;

		if (fseek(fp, 4, SEEK_CUR))		// Skip "X-origin of Image, Y-origin of Image"
			break;
		if (fread(&pImage->sizeX, 2, 1, fp) < 1)
			break;
		if (fread(&pImage->sizeY, 2, 1, fp) < 1)
			break;
		if (fread(&pImage->pixelDepth, 1, 1, fp) < 1)
			break;
		if (pImage->pixelDepth != 8 && pImage->pixelDepth != 24 &&
			pImage->pixelDepth != 16 && pImage->pixelDepth != 32) {
			break;
		}
		widthBytes = pImage->pixelDepth / 8;

		if (fread(&ImageDescriptor, 1, 1, fp) < 1)
			break;
		pImage->alphaDepth = ImageDescriptor & 0x0F;
		if (pImage->alphaDepth != 0 && pImage->alphaDepth != 8)
			break;

		if (truecolor) {
			pImage->type = 3;
			if (pImage->pixelDepth == 32)
				pImage->type = 4;
		}
		if (pImage->type == 0)
			break;

		if (fseek(fp, (long)IDLength, SEEK_CUR))
			break;

		pImage->data = (unsigned char *)malloc(pImage->sizeX * pImage->sizeY * widthBytes);
		if (pImage->data == NULL)
			break;

		if (rle) {
			unsigned CurrentPixel = 0;

			TGAError = 0;
			while (CurrentPixel < pImage->sizeX * pImage->sizeY - 1 && TGAError == 0) {
				if (fread(&PacketHeader, 1, 1, fp) < 1) {
					TGAError = 1;
					break;
				}
				PixelCount = (unsigned char)((PacketHeader & 127) + 1);
				if (PacketHeader & 128) {	// RLE
					if (fread(buf, widthBytes, 1, fp) < 1) {
						TGAError = 1;
						break;
					}
					for (i = CurrentPixel; i < CurrentPixel + PixelCount; i++) {
						for (j = 0; j < widthBytes; j++) {
							pImage->data[i * widthBytes + j] = buf[j];
						}
					}
				} else {					// RAW
					if (fread(buf, widthBytes * PixelCount, 1, fp) < 1) {
						TGAError = 1;
						break;
					}
					for (i = CurrentPixel; i < CurrentPixel + PixelCount; i++) {
						for (j = 0; j < widthBytes; j++) {
							pImage->data[i * widthBytes + j] =
								buf[(i - CurrentPixel) * widthBytes + j];
						}
					}
				}
				CurrentPixel += PixelCount;
			}
		} else {
			if (fread(pImage->data, pImage->sizeX * pImage->sizeY * widthBytes, 1, fp) < 1)
				break;
			TGAError = 0;
		}
	} while (0);

	fclose(fp);

	if (TGAError) {
		if (pImage) {
			if (pImage->data)
				free(pImage->data);
			free(pImage);
		}
		return NULL;
	}

	// BGRAからRGBAへ並び換え
	if (pImage->type == 3 || pImage->type == 4) {
		if (pImage->pixelDepth == 24 || pImage->pixelDepth == 32) {
			unsigned char temp;
			unsigned char *pixel = pImage->data;

			for (i = 0; i < pImage->sizeX * pImage->sizeY; i++) {
				temp = *pixel;
				*pixel = *(pixel + 2);
				*(pixel + 2) = temp;
				pixel += widthBytes;
			}
		}
	}

	return pImage;
}

//=======================================================================================
//	End of Files
//=======================================================================================
