#pragma once
#include <string>
#include <vector>
#include <functional>
#include <memory>

class Scene;

struct SceneEntry {
    std::string id;
    std::string name;
    std::string category;
    std::string description;
    std::string thumbPath;
    std::function<std::unique_ptr<Scene>()> factory;
    bool available = true;
};

class SceneRegistry {
public:
    static SceneRegistry& Instance();
    void Register(SceneEntry entry);
    const std::vector<SceneEntry>& All() const { return m_entries; }
    std::vector<std::string> Categories() const;
    std::vector<SceneEntry> ByCategory(const std::string& cat) const;
private:
    SceneRegistry() = default;
    std::vector<SceneEntry> m_entries;
};
