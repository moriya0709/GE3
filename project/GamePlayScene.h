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

class SpriteCommon;
class ObjectCommon;

class GamePlayScene {
public:
	// 初期化
	void Initialize(SpriteCommon* spriteCommon, ObjectCommon* objectCommon);
	// 更新
	void Update();
	// 描画
	void Draw3D();
	void Draw2D();
	// 終了
	void Finalize();

	// getter
	Camera* GetCamera() const { return camera; }


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