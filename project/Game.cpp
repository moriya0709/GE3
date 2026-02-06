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
	spriteCommon = SpriteCommon::GetInstance();
	spriteCommon->Initialize(dxCommon);

	// 3dスプライト共通部の初期化
	objectCommon = ObjectCommon::GetInstance();
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

	// パーティクルマネージャ初期化
	ParticleManager::GetInstance()->CreateParticleGroup("group1", "Resource/particle.png");
	ParticleManager::GetInstance()->CreateParticleGroup("group2", "Resource/uvChecker.png");

	// .objファイルからモデル読み込み
	ModelManager::GetInstance()->LoadModel("plane.obj");
	ModelManager::GetInstance()->LoadModel("axis.obj");

	// サウンド
	SoundManager::GetInstance()->Initialize();
	SoundManager::GetInstance()->Load("bgm", "game.mp3");

	// ImGui
	imGuiManager = new ImGuiManager();
	imGuiManager->Initialize(windowAPI, dxCommon, srvManager);


	// シーンマネージャーの生成
	sceneManager_ = SceneManager::GetInstance();
	// 最初のシーン生成
	BaseScene* scene = new TitleScene();
	// シーンマネージャーに最初のシーンをセット
	sceneManager_->SetNextScene(scene);
	// 初期化
	sceneManager_->Initialize();

	// カメラマネージャ登録
	CameraManager::GetInstance()->AddCamera("main", scene->GetCamera());
	CameraManager::GetInstance()->SetActiveCamera("main");

#pragma endregion

}

void Game::Update() {
	// ImGui受付開始
	imGuiManager->Begin();

	//　基底クラス
	M_Framework::Update();
	
	// シーンマネージャー更新
	sceneManager_->Update();

	// パーティクル更新
	ParticleManager::GetInstance()->Update();
	// ImGui受付終了
	imGuiManager->End();
}

void Game::Draw() {
	// 描画前処理
	M_Framework::BeginFrame();
	srvManager->PreDraw();

	// シーンマネージャー描画
	sceneManager_->Draw();

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
	//　サウンドマネージャー終了
	SoundManager::GetInstance()->Finalize();

	// スプライト共通部解放
	delete spriteCommon;
	// 3Dオブジェクト共通部解放
	delete objectCommon;
	// SRVマネージャ解放
	delete srvManager;
	// imGuiマネージャ解放
	delete imGuiManager;

	// シーンマネージャ解放
	delete sceneManager_;

	// 基底クラスの終了処理
	M_Framework::Finalize();
}