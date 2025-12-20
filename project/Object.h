#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <fstream>

#include "ObjectCommon.h"
#include "Calc.h"

class ObjectCommon;
class DirectXCommon;
class DebugCamera;
class WindowAPI;
class Model;


// 座標変換行列データ
struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};
// 平行光源データ
struct DirectionalLight {
	Vector4 color; // ライトの色
	Vector3 direction; // ライトの向き
	float intensity; // 輝度
};

class Object {
public:
	// 初期化
	void Initialize(ObjectCommon* objectCommon,WindowAPI* windowAPI,DirectXCommon* dxCommon);
	// 更新
	void Update();
	// 描画
	void Draw();

	// setter
	void SetModel(Model* model) { model_ = model; }
	void SetScale(const Vector3& scale) { transform.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform.translate = translate; }

	// getter
	const Vector3& GetScale() const { return transform.scale; }
	const Vector3& GetRotate() const { return transform.rotate; }
	const Vector3& GetTranslate() const { return transform.translate; }


private:
	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;

	// バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrixData = nullptr;
	DirectionalLight* directionalLightData = nullptr;

	// Transform
	Transform transform;
	Transform cameraTransform;


	// 3Dオブジェクト共通部
	ObjectCommon* objectCommon_ = nullptr;
	// DirectX共通部
	DirectXCommon* dxCommon_ = nullptr;
	// WindowAPI
	WindowAPI* windowAPI_ = nullptr;
	// モデル
	Model* model_ = nullptr;
	// デバックカメラ
	DebugCamera* debugCamera = nullptr;

};

