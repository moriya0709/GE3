#pragma once
#include <Windows.h>
#include <dxgidebug.h>
#include <strsafe.h>
#include <minidumpapiset.h>

#include "SpriteCommon.h"
#include "ObjectCommon.h"
#include "Camera.h"
#include "cameraManager.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ParticleManager.h"
#include "Sprite.h"
#include "Object.h"
#include "ParticleEmitter.h"
#include "ModelManager.h"
#include "SoundManager.h"
#include "ImGuiManager.h"
#include "M_Framework.h"

class Game : public M_Framework {
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

	// スプライト共通部
	SpriteCommon* spriteCommon = nullptr;
	// 3Dオブジェクト共通部
	ObjectCommon* objectCommon = nullptr;
	// カメラ
	Camera* camera = nullptr;
	CameraManager* cameraManager;
	// SRVマネージャ
	SrvManager* srvManager = nullptr;
	// ImGuiマネージャ
	ImGuiManager* imGuiManager = nullptr;

	// スプライト
	Sprite* sprite = nullptr;
	// 3Dオブジェクト
	Object* object[2]{};
	// パーティクルエミッタ
	ParticleEmitter* particleEmitter = nullptr;
	// サウンド
	SoundManager* soundManager = nullptr;
	SoundData soundData1; // サウンドデータ
};

