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
	object = std::make_unique <Object>();
	object->Initialize(camera.get());

	// Emitパーティクル発生
	particleEmitter = std::make_unique <ParticleEmitter>();
	particleEmitter->Initialize("group1", transformParticle, 5, 1.0f);
	particleEmitter->Emit();

	// 初期化済みの3Dオブジェクトにモデルを紐づける
	object->SetModel("terrain.obj");
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
	object->Update();

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

	// ポイントライト
	ImGui::DragFloat3("pointLightPosition", &PointLight.x, 0.01f, -100.0f, 100.0f);
	object->SetPointLightPosition(PointLight);

	// スポットライト
	ImGui::DragFloat3("spotLightPosition", &SpotLightPosition.x, 0.01f, -100.0f, 100.0f);
	ImGui::DragFloat3("spotLightDirection", &SpotLightDirection.x, 0.01f, -100.0f, 100.0f);
	ImGui::DragFloat("spotRange", &SpotLightRange, 0.01f, 0.0f, 100.0f);
	
	object->SetSpotLightPosition(SpotLightPosition);
	object->SetSpotLightDirection(SpotLightDirection);
	object->SetSpotLightRange(SpotLightRange);

	// ディスタンスフォグ
	ImGui::DragFloat("fogStart", &start, 0.1f, 0.0f, 100.0f);
	ImGui::DragFloat("fogEnd", &end, 0.1f, 0.0f, 100.0f);
	PostEffect::GetInstance()->SetDistanceFogStart(start);
	PostEffect::GetInstance()->SetDistanceFogEnd(end);

	

#endif

}

void GamePlayScene::Draw() {
	// 3Dオブジェクトの描画準備
	ObjectCommon::GetInstance()->SetCommonPipelineState();

	// 3Dオブジェクト描画
	object->Draw();

	// 3Dオブジェクトの描画準備
	ObjectCommon::GetInstance()->SetOutlinePipelineState();
	
	// 3Dオブジェクト描画
	object->Draw();
	
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
