#pragma once
#include <Windows.h>
#include <dxgidebug.h>
#include <strsafe.h>
#include <minidumpapiset.h>

#include "WindowAPI.h"
#include "DirectXCommon.h"
#include "Input.h"
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

class Game {
public:
	// 初期化
	void Initialize();
	// 更新
	void Update();
	// 描画
	void Draw();

	// 終了
	void Finalize();

	// フラグチェック
	bool IsEndRequest() { return endRequest_; }

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

	// WindowAPI
	WindowAPI* windowAPI = nullptr;
	// DirectX共通部
	DirectXCommon* dxCommon = nullptr;
	// 入力
	Input* input = nullptr;
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

	// ゲーム終了フラグ
	bool endRequest_ = false;

};

