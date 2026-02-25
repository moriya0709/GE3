#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <cstdint>
#include <dxcapi.h>
#include <memory>

#include "Calc.h"

class DirectXCommon;
class WindowAPI;

struct RenderTarget {
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{};
	D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle{};
	D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle{};
};

struct EffectData {
	int isInversion; // 色反転
	int isGrayscale; // モノクロ
	int isRadialBlur; // 放射状ブラー
	int isDistanceFog; // フォグ
	float intensity; // 全体の強さ
	Vector2 blurCenter; // ブラーの中心 (通常は 0.5, 0.5)
	float blurWidth; // ブラーの強さ (0.01～0.1程度)
	int blurSamples; // サンプリング数 (10～20程度)


	// フォグ用のパラメータ
	Vector3 distanceFogColor; // フォグの色
	float distanceFogStart; // フォグが始まる距離
	float distanceFogEnd; // 完全にフォグに覆われる距離
	float pad1; // アライメント用

	float zNear; // カメラのニアクリップ面

	float zFar; // カメラのファークリップ面
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

	// フォグ
	void SetDistanceFog(bool isFog) { effectData->isDistanceFog = isFog; }
	void SetDistanceFogColor(const Vector3& color) { effectData->distanceFogColor = color; }
	void SetDistanceFogStart(float start) { effectData->distanceFogStart = start; }
	void SetDistanceFogEnd(float end) { effectData->distanceFogEnd = end; }


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

