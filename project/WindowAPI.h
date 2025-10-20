#pragma once
#include <Windows.h>
#include <cstdint>
#include <wrl.h>
#include <dxgi1_6.h>

#include "externals/imgui\imgui.h"
#include "externals/DirectXTex/d3dx12.h"

class WindowAPI {
public:
	// �E�B���h�E�v���V�[�W��
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//�N���C�A���g�̈�̃T�C�Y
	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;

	// ������
	void Initialize();
	// �X�V
	void Update();
};

