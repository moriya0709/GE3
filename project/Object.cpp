#include "Object.h"

void Object::Initialize(ObjectCommon* objectCommon) {
	// 引数で受け取ってメンバ変数に記録する
	objectCommon_ = objectCommon;
	// モデル読み込み
	modelData = LoadObjFile("Resource", "plane.obj");

}

// .mtlファイルの読み込み
MaterialData Object::LoadMaterialTemplateFile(const std::string& directorypath, const std::string& filename) {
	return MaterialData();
}

// .objファイルの読み込み
ModelData Object::LoadObjFile(const std::string& dirextorypath, const std::string& filename) {
	return ModelData();
}
