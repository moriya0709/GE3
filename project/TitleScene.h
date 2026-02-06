#pragma once
#include "Camera.h"
#include "Sprite.h"
#include "Object.h"
#include "ParticleEmitter.h"
#include "CameraManager.h"
#include "ParticleManager.h"
#include "ModelManager.h"
#include "SoundManager.h"
#include "Input.h"
#include "ImGuiManager.h"
#include "BaseScene.h"

class SpriteCommon;
class ObjectCommon;

class TitleScene : public BaseScene {
public:
	// 初期化
	void Initialize() override;
	// 更新
	void Update() override;
	// 描画
	void Draw() override;
	// 終了
	void Finalize() override;

	// getter
	Camera* GetCamera() override { return camera; }

private:
	Transform cameraTransform
	{
		{ 1.0f, 1.0f, 1.0f }, // scale
		{ 0.0f, 0.0f, 0.0f }, // rotate
		{ 0.0f, 0.0f, -5.0f } // translate
	};
	// パーティクル
	Transform transformParticle
	{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	// カメラ
	Camera* camera = nullptr;
	// スプライト
	Sprite* sprite = nullptr;
	// 3Dオブジェクト
	Object* object[2]{};
	// パーティクルエミッタ
	ParticleEmitter* particleEmitter = nullptr;
	// サウンド
	SoundData soundData1; // サウンドデータ

};