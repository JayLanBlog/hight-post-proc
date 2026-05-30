#pragma once

#include <memory>

class IRenderBackend;

class Scene {
public:
    virtual ~Scene() = default;
    virtual void OnEnter() {}
    virtual void OnExit() {}
    virtual void OnUpdate(float dt) = 0;
    virtual void OnRender(IRenderBackend* backend) = 0;
    virtual void OnImGui() {}
    virtual bool WantsExit() const { return false; }
    virtual std::unique_ptr<Scene> GetNextScene() { return nullptr; }
};
