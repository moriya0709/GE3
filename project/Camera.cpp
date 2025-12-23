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

	// Transformの更新
	//Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	//Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	//Matrix4x4 viewMatrix = Inverse(cameraMatrix);
	//viewMatrix = debugCamera->GetViewMatrix(); // デバッグカメラのビュー行列を取得
	//Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(windowAPI_->kClientWidth) / float(windowAPI_->kClientHeight), 0.1f, 100.0f);
	//transformationMatrixData->WVP = worldMatrix * viewMatrix * projectionMatrix;   // WVP行列を設定
	//transformationMatrixData->World = worldMatrix; // World行列を設定
}
