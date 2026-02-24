#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <fstream>

#include <D3d12.h>
#include <cassert>
#include <wrl.h>
#include <dxcapi.h>

#include "Calc.h"

class Model;
class Camera;
class DirectXCommon;

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
	int isDisplay; // 表示するかどうか
};
// 環境光データ
struct AmbientLight {
	Vector4 color; // ライトの色
	float intensity; // 輝度
	int isDisplay; // 表示するかどうか
};


// アウトラインデータ
struct Outline {
	float thickness; // 太さ
	Vector4 color; // 色
	float padding[3];   // 16バイト合わせ（重要）
};

class Object {
public:
	// 初期化
	void Initialize(Camera* camera);
	// 更新
	void Update();
	// 描画
	void Draw();

	// setter
	void SetModel(Model* model) { model_ = model; }
	void SetModel(const std::string& filePath);
	void SetScale(const Vector3& scale) { transform.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform.translate = translate; }
	void SetCamera(Camera* camera) { camera_ = camera; }
	void SetOutlineThickness(float thickness) { outlineData->thickness = thickness; }
	void SetOutlineColor(Vector4 color) { outlineData->color = color; }

	// getter
	const Vector3& GetScale() const { return transform.scale; }
	const Vector3& GetRotate() const { return transform.rotate; }
	const Vector3& GetTranslate() const { return transform.translate; }


private:
	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> ambientLightResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> outlineResource;

	// バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrixData = nullptr;
	DirectionalLight* directionalLightData = nullptr;
	AmbientLight* ambientLightData = nullptr;
	Outline* outlineData = nullptr;

	// Transform
	Transform transform;
	Transform cameraTransform;


	// モデル
	Model* model_ = nullptr;
	// カメラ
	Camera* camera_ = nullptr;
	// DirectXCommonのポインタ
	DirectXCommon* dxCommon_ = nullptr;

};

