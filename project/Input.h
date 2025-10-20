#pragma once
#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定

#include <Windows.h>
#include <cassert>
#include <vector>
#include <dinput.h>
#include <wrl.h>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

class Input {
public:
	// namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	//初期化
	void Initialize(HINSTANCE hInstance,HWND hwnd);
	// 更新
	void  Update();

	// キーが押されたかどうかを調べる
	bool PushKey(BYTE keyNumBer); // プッシュ
	bool TriggerKey(BYTE keyNumber); // トリガー

private:
	// DirectInputのインスタンス
	ComPtr<IDirectInput8> directInput = nullptr;

	// キーボード
	ComPtr<IDirectInputDevice8> keyboard = nullptr;
	BYTE key[256] = {}; // 現在のキー状態
	BYTE keyPre[256] = {}; // 前フレームのキー状態


};

