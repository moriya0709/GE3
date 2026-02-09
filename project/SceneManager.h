#pragma once
#include <cassert>

#include "BaseScene.h"
#include "AbstractSceneFactory.h"

class SceneManager {
public:
	// 初期化
	void Initialize();
	// 更新
	void Update();
	// 描画
	void Draw();
	// 終了
	void Finalize();

	// 次シーン予約
	void SetNextScene(BaseScene* nextScene) {nextScene_ = nextScene;}
	// シングルトンインスタンスの取得
	static SceneManager* GetInstance();

	// シーンファクトリーのsetter
	void SetSceneFactory(AbstractSceneFactory* sceneFactory) { sceneFactory_ = sceneFactory; }
	// シーンの変更
	void ChangeScene(const std::string& sceneName);

	~SceneManager();

private:
	// シングルトンインスタンス
	static SceneManager* instance;

	// 今のシーン（実行中シーン）
	BaseScene* scene_ = nullptr;
	// 次のシーン
	BaseScene* nextScene_ = nullptr;
	// シーンファクトリー
	AbstractSceneFactory* sceneFactory_ = nullptr;

};

