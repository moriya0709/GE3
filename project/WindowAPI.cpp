#include "WindowAPI.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// �E�B���h�E�v���V�[�W��
LRESULT WindowAPI::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) {
		return true;
	}

	// ���b�Z�ێ��ɉ����ăQ�[���ŗL�̏������s��
	switch (msg) {
		// �E�B���h�E���j�󂳂ꂽ
	case WM_DESTROY:
		// OS�ɑ΂��āA�A�v���̏I����`����
		PostQuitMessage(0);
		return 0;
	}

	// �W���̃��b�Z�ێ��������s��
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void WindowAPI::Initialize() {
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

	WNDCLASS wc = {};
	// �E�B���h�E�v���V�[�W��
	wc.lpfnWndProc = WindowProc;
	//�E�B���h�E�N���X���i�Ȃ�ł��ǂ��j
	wc.lpszClassName = L"CG2WindowClass";
	// �C���X�^���X�n���h��
	wc.hInstance = GetModuleHandle(nullptr);
	// �J�[�\��
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// �E�B���h�E�N���X��o�^
	RegisterClass(&wc);

	// �E�B���h�E�T�C�Y��\���\���̂ɃN���C�A���g�̈������
	RECT wrc = { 0,0,kClientWidth,kClientHeight };

	// �N���C�A���g�̈���Ɏ��ۂ̃T�C�Y��wrc��ύX���Ă��炤
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// �E�C���h�E�̐���
	HWND hwnd = CreateWindow(
		wc.lpszClassName,		// ���p����N���X��
		L"CG3",					// �^�C�g���o�[�̕����i���ł��ǂ��j
		WS_OVERLAPPEDWINDOW,	// �ǂ�����E�B���h�E�X�^�C��
		CW_USEDEFAULT,			// �\��X���W�iWindows�ɔC����j
		CW_USEDEFAULT,			// �\��Y���W�iWindows�ɔC����)
		wrc.right - wrc.left,	// �E�B���h�E����
		wrc.bottom - wrc.top,	// �E�B���h�E�c��
		nullptr,				// �e�E�B���h�E�n���h��
		nullptr,				// ���j���[�n���h��
		wc.hInstance,			// �C���X�^���X�n���h��
		nullptr					// �I�v�V����
	);

	// �E�B���h�E��\��
	ShowWindow(hwnd, SW_SHOW);
}

void WindowAPI::Update() {
}
