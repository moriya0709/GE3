#pragma once
#include "string"
#include <dxgi1_6.h>

#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"

class DirectXCommon;

class TextureManager {
public:
	// テクスチャ1枚分のデータ
	struct TextureData {
		std::string filePath;
		DirectX::TexMetadata metadata;
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
		Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
	};

	// テクスチャデータ
	std::vector<TextureData> textureDatas;

	// 初期化
	void Initialize(DirectXCommon* dxCommon_);
	// シングルトンインスタンスの取得
	static TextureManager* GetInstance();
	// 終了
	void Finalize();

	// テクスチャファイルの読み込み
	void LoadTexture(const std::string& filePath);
	// SRVインデックスの開始番号
	uint32_t GetTextureIndexByFilePath(const std::string& filePath);
	// テクスチャ番号からGPUハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureIndex);

private:

	static TextureManager* instance;
	// SRVインデックスの開始番号
	static uint32_t kSRVIndexTop;

	DirectXCommon* dxCommon = nullptr;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = delete;
	TextureManager& operator=(TextureManager&) = delete;
};

