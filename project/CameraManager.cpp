#include <cassert>

#include "CameraManager.h"
#include "Camera.h"

CameraManager* CameraManager::GetInstance() {
    static CameraManager instance;
    return &instance;
}

void CameraManager::AddCamera(const std::string& name, Camera* camera) {
    assert(camera);
    cameras_[name] = camera;
}

void CameraManager::SetActiveCamera(const std::string& name) {
    assert(cameras_.count(name));
    activeCamera_ = cameras_[name];
}

Camera* CameraManager::GetActiveCamera() const {
    assert(activeCamera_);
    return activeCamera_;
}

// íœ
void CameraManager::RemoveCamera(const std::string& name) {
    cameras_.erase(name);
}

void CameraManager::Update() {
    if (activeCamera_) {
        activeCamera_->Update();
    }
}
