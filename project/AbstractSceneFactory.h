#pragma once
#include<string>

#include "BaseScene.h"

class AbstractSceneFactory {
public:
	// 仮想デストラクタ
	virtual ~AbstractSceneFactory() = default;
	// シーン生成
	virtual BaseScene* CreateScene(const std::string& sceneNama) = 0;




};

