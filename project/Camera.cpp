#include "Camera.h"
#include "WindowAPI.h"

Camera::Camera()
	: transform({{1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f}})
	, fovY_(0.45f)
	, aspectRatio_(float(WindowAPI::kClientWidth) / float (WindowAPI::kClientHeight))
	, nearClip_(0.1f)
	, farClip_(100.0f)
	, worldMatrix(MakeAffineMatrix(transform.scale, transform.rotate, transform.translate))
	, viewMatrix(Inverse(worldMatrix))
	, projectionMatrix(MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_))
	, viewProjectionMatrix(Multiply(viewMatrix, projectionMatrix))
{}

void Camera::Update() {
	// ビュー行列
	worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	viewMatrix = Inverse(worldMatrix);

	// プロジェクション行列
	projectionMatrix = MakePerspectiveFovMatrix(fovY_,aspectRatio_,nearClip_,farClip_);

	// 合成行列
	viewProjectionMatrix = Multiply(viewMatrix,projectionMatrix);

}
