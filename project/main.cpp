#define _USE_MATH_DEFINES

#include <Windows.h>
#include <cstdint>
#include <string>
#include <fstream>
#include <chrono>
#include <filesystem>
#include <D3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <dbghelp.h>
#include <strsafe.h>
#include <dxgidebug.h>
#include <dxcapi.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <sstream>
#include <wrl.h>
#include <xaudio2.h>
#include <dinput.h>
#include <cmath>

#include "Calc.h"
#include "Input.h"
#include "WindowAPI.h"
#include "DirectXCommon.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "TextureManager.h"
#include "ObjectCommon.h"
#include "Object.h"
#include "ModelCommon.h"
#include "Model.h"
#include "ModelManager.h"
#include "Camera.h"
#include "CameraManager.h"
#include "SrvManager.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"
#include "ImGuiManager.h"
#include "SoundManager.h"
#include "Game.h"


#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"

//extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#pragma comment(lib,"D3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"Dbghelp.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")
#pragma comment(lib,"xaudio2.lib")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")


//文字列を格納する
std::string str0{ "STRING" };

//ログのディレクトリを用意
namespace fs = std::filesystem;

// 3x3行列
struct Matrix3x3 {
	float m[3][3] = { 0 };
};

Transform cameraTransform
{
	{ 1.0f, 1.0f, 1.0f }, // scale
	{ 0.0f, 0.0f, 0.0f }, // rotate
	{ 0.0f, 0.0f, -5.0f } // translate
};

// transformの初期化
Transform transform
{
	{ 1.0f, 1.0f, 1.0f }, // scale
	{ 0.0f, 0.0f, 0.0f }, // rotate
	{ 0.0f, 0.0f, 0.0f }  // translate
};

// transformSpriteの初期化
Transform tranaformSprite
{
	{1.0f,1.0f,1.0f},
	{0.0f,0.0f,0.0f},
	{0.0f,0.0f,0.0f}
};

// パーティクル
Transform transformParticle
{
	{1.0f,1.0f,1.0f},
	{0.0f,0.0f,0.0f},
	{0.0f,0.0f,0.0f}
};

Transform uvTransformSprite{
	{1.0f,1.0f,1.0f},
	{0.0f,0.0f,0.0f},
	{0.0f,0.0f,0.0f},
};

// SRV切り替え
bool useMonsterBall = true;

static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"./Duumps", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
	HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	// processId(このexeのID)とクラッシュ（例外）の発生したthreadIdを取得
	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();
	// 設定情報を入力
	MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{ 0 };
	minidumpInformation.ThreadId = threadId;
	minidumpInformation.ExceptionPointers = exception;
	minidumpInformation.ClientPointers = TRUE;
	//Dumpを出力。MiniDumpNormalは最低限の情報を出力するプラグ
	MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInformation, nullptr, nullptr);

	return EXCEPTION_EXECUTE_HANDLER;
}

void Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
}


void Log(std::ostream& os, const std::string& message) {
	os << message << std::endl;
	OutputDebugStringA(message.c_str());
}

// ウィンドウプロシージャ
//LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
//	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) {
//		return true;
//	}
//
//	// メッセージに応じてゲーム固有の処理を行う
//	switch (msg) {
//		// ウィンドウが破壊された
//	case WM_DESTROY:
//	// OSに対して、アプリの終了を伝える
//	PostQuitMessage(0);
//	return 0;
//	}
//	// 標準のメッセージ処理を行う
//	return DefWindowProc(hwnd, msg, wParam, lParam);
//
//}



Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device>& device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}




// 正規化関数
void Normalize(float& x, float& y, float& z) {
	float len = std::sqrt(x * x + y * y + z * z);
	if (len > 0.00001f) {
		x /= len;
		y /= len;
		z /= len;
	}
}





// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	Game* game = new Game();
	game->Initialize();

	MSG msg{};
	// ウィンドウのｘボタンが押されるまでループ
	while (msg.message != WM_QUIT) {
		// Windowsのメッセ維持処理
		if (windowAPI->ProcessMessage()) {
			// ゲームループを抜ける
			break;
		}

		// ImGui受付開始
		imGuiManager->Begin();
		
		// ゲームの処理

		// 入力の更新
		input->Update();
		// カメラ更新
		CameraManager::GetInstance()->Update();


		// 数字の０キーが押されていたら
		if (input->TriggerKey(DIK_0)) {
			OutputDebugStringA("Hit 0\n"); // 出力ウィンドウに「Hit ０」と表示
			// テクスチャ変更
			sprite->ChangeTexture("Resource/uvChecker.png");
			particleEmitter->SetActive("group2");
		}

		// y軸回転処理
		transform.rotate.y = 3.00f;

		// * 3Dオブジェクト* //
		for (int i = 0; i < 2; i++) {
			object[i]->Update();
		}

		// パーティクルエミッタ更新
		particleEmitter->Update();
		// パーティクル更新
		ParticleManager::GetInstance()->Update();

		// *スプライト* //

		// sprite更新
		sprite->Update();
		






		// スライダー
		//UI
		//ImGui::SliderFloat("SpritePosX", &tranaformSprite.translate.x, 0.0f, 500.0f);
		//ImGui::SliderFloat("SpritePosY", &tranaformSprite.translate.y, 0.0f, 500.0f);

		// ライトの向き
		//ImGui::SliderFloat("directionX", &directionalLightData->direction.x, -10.0f, 10.0f);
		//ImGui::SliderFloat("directionY", &directionalLightData->direction.y, -10.0f, 10.0f);
		//ImGui::SliderFloat("directionZ", &directionalLightData->direction.z, -10.0f, 10.0f);

		// SRVの切り替え
		//ImGui::Checkbox("UseMonsterBall", &useMonsterBall);

		// UV座標
		//ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		//ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
		//ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);

		// モデル
		//ImGui::DragFloat3("scale", &transform.scale.x, 0.01f, -10.0f, 10.0f);
		//ImGui::DragFloat3("translate", &transform.translate.x, 0.01f, -10.0f, 10.0f);
		//ImGui::DragFloat3("rotate", &transform.rotate.x, 0.01f, -10.0f, 10.0f);

	#ifdef USE_IMGUI
		// カメラ
		ImGui::DragFloat3("cameraTranslate", &cameraTransform.translate.x, 0.01f, -100.0f, 100.0f);
		ImGui::DragFloat3("cameraRotate", &cameraTransform.rotate.x, 0.01f, -180.0f, 180.0f);
		
	#endif

		// ImGui受付終了
		imGuiManager->End();

		// 描画前処理
		dxCommon->PreDraw();
		srvManager->PreDraw();

		// 3Dオブジェクトの描画準備
		objectCommon->SetCommonPipelineState();

		// 3Dオブジェクト描画
		//for (int i = 0; i < 2; i++) {
		//	object[i]->Draw();
		//}

		// パーティクル描画
		ParticleManager::GetInstance()->Draw();

		// スプライトの描画準備
		spriteCommon->SetCommonPipelineState();

		// スプライト描画
		//sprite->Draw();

		

		// ImGui描画
		imGuiManager->Draw();

		dxCommon->PostDraw();

	}

	// ImGuiの終了処理
	imGuiManager->Finalize();

	// 解放
	CloseHandle(dxCommon->fenceEvent);

	// テクスチャマネージャの終了
	TextureManager::GetInstance()->Finalize();
	// 3Dモデルマネージャの終了
	ModelManager::GetInstance()->Finalize();
	// Particleマネージャの終了
	ParticleManager::GetInstance()->Finalize();

	//　サウンドマネージャー終了
	soundManager->Finalize(&soundData1);

	// 入力の初期化
	delete input;
	// WindowAPIの終了処理
	windowAPI->Finalize();
	// WindowAPIの解放
	delete windowAPI;
	// DirectX解放
	delete dxCommon;
	// スプライト解放
	delete sprite;
	delete spriteCommon;
	// 3Dオブジェクト解放
	for (int i = 0; i < 2; i++) {
		delete object[i];
	}
	delete objectCommon;
	// カメラ解放
	delete camera;
	// SRVマネージャ解放
	delete srvManager;
	// imGuiマネージャ解放
	delete imGuiManager;

	CoUninitialize();

	return 0;

}