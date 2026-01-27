#include "GamePlayScene.h"

void GamePlayScene::Initialize(SpriteCommon* spriteCommon, ObjectCommon* objectCommon) {
	// カメラ初期化
	camera = new Camera();
	camera->SetRotate({ cameraTransform.rotate });
	camera->SetTranslate({ cameraTransform.translate });

	// カメラマネージャ登録
	CameraManager::GetInstance()->AddCamera("main", camera);
	CameraManager::GetInstance()->SetActiveCamera("main");

	// スプライト
	sprite = new Sprite();
	sprite->Initialize(spriteCommon,"Resource/uvChecker.png");

	// 3Dオブジェクト
	for (int i = 0; i < 2; i++) {
		object[i] = new Object();
		object[i]->Initialize(objectCommon);
	}

	// パーティクルマネージャ初期化
	ParticleManager::GetInstance()->CreateParticleGroup("group1", "Resource/particle.png");
	ParticleManager::GetInstance()->CreateParticleGroup("group2", "Resource/uvChecker.png");

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

	// 音声読み込み
	soundData1 = SoundManager::GetInstance()->SoundLoadFile("game.mp3");
	// 音声再生
	SoundManager::GetInstance()->SoundPlayWave(soundData1);
}

void GamePlayScene::Update() {
	// 入力取得
	auto input = Input::GetInstance();
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


	// *スプライト* //
	// sprite更新
	sprite->Update();

#ifdef USE_IMGUI
	// ImGui

	// カメラ
	ImGui::DragFloat3("cameraTranslate", &cameraTransform.translate.x, 0.01f, -100.0f, 100.0f);
	ImGui::DragFloat3("cameraRotate", &cameraTransform.rotate.x, 0.01f, -180.0f, 180.0f);
	camera->SetTranslate({ cameraTransform.translate });
	camera->SetRotate({ cameraTransform.rotate });
#endif

}

void GamePlayScene::Draw3D() {
	// 3Dオブジェクト描画
	//for (int i = 0; i < 2; i++) {
	//	object[i]->Draw();
	//}
}
void GamePlayScene::Draw2D() {
	// スプライト描画
	sprite->Draw();
}

void GamePlayScene::Finalize() {
	//　サウンドマネージャー終了
	SoundManager::GetInstance()->Finalize(&soundData1);
	// スプライト解放
	delete sprite;
	// 3Dオブジェクト解放
	for (int i = 0; i < 2; i++) {
		delete object[i];
	}
	// カメラ解放
	delete camera;
}
