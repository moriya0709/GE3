#pragma once

#include <Windows.h>
#include <cassert>
#include <vector>
#include <dinput.h>
#include <wrl.h>

class Input {
public:
	// namespace�ȗ�
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	//������
	void Initialize(HINSTANCE hInstance,HWND hwnd);
	// �X�V
	void  Update();

	// �L�[�������ꂽ���ǂ����𒲂ׂ�
	bool PushKey(BYTE keyNumBer); // �v�b�V��
	bool TriggerKey(BYTE keyNumber); // �g���K�[

private:
	// DirectInput�̃C���X�^���X
	ComPtr<IDirectInput8> directInput = nullptr;

	// �L�[�{�[�h
	ComPtr<IDirectInputDevice8> keyboard = nullptr;
	BYTE key[256] = {}; // ���݂̃L�[���
	BYTE keyPre[256] = {}; // �O�t���[���̃L�[���


};

