#include "Game.h"

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	Game game;
	// 初期化
	game.Initialize();

	MSG msg{};
	// ウィンドウのｘボタンが押されるまでループ
	while (msg.message != WM_QUIT) {

		// 更新処理
		game.Update();

		if (game.IsEndRequest()) {
			// ゲームループを抜ける
			break;
		}

		// 描画処理
		game.Draw();

	}

	// 終了処理
	game.Finalize();

	return 0;

}