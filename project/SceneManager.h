#pragma once
#include "BaseScene.h"
#include "TitleScene.h"
#include "GamePlayScene.h"

class SceneManager {
public:
	// 初期化
	void Initialize();
	// 更新
	void Update();
	// 描画
	void Draw();

	// 次シーン予約
	void SetNextScene(BaseScene* nextScene) {nextScene_ = nextScene;}
	// シングルトンインスタンスの取得
	static SceneManager* GetInstance();

	~SceneManager();

private:
	// シングルトンインスタンス
	static SceneManager* instance;

	// 今のシーン（実行中シーン）
	BaseScene* scene_ = nullptr;
	// 次のシーン
	BaseScene* nextScene_ = nullptr;

};

