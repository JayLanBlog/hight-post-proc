#include "app/SceneRegistry.h"
#include <set>

SceneRegistry& SceneRegistry::Instance() {
    static SceneRegistry inst;
    return inst;
}

void SceneRegistry::Register(SceneEntry entry) {
    m_entries.push_back(std::move(entry));
}

std::vector<std::string> SceneRegistry::Categories() const {
    std::set<std::string> cats;
    for (auto& e : m_entries) cats.insert(e.category);
    return std::vector<std::string>(cats.begin(), cats.end());
}

std::vector<SceneEntry> SceneRegistry::ByCategory(const std::string& cat) const {
    if (cat == "全部") return m_entries;
    std::vector<SceneEntry> result;
    for (auto& e : m_entries)
        if (e.category == cat) result.push_back(e);
    return result;
}
