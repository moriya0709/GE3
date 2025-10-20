#pragma once
#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定

#include <Windows.h>
#include <cassert>
#include <vector>
#include <dinput.h>
#include <wrl.h>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

using namespace Microsoft::WRL;

class Input {
public:
	//初期化
	void Initialize(HINSTANCE hInstance,HWND hwnd);
	// 更新
	void  Update();
};

