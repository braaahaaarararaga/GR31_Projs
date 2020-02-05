#ifndef _MATRIX_H
#define _MATRIX_H

#include <math.h>
#include <string.h>

typedef float MATRIX[4][4];
typedef float VECTOR[4];

void MatrixRotationYawPitchRoll(MATRIX m, float fYaw, float fPitch, float fRoll);
void MatrixRotationX(MATRIX m, float fPitch);
void MatrixRotationY(MATRIX m, float fYaw);
void MatrixRotationZ(MATRIX m, float fRoll);
void MatrixMultiply(MATRIX m, const MATRIX mL, const MATRIX mR);
void MatrixIdentity(MATRIX m);
void MatrixTranslation(MATRIX m, float x, float y, float z);
void VectorTransformCoord(VECTOR v, const MATRIX mL, const VECTOR vR);

#endif	// _MATRIX_H