#pragma once

/// AL3�T���v���v���O�����p�̐��w���C�u�����B
/// MT3�����ŁAKamataEngine�����̐��w���C�u�����Əd������B

struct Matrix4x4 final {
    float m[4][4];
};

struct Vector4 final {
    float x;
    float y;
    float z;
    float w;
};

struct Vector3 final {
    float x;
    float y;
    float z;
};

struct Vector2 final
{
    float x;
    float y;
};

// �~����
const float PI = 3.141592654f;

struct AABB
{
    Vector3 min;
    Vector3 max;
};

// 02_14 29���� �P�����Z�q�I�[�o�[���[�h
Vector3 operator+(const Vector3& v);
Vector3 operator-(const Vector3& v);

// 02_06��CameraController��Update/Reset�֐��ŕK�v
const Vector3 operator+(const Vector3& lhv, const Vector3& rhv);

// 02_06�̃X���C�h24���ڂ�Lerp�֐�
Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);

// 02_06 �X���C�h29���ڂŒǉ�
const Vector3 operator*(const Vector3& v1, const float f);

// ������Z�q�I�[�o�[���[�h
Vector3& operator+=(Vector3& lhs, const Vector3& rhv);
Vector3& operator-=(Vector3& lhs, const Vector3& rhv);
Vector3& operator*=(Vector3& v, float s);
Vector3& operator/=(Vector3& v, float s);

Vector3 operator/(const Vector3& v, float scalar);

// �s��̏�Z
Matrix4x4 Multiply(Matrix4x4 matrix1, Matrix4x4 matrix2);

// �P�ʍs��̍쐬
Matrix4x4 MakeIdentityMatrix();
// �X�P�[�����O�s��̍쐬
Matrix4x4 MakeScaleMatrix(const Vector3& scale);
// ��]�s��̍쐬
Matrix4x4 MakeRotateXMatrix(float theta);
Matrix4x4 MakeRotateYMatrix(float theta);
Matrix4x4 MakeRotateZMatrix(float theta);
// ���s�ړ��s��̍쐬
Matrix4x4 MakeTranslateMatrix(const Vector3& translate);
// �A�t�B���ϊ��s��̍쐬
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rot, const Vector3& translate);
// �ݐς̉�]�s��̏ꍇ
Matrix4x4 MakeAffineMatrixR(const Vector3& scale, const Matrix4x4& matRot, const Vector3& translate);
// �������e�s��
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);
// ���s�ˉe�s��
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);
// �r���[�{�[�g�ϊ��s��
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

// �t�s��
Matrix4x4 Inverse(const Matrix4x4& m);

// ������Z�q�I�[�o�[���[�h
Matrix4x4& operator*=(Matrix4x4& lhm, const Matrix4x4& rhm);

// 2�����Z�q�I�[�o�[���[�h
Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2);

float Lerp(float x1, float x2, float t);

float EaseIn(float x1, float x2, float t);

float EaseOut(float x1, float x2, float t);

float EaseInOut(float x1, float x2, float t);

bool IsCollision(const AABB& aabb1, const AABB& aabb2);