#include "Input.h"

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
