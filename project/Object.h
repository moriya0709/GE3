#pragma once
#include <Windows.h>
#include <string>
#include <vector>

#include "ObjectCommon.h"
#include "Calc.h"

class ObjectCommon;

struct MaterialData {
	std::string textureFilePath;
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};
// 頂点データ
struct VertexData {
	Vector4 position; // 頂点座標
	Vector2 texcoord; // テクスチャ座標
	Vector3 normal; // 正規化座標
};



class Object {
public:
	// 初期化
	void Initialize(ObjectCommon* objectCommon);

	// .mtlファイルの読み込み
	static MaterialData LoadMaterialTemplateFile(const std::string& directorypath, const std::string& filename);
	// .objファイルの読み込み
	static ModelData LoadObjFile(const std::string& dirextorypath, const std::string& filename);

private:
	// Objファイルのデータ
	ModelData modelData;

	// *頂点データ* //
	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;


	// 3Dオブジェクト共通部
	ObjectCommon* objectCommon_ = nullptr;

};

