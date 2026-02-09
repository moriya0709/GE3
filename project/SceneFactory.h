#pragma once
#include "AbstractSceneFactory.h"
#include "TitleScene.h"
#include "GamePlayScene.h"

class SceneFactory : public AbstractSceneFactory{
public:
	// ÉVÅ[Éìê∂ê¨
	BaseScene* CreateScene(const std::string& sceneName) override;

};

