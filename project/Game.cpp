#include "Game.h"

#pragma comment(lib,"Dbghelp.lib")
#pragma comment(lib,"dxcompiler.lib")



void Game::Initialize() {
	// 基底クラスの初期化
	M_Framework::Initialize();


#pragma region 基盤システム

	// スプライト共通部の初期化
	spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(dxCommon);

	// 3dスプライト共通部の初期化
	objectCommon = new ObjectCommon();
	objectCommon->Initialize(dxCommon);

	// カメラ初期化
	camera = new Camera();
	camera->SetRotate({ cameraTransform.rotate });
	camera->SetTranslate({ cameraTransform.translate });

	// カメラマネージャ登録
	cameraManager = CameraManager::GetInstance();
	cameraManager->AddCamera("main", camera);
	cameraManager->SetActiveCamera("main");

	// カメラを3Dオブジェクト共通部にセット
	objectCommon->SetDefaultCamera(camera);

	// SRVマネージャ
	srvManager = new SrvManager();
	srvManager->Initialize(dxCommon);

	// テクスチャマネージャの初期化
	TextureManager::GetInstance()->Initialize(dxCommon, srvManager);

	// 3Dモデルマネージャの初期化
	ModelManager::GetInstance()->Initialize(dxCommon);

	// Particleマネージャ
	ParticleManager::GetInstance()->Initialize(dxCommon, srvManager, camera, "Resource/", "plane.obj");
	ParticleManager::GetInstance()->CreateParticleGroup("group1", "Resource/particle.png");
	ParticleManager::GetInstance()->CreateParticleGroup("group2", "Resource/uvChecker.png");

#pragma endregion

#pragma region 最初のシーン

	// スプライト
	sprite = new Sprite();
	sprite->Initialize(spriteCommon, dxCommon, "Resource/uvChecker.png");

	// 3Dオブジェクト
	for (int i = 0; i < 2; i++) {
		object[i] = new Object();
		object[i]->Initialize(objectCommon, windowAPI, dxCommon);
	}

	// Emitパーティクル発生
	particleEmitter = new ParticleEmitter();
	particleEmitter->Initialize("group1", transformParticle, 5, 1.0f);
	particleEmitter->Emit();

	// .objファイルからモデル読み込み
	ModelManager::GetInstance()->LoadModel("plane.obj");
	ModelManager::GetInstance()->LoadModel("axis.obj");

	// 初期化済みの3Dオブジェクトにモデルを紐づける
	object[0]->SetModel("plane.obj");
	object[1]->SetModel("axis.obj");

	// サウンド
	soundManager = new SoundManager();
	soundManager->Initialize();

	// 音声読み込み
	soundData1 = soundManager->SoundLoadFile("game.mp3");
	// 音声再生
	soundManager->SoundPlayWave(soundData1);

	// ImGui
	imGuiManager = new ImGuiManager();
	imGuiManager->Initialize(windowAPI, dxCommon, srvManager);

#pragma endregion

}

void Game::Update() {
	//　基底クラス
	M_Framework::Update();

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

#ifdef USE_IMGUI
	// ImGui

	// カメラ
	ImGui::DragFloat3("cameraTranslate", &cameraTransform.translate.x, 0.01f, -100.0f, 100.0f);
	ImGui::DragFloat3("cameraRotate", &cameraTransform.rotate.x, 0.01f, -180.0f, 180.0f);

#endif

	// ImGui受付終了
	imGuiManager->End();
}

void Game::Draw() {
	// 描画前処理
	M_Framework::BeginFrame();
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
	sprite->Draw();

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
	soundManager->Finalize(&soundData1);

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

	// 基底クラスの終了処理
	M_Framework::Finalize();
}
