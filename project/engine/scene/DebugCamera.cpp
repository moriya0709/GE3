#include "DebugCamera.h"
#include <cmath>

void DebugCamera::Initialize()
{
	// �ݐω�]�s��
	Matrix4x4 matRotX = MakeRotateXMatrix(0.0f);
	Matrix4x4 matRotY = MakeRotateYMatrix(0.0f);
	Matrix4x4 matRotZ = MakeRotateZMatrix(0.0f);
	// ��]�s��̍���
	matRot_ = matRotZ * matRotX * matRotY;

	// ���[�J�����W
	translation_ = { 0.0f,0.0f,-5.0f };
}

void DebugCamera::Update(HWND hwnd)
{
	POINT mousePosition;
	GetCursorPos(&mousePosition);
	ScreenToClient(hwnd, &mousePosition);

	// ����̂݊���W���L�^
	static bool firstFrame = true;
	if (firstFrame)
	{
		prevMouseX = mousePosition.x;
		prevMouseY = mousePosition.y;
		firstFrame = false;
	}

	// �E�N���b�N���݈̂ړ�
	if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
	{
		const float moveSpeed = 0.002f; // ���x
		translation_.y += (mousePosition.y - prevMouseY) * moveSpeed;
		translation_.x += (mousePosition.x - prevMouseX) * -moveSpeed;
	}

	// ���N���b�N���̂݉�]
	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
	{
		const float rotateSpeed = -0.002f; // ���x
		
		// �ǉ���]���̉�]�s��𐶐�
		Matrix4x4 matRotDelta = MakeIdentityMatrix();
		matRotDelta *= MakeRotateXMatrix((mousePosition.y - prevMouseY) * rotateSpeed);
		matRotDelta *= MakeRotateYMatrix((mousePosition.x - prevMouseX) * rotateSpeed);
		
		// �ݐς̉�]�s�������
		matRot_ = matRotDelta * matRot_;
	}

	// �}�E�X�ʒu�X�V
	prevMouseX = mousePosition.x;
	prevMouseY = mousePosition.y;

	Vector3 target = { 0.0f, 0.0f, 0.0f }; // ���f���̒��S���W

	// ���f�����S�ɉ�]�������킹��
	Matrix4x4 matTranslateToOrigin = MakeTranslateMatrix(-target);
	Matrix4x4 matTranslateBack = MakeTranslateMatrix(target);

	Matrix4x4 cameraMatrix_ = matTranslateBack * matRot_ * matTranslateToOrigin;

	// �J�����̈ʒu�𔽉f�i��������ނ�����j
	Matrix4x4 matCameraTranslate = MakeTranslateMatrix(translation_);
	cameraMatrix_ = matCameraTranslate * cameraMatrix_;

	viewMatrix_ = Inverse(cameraMatrix_);

}

// ���s�ړ��s��
Matrix4x4 DebugCamera::MakeTranslateMatrix(const Vector3& translate)
{
	Matrix4x4 result = {};

	result.m[0][0] = 1.0f; // X�X�P�[��
	result.m[1][1] = 1.0f; // Y�X�P�[��
	result.m[2][2] = 1.0f; // Z�X�P�[��
	result.m[3][0] = translate.x; // X���s�ړ�
	result.m[3][1] = translate.y; // Y���s�ړ�
	result.m[3][2] = translate.z; // Z���s�ړ�
	result.m[3][3] = 1.0f;        // �������W

	return result;
}

// �g��k���s��
Matrix4x4 DebugCamera::MakeScaleMatrix(const Vector3& scale)
{
	Matrix4x4 result = {};

	result.m[0][0] = scale.x; // X�X�P�[��
	result.m[1][1] = scale.y; // Y�X�P�[��
	result.m[2][2] = scale.z; // Z�X�P�[��
	result.m[3][3] = 1.0f;    // �������W

	return result;
}

// x����]�s��
Matrix4x4 DebugCamera::MakeRotateXMatrix(float radian)
{
	Matrix4x4 result = {};
	result.m[0][0] = 1.0f;
	result.m[1][1] = std::cos(radian);
	result.m[1][2] = -std::sin(radian);
	result.m[2][1] = std::sin(radian);
	result.m[2][2] = std::cos(radian);
	result.m[3][3] = 1.0f;
	return result;
}

// y����]�s��
Matrix4x4 DebugCamera::MakeRotateYMatrix(float radian)
{
	Matrix4x4 result = {};
	result.m[0][0] = std::cos(radian);
	result.m[0][2] = std::sin(radian);
	result.m[1][1] = 1.0f;
	result.m[2][0] = -std::sin(radian);
	result.m[2][2] = std::cos(radian);
	result.m[3][3] = 1.0f;
	return result;
}

// z����]�s��
Matrix4x4 DebugCamera::MakeRotateZMatrix(float radian)
{
	Matrix4x4 result = {};
	result.m[0][0] = std::cos(radian);
	result.m[0][1] = -std::sin(radian);
	result.m[1][0] = std::sin(radian);
	result.m[1][1] = std::cos(radian);
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;
	return result;
}


// �t�s��
Matrix4x4 DebugCamera::Inverse(const Matrix4x4& m)
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

// �������e�s��
Matrix4x4 DebugCamera::MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip)
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

Matrix4x4 DebugCamera::MakeIdentityMatrix()
{
	static const Matrix4x4 result{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	return result;
}

// 3�����A�t�B���ϊ��s��
Matrix4x4 DebugCamera::MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate)
{
	Matrix4x4 result = {};

	// �g��k���s��𐶐�����
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);

	// ��]�s��𐶐�����
	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);
	Matrix4x4 rotateXYZMatrix = Multiply(rotateXMatrix, Multiply(rotateYMatrix, rotateZMatrix));

	// ���s�ړ��s��𐶐�����
	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);

	// ���v
	result = Multiply(Multiply(scaleMatrix, rotateXYZMatrix), translateMatrix);

	return result;
}

