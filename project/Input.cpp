#define DIRECTINPUT_VERSION 0x0800

#include "Input.h"

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

// ������
void Input::Initialize(HINSTANCE hInstance, HWND hwnd) {
	HRESULT result;

	// DirectInput�̏�����
	result = DirectInput8Create(
		hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	// �L�[�{�[�h�f�o�C�X�̐���
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));

	// ���̓f�[�^�`���̃Z�b�g
	result = keyboard->SetDataFormat(&c_dfDIKeyboard); // �W���`��
	assert(SUCCEEDED(result));

	// �r�����䃌�x���̃Z�b�g
	result = keyboard->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
}

// �X�V
void Input::Update() {
	// �O��̃L�[���͂�ۑ�
	memcpy(keyPre, key, sizeof(key));

	// �L�[�{�[�h���̎擾�J�n
	keyboard->Acquire();
	// �S�L�[�̓��͏����擾����
	keyboard->GetDeviceState(sizeof(key), key);
}

// �v�b�V������
bool Input::PushKey(BYTE keyNumBer) {
	// �w��L�[�������Ă����true��Ԃ�
	if (key[keyNumBer]) {
		return true;
	}

	// �����Ă��Ȃ����false��Ԃ�
	return false;
}
// �g���K�[����
bool Input::TriggerKey(BYTE keyNumber) {
	// �w��L�[��������Ă��āA�O��͉�����Ă��Ȃ����true��Ԃ�
	if (key[keyNumber] && !keyPre[keyNumber]) {
		return true;
	}

	return false;
}

// �}�E�X�{�^���������ꂽ���ǂ���
bool Input::IsMouseButtonPressed(int button) {
	// �w��{�^�������Ă����true��Ԃ�
	if (mouseState.rgbButtons[button]) {
		return true;
	}

	// �����Ă��Ȃ����false��Ԃ�
	return false;
}

// �Q�[���p�b�h�̃{�^���������ꂽ���ǂ���
bool Input::IsPadButtonPressed(int padIndex, int button) {
	if (padIndex >= padStates.size()) return false;
	
	if (padStates[padIndex].rgbButtons[button]) {
		return true;
	}

	return false;
}

// �Q�[���p�b�h�̎��̒l���擾
LONG Input::GetPadAxisX(int padIndex) {
	return padStates[padIndex].lX; // ���X�e�B�b�NX
}
LONG Input::GetPadAxisY(int padIndex) {
	return padStates[padIndex].lY; // ���X�e�B�b�NY
}

