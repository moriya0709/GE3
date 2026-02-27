#include "TitleScene.h"
#include "ObjectCommon.h"
#include "SpriteCommon.h"
#include "SceneManager.h"

void TitleScene::Initialize() {
	// カメラ初期化
	camera = std::make_unique <Camera>();
	camera->SetRotate({ cameraTransform.rotate });
	camera->SetTranslate({ cameraTransform.translate });

	// カメラマネージャ登録
	CameraManager::GetInstance()->AddCamera("main", camera.get());
	CameraManager::GetInstance()->SetActiveCamera("main");

	// スプライト
	sprite = std::make_unique <Sprite>();
	sprite->Initialize("Resource/monsterBall.png");

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

	// 音声再生
	SoundManager::GetInstance()->Play("bgm");

}

void TitleScene::Update() {
	// 入力取得
	auto input = Input::GetInstance();
	// カメラ更新
	CameraManager::GetInstance()->Update();

	// ENTERキーを押したら
	if (input->TriggerKey(DIK_RETURN)) {
		// ゲームプレイシーン(次シーン)を生成
		SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
		// 音声再生
		SoundManager::GetInstance()->Stop("bgm");
	}

	// 数字の０キーが押されていたら
	if (input->TriggerKey(DIK_0)) {
		OutputDebugStringA("Hit 0\n"); // 出力ウィンドウに「Hit ０」と表示
		// テクスチャ変更
		sprite->ChangeTexture("Resource/uvChecker.png");
		particleEmitter->SetActive("group2");

		// エフェクト有効化(色反転)
		PostEffect::GetInstance()->SetInversion(true);
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

	// ハイトフォグ
	// 行列更新
	PostEffect::GetInstance()->HightFogUpdate(camera.get());

#ifdef USE_IMGUI
	// ImGui

	// カメラ
	ImGui::DragFloat3("cameraTranslate", &cameraTransform.translate.x, 0.01f, -100.0f, 100.0f);
	ImGui::DragFloat3("cameraRotate", &cameraTransform.rotate.x, 0.01f, -180.0f, 180.0f);
	camera->SetTranslate({ cameraTransform.translate });
	camera->SetRotate({ cameraTransform.rotate });

	// ディスタンスフォグ
	ImGui::DragFloat("fogStart", &start, 0.1f, 0.0f, 100.0f);
	ImGui::DragFloat("fogEnd", &end, 0.1f, 0.0f, 100.0f);
	PostEffect::GetInstance()->SetDistanceFogStart(start);
	PostEffect::GetInstance()->SetDistanceFogEnd(end);

	// ハイトフォグ
	ImGui::DragFloat("heightFogTop", &heightFogTop, 0.1f, -100.0f, 100.0f);
	ImGui::DragFloat("heightFogBottom", &heightFogBottom, 0.1f, -100.0f, 100.0f);
	ImGui::DragFloat("heightFogDensity", &heightFogDensity, 0.01f, 0.0f, 10.0f);
	PostEffect::GetInstance()->SetHeightFogTop(heightFogTop);
	PostEffect::GetInstance()->SetHeightFogBottom(heightFogBottom);
	PostEffect::GetInstance()->SetHeightFogDensity(heightFogDensity);


#endif

}

void TitleScene::Draw() {
	// 3Dオブジェクトの描画準備
	ObjectCommon::GetInstance()->SetCommonPipelineState();

	// 3Dオブジェクト描画
	for (int i = 0; i < 2; i++) {
		object[i]->Draw();
	}

	// 3Dオブジェクトの描画準備
	ObjectCommon::GetInstance()->SetOutlinePipelineState();

	// 3Dオブジェクト描画
	for (int i = 0; i < 2; i++) {
		object[i]->Draw();
	}

	// パーティクル描画
	ParticleManager::GetInstance()->Draw();

	// 2Dオブジェクトの描画準備
	SpriteCommon::GetInstance()->SetCommonPipelineState();

	// スプライト描画
	//sprite->Draw();
}

void TitleScene::Finalize() {
	CameraManager::GetInstance()->RemoveCamera("main");
}
