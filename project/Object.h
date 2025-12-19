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

// テクスチャ
struct MaterialData {
	std::string textureFilePath;
	uint32_t textureIndex = 0;
};
// 頂点データ
struct VertexData {
	Vector4 position; // 頂点座標
	Vector2 texcoord; // テクスチャ座標
	Vector3 normal; // 正規化座標
};
// モデルデータ
struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};
// マテリアルデータ
struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};
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

	// .mtlファイルの読み込み
	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
	// .objファイルの読み込み
	static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

private:
	// Objファイルのデータ
	ModelData modelData;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;

	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	Material* materialData = nullptr;
	TransformationMatrix* transformationMatrixData = nullptr;
	DirectionalLight* directionalLightData = nullptr;

	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	// Transform
	Transform transform;
	Transform cameraTransform;


	// 3Dオブジェクト共通部
	ObjectCommon* objectCommon_ = nullptr;
	// DirectX共通部
	DirectXCommon* dxCommon_ = nullptr;
	// WindowAPI
	WindowAPI* windowAPI_ = nullptr;
	// デバックカメラ
	DebugCamera* debugCamera = nullptr;

};

