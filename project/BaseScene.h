#pragma once

class DirectXCommon;
class Camera;

class BaseScene {
public:
	// ‰Šú‰»
	virtual void Initialize(DirectXCommon* dxCommon) = 0;
	// XV
	virtual void Update() = 0;
	// •`‰æ
	virtual void Draw3D() = 0;
	virtual void Draw2D() = 0;
	// I—¹
	virtual void Finalize() = 0;

	// getter
	virtual Camera* GetCamera() = 0;

	virtual ~BaseScene() = default;

};

