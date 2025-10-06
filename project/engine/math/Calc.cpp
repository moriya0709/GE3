#include "Calc.h"
#include <cmath>
#include <numbers>

// 02_14 29���� �P�����Z�q�I�[�o�[���[�h
Vector3 operator+(const Vector3& v) { return v; }
Vector3 operator-(const Vector3& v) { return Vector3(-v.x, -v.y, -v.z); }

// 02_06��29����(CameraController��Update)�ŕK�v
const Vector3 operator*(const Vector3& v1, const float f)
{
	Vector3 temp(v1);
	return temp *= f;
}

// 02_06��CameraController��Update/Reset�֐��ŕK�v
const Vector3 operator+(const Vector3& v1, const Vector3& v2)
{
	Vector3 temp(v1);
	return temp += v2;
}

// 02_06�̃X���C�h24���ڂ�Lerp�֐�
Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t) { return Vector3(Lerp(v1.x, v2.x, t), Lerp(v1.y, v2.y, t), Lerp(v1.z, v2.z, t)); }

Vector3& operator+=(Vector3& lhv, const Vector3& rhv)
{
	lhv.x += rhv.x;
	lhv.y += rhv.y;
	lhv.z += rhv.z;
	return lhv;
}

Vector3& operator-=(Vector3& lhv, const Vector3& rhv)
{
	lhv.x -= rhv.x;
	lhv.y -= rhv.y;
	lhv.z -= rhv.z;
	return lhv;
}

Vector3& operator*=(Vector3& v, float s)
{
	v.x *= s;
	v.y *= s;
	v.z *= s;
	return v;
}

Vector3& operator/=(Vector3& v, float s)
{
	v.x /= s;
	v.y /= s;
	v.z /= s;
	return v;
}

Vector3 operator/(const Vector3& v, float scalar)
{
	return Vector3{ v.x / scalar, v.y / scalar, v.z / scalar };
}

// �s��̏�Z
Matrix4x4 Multiply(Matrix4x4 matrix1, Matrix4x4 matrix2)
{
	Matrix4x4 result = {};

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				result.m[i][j] += matrix1.m[i][k] * matrix2.m[k][j];
			}
		}
	}

	return result;
}


Matrix4x4 MakeIdentityMatrix()
{
	static const Matrix4x4 result{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	return result;
}

Matrix4x4 MakeScaleMatrix(const Vector3& scale)
{

	Matrix4x4 result{ scale.x, 0.0f, 0.0f, 0.0f, 0.0f, scale.y, 0.0f, 0.0f, 0.0f, 0.0f, scale.z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	return result;
}

Matrix4x4 MakeRotateXMatrix(float theta)
{
	float sin = std::sin(theta);
	float cos = std::cos(theta);

	Matrix4x4 result{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, cos, sin, 0.0f, 0.0f, -sin, cos, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	return result;
}

Matrix4x4 MakeRotateYMatrix(float theta)
{
	float sin = std::sin(theta);
	float cos = std::cos(theta);

	Matrix4x4 result{ cos, 0.0f, -sin, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, sin, 0.0f, cos, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	return result;
}

Matrix4x4 MakeRotateZMatrix(float theta)
{
	float sin = std::sin(theta);
	float cos = std::cos(theta);

	Matrix4x4 result{ cos, sin, 0.0f, 0.0f, -sin, cos, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	return result;
}

Matrix4x4 MakeTranslateMatrix(const Vector3& translate)
{
	Matrix4x4 result{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, translate.x, translate.y, translate.z, 1.0f };

	return result;
}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rot, const Vector3& translate)
{

	// �X�P�[�����O�s��̍쐬
	Matrix4x4 matScale = MakeScaleMatrix(scale);

	Matrix4x4 matRotX = MakeRotateXMatrix(rot.x);
	Matrix4x4 matRotY = MakeRotateYMatrix(rot.y);
	Matrix4x4 matRotZ = MakeRotateZMatrix(rot.z);
	// ��]�s��̍���
	Matrix4x4 matRot = matRotZ * matRotX * matRotY;

	// ���s�ړ��s��̍쐬
	Matrix4x4 matTrans = MakeTranslateMatrix(translate);

	// �X�P�[�����O�A��]�A���s�ړ��̍���
	Matrix4x4 matTransform = matScale * matRot * matTrans;

	return matTransform;
}

// �ݐς̉�]�s��̏ꍇ
Matrix4x4 MakeAffineMatrixR(const Vector3& scale, const Matrix4x4& matRot, const Vector3& translate)
{

	// �X�P�[�����O�s��̍쐬
	Matrix4x4 matScale = MakeScaleMatrix(scale);

	// ���s�ړ��s��̍쐬
	Matrix4x4 matTrans = MakeTranslateMatrix(translate);

	// �X�P�[�����O�A��]�A���s�ړ��̍���
	Matrix4x4 matTransform = matScale * matRot * matTrans;

	return matTransform;
}


// �������e�s��
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip)
{
	Matrix4x4 result = {};

	float yScale = 1.0f / std::tan(fovY * 0.5f);
	float xScale = yScale / aspectRatio;

	result.m[0][0] = xScale;
	result.m[1][1] = yScale;
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[2][3] = 1.0f;
	result.m[3][2] = -nearClip * farClip / (farClip - nearClip);
	result.m[3][3] = 0.0f;

	return result;
}

// ���s�ˉe�s��
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip)
{
	Matrix4x4 result = {};

	result.m[0][0] = 2.0f / (right - left);
	result.m[1][1] = 2.0f / (top - bottom);
	result.m[2][2] = 1.0f / (farClip - nearClip);
	result.m[3][0] = (left + right) / (left - right);
	result.m[3][1] = (top + bottom) / (bottom - top);
	result.m[3][2] = nearClip / (nearClip - farClip);
	result.m[3][3] = 1.0f;

	return result;
}

// �r���[�{�[�g�ϊ��s��
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth)
{
	Matrix4x4 m = {};
	m.m[0][0] = width / 2.0f;
	m.m[1][1] = -height / 2.0f; // Y���W�͔��]
	m.m[2][2] = maxDepth - minDepth;
	m.m[3][0] = left + width / 2.0f;
	m.m[3][1] = top + height / 2.0f;
	m.m[3][2] = minDepth;
	m.m[3][3] = 1.0f;
	return m;
}


// �t�s��
Matrix4x4 Inverse(const Matrix4x4& m)
{
	Matrix4x4 result = {};

	// ��3x3�����i��]�E�g��k���j�̋t�s��
	float det =
		m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]) -
		m.m[0][1] * (m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0]) +
		m.m[0][2] * (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]);
	float invDet = 1.0f / det;

	// 3x3�����̋t�s��i�N�������̌����j
	result.m[0][0] = (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]) * invDet;
	result.m[0][1] = -(m.m[0][1] * m.m[2][2] - m.m[0][2] * m.m[2][1]) * invDet;
	result.m[0][2] = (m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1]) * invDet;

	result.m[1][0] = -(m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0]) * invDet;
	result.m[1][1] = (m.m[0][0] * m.m[2][2] - m.m[0][2] * m.m[2][0]) * invDet;
	result.m[1][2] = -(m.m[0][0] * m.m[1][2] - m.m[0][2] * m.m[1][0]) * invDet;

	result.m[2][0] = (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]) * invDet;
	result.m[2][1] = -(m.m[0][0] * m.m[2][1] - m.m[0][1] * m.m[2][0]) * invDet;
	result.m[2][2] = (m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0]) * invDet;

	// ���s�ړ������̋t�ϊ�
	result.m[3][0] = -(result.m[0][0] * m.m[3][0] + result.m[1][0] * m.m[3][1] + result.m[2][0] * m.m[3][2]);
	result.m[3][1] = -(result.m[0][1] * m.m[3][0] + result.m[1][1] * m.m[3][1] + result.m[2][1] * m.m[3][2]);
	result.m[3][2] = -(result.m[0][2] * m.m[3][0] + result.m[1][2] * m.m[3][1] + result.m[2][2] * m.m[3][2]);
	result.m[3][3] = 1.0f;

	return result;
}

Matrix4x4& operator*=(Matrix4x4& lhm, const Matrix4x4& rhm)
{
	Matrix4x4 result{};

	for (size_t i = 0; i < 4; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			for (size_t k = 0; k < 4; k++)
			{
				result.m[i][j] += lhm.m[i][k] * rhm.m[k][j];
			}
		}
	}
	lhm = result;
	return lhm;
}

Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2)
{
	Matrix4x4 result = m1;

	return result *= m2;
}

float Lerp(float x1, float x2, float t) { return (1.0f - t) * x1 + t * x2; }

float EaseIn(float x1, float x2, float t)
{
	float easedT = t * t;

	return Lerp(x1, x2, easedT);
}

float EaseOut(float x1, float x2, float t)
{
	float easedT = 1.0f - std::powf(1.0f - t, 3.0f);

	return Lerp(x1, x2, easedT);
}

float EaseInOut(float x1, float x2, float t)
{
	float easedT = -(std::cosf(std::numbers::pi_v<float> *t) - 1.0f) / 2.0f;

	return Lerp(x1, x2, easedT);
}

bool IsCollision(const AABB& aabb1, const AABB& aabb2)
{
	return (aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x) && // x��
		(aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y) && // y��
		(aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z);   // z��
}