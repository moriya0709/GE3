#include "Game.h"


#pragma comment(lib,"Dbghelp.lib")
#pragma comment(lib,"dxcompiler.lib")

void Game::Initialize() {
	// 基底クラスの初期化
	M_Framework::Initialize();


#pragma region 基盤システム

	// カメラマネージャ
	CameraManager::GetInstance();

	// スプライト共通部の初期化
	spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(dxCommon);

	// 3dスプライト共通部の初期化
	objectCommon = new ObjectCommon();
	objectCommon->Initialize(dxCommon);

	// SRVマネージャ
	srvManager = new SrvManager();
	srvManager->Initialize(dxCommon);

	// テクスチャマネージャの初期化
	TextureManager::GetInstance()->Initialize(dxCommon, srvManager);
	// 3Dモデルマネージャの初期化
	ModelManager::GetInstance()->Initialize(dxCommon);
	// Particleマネージャ
	ParticleManager::GetInstance()->Initialize(dxCommon, srvManager, "Resource/", "plane.obj");


#pragma endregion

#pragma region 最初のシーン

	// サウンド
	SoundManager::GetInstance()->Initialize();

	// ImGui
	imGuiManager = new ImGuiManager();
	imGuiManager->Initialize(windowAPI, dxCommon, srvManager);

	// ゲームプレイシーンの初期化
	scene->Initialize(dxCommon);

	// カメラを3Dオブジェクト共通部にセット
	objectCommon->SetDefaultCamera(scene->GetCamera());

#pragma endregion

}

void Game::Update() {
	// ImGui受付開始
	imGuiManager->Begin();

	//　基底クラス
	M_Framework::Update();
	// シーン更新
	scene->Update();

	// パーティクル更新
	ParticleManager::GetInstance()->Update(scene->GetCamera());
	// ImGui受付終了
	imGuiManager->End();
}

void Game::Draw() {
	// 描画前処理
	M_Framework::BeginFrame();
	srvManager->PreDraw();

	// 3Dオブジェクトの描画準備
	objectCommon->SetCommonPipelineState();
	// 3Dシーン描画
	scene->Draw3D();

	// パーティクル描画
	ParticleManager::GetInstance()->Draw();

	// スプライトの描画準備
	spriteCommon->SetCommonPipelineState();
	// 2Dシーン描画
	scene->Draw2D();

	// ImGui描画
	imGuiManager->Draw();

	// 描画後処理
	M_Framework::EndFrame();

}

void Game::Finalize() {
	// ImGuiの終了処理
	imGuiManager->Finalize();

	// テクスチャマネージャの終了
	TextureManager::GetInstance()->Finalize();
	// 3Dモデルマネージャの終了
	ModelManager::GetInstance()->Finalize();
	// Particleマネージャの終了
	ParticleManager::GetInstance()->Finalize();

	// スプライト共通部解放
	delete spriteCommon;
	// 3Dオブジェクト共通部解放
	delete objectCommon;
	// SRVマネージャ解放
	delete srvManager;
	// imGuiマネージャ解放
	delete imGuiManager;

	// 基底クラスの終了処理
	M_Framework::Finalize();
}