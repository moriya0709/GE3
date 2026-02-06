#include "SceneManager.h"

SceneManager* SceneManager::instance = nullptr;

void SceneManager::Initialize() {
	if (nextScene_) {
		// シーン切り換え
		scene_ = nextScene_;
		nextScene_ = nullptr;

		// シーンマネージャーをセット
		scene_->SetSceneManager(this);

		// 次シーンを初期化
		scene_->Initialize();
	}
}

void SceneManager::Update() {
	// シーン切り替え処理
	if (nextScene_) {
		// 旧シーン終了
		if (scene_) {
			scene_->Finalize();
			delete scene_;
		}

		// シーン切り換え
		scene_ = nextScene_;
		nextScene_ = nullptr;

		// シーンマネージャーをセット
		scene_->SetSceneManager(this);

		// 次シーンを初期化
		scene_->Initialize();

	}

	// 実行中シーンを更新
	scene_->Update();

}

void SceneManager::Draw() {
	scene_->Draw();
}

void SceneManager::Finalize() {
	delete instance;
	instance = nullptr;
}

SceneManager* SceneManager::GetInstance() {
	if (instance == nullptr) {
		instance = new SceneManager;
	}
	return instance;
}

SceneManager::~SceneManager() {
	// 最後のシーンの終了と解放
	if (scene_) {
		scene_->Finalize();
		delete scene_;
	}
}
