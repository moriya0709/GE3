#include "ImGuiManager.h"
#include "WindowAPI.h"
#include "DirectXCommon.h"
#include "SrvManager.h"

void ImGuiManager::Initialize(WindowAPI* windowAPI, DirectXCommon* dxCommon, SrvManager* srvManager) {
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
	
	// ImGuiのコンテキストを生成
	ImGui::CreateContext();
	// ImGuiのスタイルを設定
	ImGui::StyleColorsDark();
	// win32用初期化
	ImGui_ImplWin32_Init(windowAPI->GetHwnd());
	// DirectX12用初期化
	uint32_t index = srvManager->Allocate();
	ImGui_ImplDX12_Init(dxCommon_->GetDevice(),
		static_cast<int>(dxCommon_->GetSwapChainResourceNum()),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		srvManager->GetDescriptorHeap(),
		srvManager->GetCPUDescriptorHandle(index),
		srvManager->GetGPUDescriptorHandle(index));
}

void ImGuiManager::Draw() {
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	
	// デスクリプタヒープの配列をセットするコマンド
	ID3D12DescriptorHeap* ppHeaps[] = { srvManager_->GetDescriptorHeap() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	// 描画コマンドを発行
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}

void ImGuiManager::Finalize() {
	// 後始末
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

// 受付開始
void ImGuiManager::Begin() {
	// ImGuiフレーム開始
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

// 受付終了
void ImGuiManager::End() {
	// 描画前準備
	ImGui::Render();
}
