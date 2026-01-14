#include "ImGuiManager.h"
#include "WindowAPI.h"
#include "DirectXCommon.h"
#include "SrvManager.h"

void ImGuiManager::Initialize(WindowAPI* windowAPI, DirectXCommon* dxCommon, SrvManager* srvManager) {
	// ImGuiのコンテキストを生成
	ImGui::CreateContext();
	// ImGuiのスタイルを設定
	ImGui::StyleColorsDark();
	// win32用初期化
	ImGui_ImplWin32_Init(windowAPI->GetHwnd());
	// DirectX12用初期化
	uint32_t index = srvManager->Allocate();
	ImGui_ImplDX12_Init(dxCommon->GetDevice(),
		static_cast<int>(dxCommon->GetSwapChainResourceNum()),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		srvManager->GetDescriptorHeap(),
		srvManager->GetCPUDescriptorHandle(index),
		srvManager->GetGPUDescriptorHandle(index));
}

void ImGuiManager::Finalize() {
	// 後始末
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
