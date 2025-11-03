#pragma once

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <array>
#include <dxcapi.h>
#include <debugapi.h>
#include <format>

#include "WindowAPI.h"
#include "Logger.h"

#include "externals/imgui\imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

using namespace Logger;

class DirectXCommon {
public:
	void Initialize(WindowAPI* windowAPI); // 初期化

	void CreateDevice(); // デバイス関連
	void CreateCommandList(); // コマンドリスト関連
	void CreateSwapChain(); // スワップチェイン関連
	void CreateDepth(); // 深度バッファ関連
	void CreateDescriptor(); // デスクリプタヒープ関連
	void CreateDxcCompiler(); // DXCコンパイラの生成

	void InitializeRTV(); // レンダーターゲットビューの初期化
	void InitializeDSV(); // 深度ステンシルビューの初期化
	void InitializeFence(); // フェンスの初期化
	void InitializeViewport(); // ビューポート矩形の初期化
	void InitializeScissorRect(); // シザリング矩形の初期化
	void InitializeImGui(); // ImGuiの初期化

	// デスクリプタヒープ生成
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device>& device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);
	// 深度バッファ用リソース生成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreatDepthStenCilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device>& device, int32_t width, int32_t height);
	// SRVの指定番号のCPUデスクリプタハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);
	// SRVの指定番号のGPUデスクリプタハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

	std::string ConvertString(const std::wstring& str);

private:
	// DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	// DXGIファクトリー
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;
	// コマンドキュー
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;

	// depthStencilリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource;

	// ディスクリプタサイズ
	uint32_t descriptorSizeSRV;
	// デスクリプタヒープ
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> srvDescriptorHeap; // SRV
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> rtvDescriptorHeap; // RTV
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> dsvDescriptorHeap; // DSV

	// デスクリプタヒープ
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{}; // RTV設定

	// スワップチェイン
	Microsoft::WRL::ComPtr <IDXGISwapChain4> swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{}; // スワップチェイン設定
	// スワップチェーンリソース
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChinResources;



	// ビューポート
	D3D12_VIEWPORT viewport{};
	// シザー矩形
	D3D12_RECT scissorRect{};

	// WindowAPI
	WindowAPI* windowAPI_ = nullptr;

	// 指定番号のCPUデスクリプタハンドルを取得
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);
	// 指定番号のGPUデスクリプタハンドルを取得
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);

};

