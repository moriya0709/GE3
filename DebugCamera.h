#pragma once
#include <windows.h>
#include "Math.h"


class DebugCamera
{
public:

	// ������
	void Initialize();
	// �X�V
	void Update(HWND hwnd);

	// �r���[�s����擾
	Matrix4x4 GetViewMatrix() { return viewMatrix_; };

private:
	// �ݐω�]�s��
	Matrix4x4 matRot_ = {};
	// ���[�J�����W
	Vector3 translation_ = { 0,0,-5 };
	// �r���[�s��
	Matrix4x4 viewMatrix_ = {};
	// �ˉe�s��
	Matrix4x4 projectionMatrix = {};
	//�N���C�A���g�̈�̃T�C�Y
	const int kClientWidth = 1280;
	const int kClientHeight = 720;

	// �O��̃}�E�X���W
	int prevMouseX = 0;
	int prevMouseY = 0;

	// �t�s��
	Matrix4x4 Inverse(const Matrix4x4& m);
	// �������e�s��
	Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

	Matrix4x4 MakeIdentityMatrix();
	Matrix4x4 MakeScaleMatrix(const Vector3& scale);
	Matrix4x4 MakeRotateXMatrix(float theta);
	Matrix4x4 MakeRotateYMatrix(float theta);
	Matrix4x4 MakeRotateZMatrix(float theta);
	Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rot, const Vector3& translate);
};

