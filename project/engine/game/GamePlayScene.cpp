#include "GamePlayScene.h"
#include "ObjectCommon.h"
#include "SpriteCommon.h"
#include "SceneManager.h"

void GamePlayScene::Initialize() {
	// カメラ初期化
	camera = std::make_unique <Camera>();
	camera->SetRotate({ cameraTransform.rotate });
	camera->SetTranslate({ cameraTransform.translate });

	// カメラマネージャ登録
	CameraManager::GetInstance()->AddCamera("main", camera.get());
	CameraManager::GetInstance()->SetActiveCamera("main");

	// スプライト
	sprite = std::make_unique <Sprite>();
	sprite->Initialize("Resource/uvChecker.png");

	// 3Dオブジェクト
	for (int i = 0; i < 2; i++) {
		object[i] = std::make_unique <Object>();
		object[i]->Initialize(camera.get());
	}

	// Emitパーティクル発生
	particleEmitter = std::make_unique <ParticleEmitter>();
	particleEmitter->Initialize("group1", transformParticle, 5, 1.0f);
	particleEmitter->Emit();

	// 初期化済みの3Dオブジェクトにモデルを紐づける
	object[0]->SetModel("plane.obj");
	object[1]->SetModel("axis.obj");
}

void GamePlayScene::Update() {
	// 入力取得
	auto input = Input::GetInstance();
	// カメラ更新
	CameraManager::GetInstance()->Update();

	// ENTERキーを押したら
	if (input->TriggerKey(DIK_RETURN)) {
		// シーン切り換え
		SceneManager::GetInstance()->ChangeScene("TITLE");
	}

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

void GamePlayScene::Draw() {
	// 3Dオブジェクトの描画準備
	ObjectCommon::GetInstance()->SetCommonPipelineState();

	// 3Dオブジェクト描画
	//for (int i = 0; i < 2; i++) {
	//	object[i]->Draw();
	//}
	
	// パーティクル描画
	ParticleManager::GetInstance()->Draw();

	// 2Dオブジェクトの描画準備
	SpriteCommon::GetInstance()->SetCommonPipelineState();

	// スプライト描画
	sprite->Draw();
}

void GamePlayScene::Finalize() {
	CameraManager::GetInstance()->RemoveCamera("main");
}
