#include "Object.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "Model.h"
#include "ModelManager.h"
#include "Camera.h"
#include "CameraManager.h"

void Object::Initialize(ObjectCommon* objectCommon, WindowAPI* windowAPI, DirectXCommon* dxCommon) {
	// 引数で受け取ってメンバ変数に記録する
	objectCommon_ = objectCommon;
	windowAPI_ = windowAPI;
	dxCommon_ = dxCommon;

	// デフォルトカメラをセット
	camera_ = objectCommon_->GetDefaultCamera();

	
	// *座標変換行列* //
	transformationMatrixResource = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	// 書き込む為のアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	// 単位行列を書き込んでおく
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();

	// *平行光源* //

	// リソース
	directionalLightResource = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
	// 書き込む
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	// 初期値を書き込む
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;
	directionalLightResource->Unmap(0, nullptr);

	// *Transform* //
	transform = {
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};
	cameraTransform = {
		{1.0f,1.0f,1.0f},
		{0.3f,0.0f,0.0f},
		{0.0f,4.0f,-10.0f}
	};

}

void Object::Update() {
	// Transformの更新
	camera_ = CameraManager::GetInstance()->GetActiveCamera();

	Matrix4x4 world =
		MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	Matrix4x4 wvp =
		Multiply(world, camera_->GetViewProjectionMatrix());

	transformationMatrixData->WVP = wvp;
	transformationMatrixData->World = world;
}

void Object::Draw() {
	// wvp用とWorld用のCBufferの場所を設定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	// 平行光源
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

	// 3Dモデルが割り当てられていれば描画する
	if (model_) {
		model_->Draw();
	}

}

void Object::SetModel(const std::string& filePath) {
	// モデルを検索してセットする
	model_ = ModelManager::GetInstance()->FindModel(filePath);
}
