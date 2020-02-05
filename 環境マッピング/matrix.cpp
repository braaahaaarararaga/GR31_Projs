#include "matrix.h"

void MatrixRotationYawPitchRoll(MATRIX m, float fYaw, float fPitch, float fRoll)
{
	MATRIX matX, matY, matZ;
	MatrixRotationY(matY, fYaw);
	MatrixRotationX(matX, fPitch);
	MatrixRotationZ(matZ, fRoll);
	MatrixMultiply(m, matY, matX);
	MatrixMultiply(m, m, matZ);
}

void MatrixRotationX(MATRIX m, float fPitch)
{
	m[0][0] = 1.0f;
	m[1][0] = 0.0f;
	m[2][0] = 0.0f;
	m[3][0] = 0.0f;
	m[0][1] = 0.0f;
	m[1][1] = cosf(fPitch);
	m[2][1] = -sinf(fPitch);
	m[3][1] = 0.0f;
	m[0][2] = 0.0f;
	m[1][2] = -m[2][1];
	m[2][2] = m[1][1];
	m[3][2] = 0.0f;
	m[0][3] = 0.0f;
	m[1][3] = 0.0f;
	m[2][3] = 0.0f;
	m[3][3] = 1.0f;
}

void MatrixRotationY(MATRIX m, float fYaw)
{
	m[0][0] = cosf(fYaw);
	m[1][0] = 0.0f;
	m[2][0] = sinf(fYaw);
	m[3][0] = 0.0f;
	m[0][1] = 0.0f;
	m[1][1] = 1.0f;
	m[2][1] = 0.0f;
	m[3][1] = 0.0f;
	m[0][2] = -m[2][0];
	m[1][2] = 0.0f;
	m[2][2] = m[0][0];
	m[3][2] = 0.0f;
	m[0][3] = 0.0f;
	m[1][3] = 0.0f;
	m[2][3] = 0.0f;
	m[3][3] = 1.0f;
}

void MatrixRotationZ(MATRIX m, float fRoll)
{
	m[0][0] = cosf(fRoll);
	m[1][0] = -sinf(fRoll);
	m[2][0] = 0.0f;
	m[3][0] = 0.0f;
	m[0][1] = -m[1][0];
	m[1][1] = m[0][0];
	m[2][1] = 0.0f;
	m[3][1] = 0.0f;
	m[0][2] = 0.0f;
	m[1][2] = 0.0f;
	m[2][2] = 1.0f;
	m[3][2] = 0.0f;
	m[0][3] = 0.0f;
	m[1][3] = 0.0f;
	m[2][3] = 0.0f;
	m[3][3] = 1.0f;
}

void MatrixMultiply(MATRIX m, const MATRIX mL, const MATRIX mR)
{
	MATRIX mat;
	mat[0][0] = mL[0][0] * mR[0][0] + mL[1][0] * mR[0][1] + mL[2][0] * mR[0][2] + mL[3][0] * mR[0][3];
	mat[1][0] = mL[0][0] * mR[1][0] + mL[1][0] * mR[1][1] + mL[2][0] * mR[1][2] + mL[3][0] * mR[1][3];
	mat[2][0] = mL[0][0] * mR[2][0] + mL[1][0] * mR[2][1] + mL[2][0] * mR[2][2] + mL[3][0] * mR[2][3];
	mat[3][0] = mL[0][0] * mR[3][0] + mL[1][0] * mR[3][1] + mL[2][0] * mR[3][2] + mL[3][0] * mR[3][3];
	mat[0][1] = mL[0][1] * mR[0][0] + mL[1][1] * mR[0][1] + mL[2][1] * mR[0][2] + mL[3][1] * mR[0][3];
	mat[1][1] = mL[0][1] * mR[1][0] + mL[1][1] * mR[1][1] + mL[2][1] * mR[1][2] + mL[3][1] * mR[1][3];
	mat[2][1] = mL[0][1] * mR[2][0] + mL[1][1] * mR[2][1] + mL[2][1] * mR[2][2] + mL[3][1] * mR[2][3];
	mat[3][1] = mL[0][1] * mR[3][0] + mL[1][1] * mR[3][1] + mL[2][1] * mR[3][2] + mL[3][1] * mR[3][3];
	mat[0][2] = mL[0][2] * mR[0][0] + mL[1][2] * mR[0][1] + mL[2][2] * mR[0][2] + mL[3][2] * mR[0][3];
	mat[1][2] = mL[0][2] * mR[1][0] + mL[1][2] * mR[1][1] + mL[2][2] * mR[1][2] + mL[3][2] * mR[1][3];
	mat[2][2] = mL[0][2] * mR[2][0] + mL[1][2] * mR[2][1] + mL[2][2] * mR[2][2] + mL[3][2] * mR[2][3];
	mat[3][2] = mL[0][2] * mR[3][0] + mL[1][2] * mR[3][1] + mL[2][2] * mR[3][2] + mL[3][2] * mR[3][3];
	mat[0][3] = mL[0][3] * mR[0][0] + mL[1][3] * mR[0][1] + mL[2][3] * mR[0][2] + mL[3][3] * mR[0][3];
	mat[1][3] = mL[0][3] * mR[1][0] + mL[1][3] * mR[1][1] + mL[2][3] * mR[1][2] + mL[3][3] * mR[1][3];
	mat[2][3] = mL[0][3] * mR[2][0] + mL[1][3] * mR[2][1] + mL[2][3] * mR[2][2] + mL[3][3] * mR[2][3];
	mat[3][3] = mL[0][3] * mR[3][0] + mL[1][3] * mR[3][1] + mL[2][3] * mR[3][2] + mL[3][3] * mR[3][3];
	memcpy(m, mat, sizeof mat);
}

void MatrixIdentity(MATRIX m)
{
	m[0][0] = 1.0f;
	m[1][0] = 0.0f;
	m[2][0] = 0.0f;
	m[3][0] = 0.0f;
	m[0][1] = 0.0f;
	m[1][1] = 1.0f;
	m[2][1] = 0.0f;
	m[3][1] = 0.0f;
	m[0][2] = 0.0f;
	m[1][2] = 0.0f;
	m[2][2] = 1.0f;
	m[3][2] = 0.0f;
	m[0][3] = 0.0f;
	m[1][3] = 0.0f;
	m[2][3] = 0.0f;
	m[3][3] = 1.0f;
}

void MatrixTranslation(MATRIX m, float x, float y, float z)
{
	m[0][0] = 1.0f;
	m[1][0] = 0.0f;
	m[2][0] = 0.0f;
	m[3][0] = x;
	m[0][1] = 0.0f;
	m[1][1] = 1.0f;
	m[2][1] = 0.0f;
	m[3][1] = y;
	m[0][2] = 0.0f;
	m[1][2] = 0.0f;
	m[2][2] = 1.0f;
	m[3][2] = z;
	m[0][3] = 0.0f;
	m[1][3] = 0.0f;
	m[2][3] = 0.0f;
	m[3][3] = 1.0f;
}

void VectorTransformCoord(VECTOR v, const MATRIX mL, const VECTOR vR)
{
	VECTOR vec;
	vec[0] = mL[0][0] * vR[0] + mL[1][0] * vR[1] + mL[2][0] * vR[2] + mL[3][0] * vR[3];
	vec[1] = mL[0][1] * vR[0] + mL[1][1] * vR[1] + mL[2][1] * vR[2] + mL[3][1] * vR[3];
	vec[2] = mL[0][2] * vR[0] + mL[1][2] * vR[1] + mL[2][2] * vR[2] + mL[3][2] * vR[3];
	vec[3] = mL[0][3] * vR[0] + mL[1][3] * vR[1] + mL[2][3] * vR[2] + mL[3][3] * vR[3];
	v[0] = vec[0];
	v[1] = vec[1];
	v[2] = vec[2];
	v[3] = vec[3];
}
