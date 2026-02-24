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

class GamePlayScene : public BaseScene{
public:
	// 初期化
	void Initialize() override;
	// 更新
	void Update() override;
	// 描画
	void Draw() override;
	// 終了
	void Finalize() override;

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

	// ポイントライト
	Vector3 PointLight = { 1.0f, 1.0f, 0.0f };

	// スポットライト
	Vector3 SpotLightPosition = { 0.0f, 0.0f, 0.0f };
	Vector3 SpotLightDirection = { 0.0f, 0.0f, 0.0f };
	float SpotLightRange = 10.0f;

	// カメラ
	std::unique_ptr <Camera> camera = nullptr;
	// スプライト
	std::unique_ptr <Sprite> sprite = nullptr;
	// 3Dオブジェクト
	std::unique_ptr <Object> object{};
	// パーティクルエミッタ
	std::unique_ptr <ParticleEmitter> particleEmitter = nullptr;

};