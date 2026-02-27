#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <cstdint>
#include <dxcapi.h>
#include <memory>

#include "Calc.h"

class DirectXCommon;
class WindowAPI;
class Camera;

struct RenderTarget {
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{};
	D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle{};
	D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle{};
};

struct EffectData {
	// [Block 0] 16 bytes
	int isInversion;
	int isGrayscale;
	int isRadialBlur;
	int isDistanceFog;

	// [Block 1] 16 bytes
	int isHeightFog;
	float intensity;
	float pad0[2]; // 16バイトに合わせるための詰め物

	// [Block 2] 16 bytes
	Vector2 blurCenter;
	float blurWidth;
	int blurSamples;

	// [Block 3] 16 bytes
	Vector3 distanceFogColor;
	float distanceFogStart;

	// [Block 4] 16 bytes
	float distanceFogEnd;
	float zNear;
	float zFar;
	float pad1; // 16バイトに合わせる

	// [Block 5] 16 bytes
	Vector3 heightFogColor;
	float heightFogTop;

	// [Block 6] 16 bytes
	float heightFogBottom;
	float heightFogDensity;
	float pad2[2]; // ここで行列の開始位置を16バイトの倍数に強制する

	// [Block 7-10] 64 bytes
	Matrix4x4 matInverseViewProjection;

	// [Padding to 256 bytes]
	float finalPad[28];
};

class PostEffect {
public:
	// 初期化
	void Initialize(DirectXCommon* dxCommon,WindowAPI* windowAPI);
	// 描画
	void Draw();

	// 描画前処理
	void PreDraw();
	// 描画後処理
	void PostDraw();

	// エフェクトの有効化
	void SetInversion(bool isInversion) { effectData->isInversion = isInversion; }
	void SetGrayscale(bool isGrayscale) { effectData->isGrayscale = isGrayscale; }
	void SetIntensity(float intensity) { effectData->intensity = intensity; }

	// ディスタンスフォグ
	void SetDistanceFog(bool isFog) { effectData->isDistanceFog = isFog; }
	void SetDistanceFogColor(const Vector3& color) { effectData->distanceFogColor = color; }
	void SetDistanceFogStart(float start) { effectData->distanceFogStart = start; }
	void SetDistanceFogEnd(float end) { effectData->distanceFogEnd = end; }
	
	// ハイトフォグ
	void SetHeightFog(bool isFog) { effectData->isHeightFog = isFog; }
	void SetHeightFogColor(const Vector3& color) { effectData->heightFogColor = color; }
	void SetHeightFogTop(float top) { effectData->heightFogTop = top; }
	void SetHeightFogBottom(float bottom) { effectData->heightFogBottom = bottom; }
	void SetHeightFogDensity(float density) { effectData->heightFogDensity = density; }
	void SetInverseViewProjectionMatrix(const Matrix4x4& mat) { effectData->matInverseViewProjection = mat; }
	void HightFogUpdate(Camera* camera); // カメラの位置からハイトフォグ用の逆行列を計算してセットする関数

	// getter
	Matrix4x4 GetInverseViewProjectionMatrix() {
		return effectData->matInverseViewProjection;
	}

	// シングルトンインスタンスの取得
	static PostEffect* GetInstance();

private:
	// ルートシグネイチャ
	Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = nullptr;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = nullptr;

	// グラフィックスパイプライン
	Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicsPipelineState = nullptr;

	// レンダーターゲット
	RenderTarget renderTarget_;
	// ビューポート
	D3D12_VIEWPORT viewport_;
	// シザー矩形
	D3D12_RECT scissorRect_;

	// クリアカラー
	float clearColor[4] = { 0.1f, 0.25f, 0.5f, 1.0f };
	D3D12_RESOURCE_STATES currentState_;

	// *エフェクト切り換え用* //
	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> effectResource;
	// エフェクトデータ
	EffectData* effectData = nullptr;


	// シングルトンインスタンス
	static std::unique_ptr <PostEffect> instance;

	// ポインター
	DirectXCommon* dxCommon_ = nullptr;
	WindowAPI* windowAPI_ = nullptr;	

	// レンダーターゲットの生成
	RenderTarget CreateRenderTarget(
		ID3D12Device* device,
		uint32_t width,
		uint32_t height,
		DXGI_FORMAT format,
		const float clearColor[4],
		ID3D12DescriptorHeap* rtvHeap,
		UINT rtvIndex,
		ID3D12DescriptorHeap* srvHeap,
		UINT srvIndex
	);

	// リソースバリアの発行
	void Transition(D3D12_RESOURCE_STATES newState);
	// バックバッファを指定の状態に遷移
	void TransitionBackBuffer(D3D12_RESOURCE_STATES newState);
	// 深度バッファを指定の状態に遷移
	void TransitionDepthBuffer(D3D12_RESOURCE_STATES newState);

	// ルートシグネイチャ生成
	void CreateRootSignature();
	// グラフィックスパイプライン生成
	void CreateGraphicsPipeline();

	// ビューポート初期化
	void InitializeViewport();
	// シザリング矩形初期化
	void InitializeScissorRect();
};

