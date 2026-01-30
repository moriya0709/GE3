#pragma once
#include <Windows.h>
#include <dxgidebug.h>
#include <strsafe.h>
#include <minidumpapiset.h>

#include "SpriteCommon.h"
#include "ObjectCommon.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "ParticleManager.h"
#include "ImGuiManager.h"
#include "M_Framework.h"
#include "GamePlayScene.h"
#include "SoundManager.h"
#include "ModelManager.h"

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

	// スプライト共通部
	SpriteCommon* spriteCommon = nullptr;
	// 3Dオブジェクト共通部
	ObjectCommon* objectCommon = nullptr;
	// SRVマネージャ
	SrvManager* srvManager = nullptr;
	// ImGuiマネージャ
	ImGuiManager* imGuiManager = nullptr;
};