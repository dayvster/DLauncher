#include "preferences.h"
#include "xdg.h"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace prefs {

Preferences::Preferences(const std::filesystem::path &path) : file(path) { load(); }
Preferences::~Preferences() { }

void Preferences::load() {
  map.clear();
  try {
    if (!std::filesystem::exists(file)) return;
    std::ifstream f(file);
    if (!f.is_open()) return;
    json j; f >> j;
    for (auto it = j.begin(); it != j.end(); ++it) {
      Entry e;
      if (it.value().contains("pinned")) e.pinned = it.value()["pinned"].get<bool>();
      if (it.value().contains("hidden")) e.hidden = it.value()["hidden"].get<bool>();
      map[it.key()] = e;
    }
  } catch(...) {}
}

void Preferences::save() {
  try {
    json j;
    for (auto &p : map) {
      j[p.first] = { {"pinned", p.second.pinned}, {"hidden", p.second.hidden} };
    }
    std::filesystem::create_directories(file.parent_path());
    std::ofstream f(file, std::ios::trunc);
    f << j.dump(2);
  } catch(...) {}
}

bool Preferences::isPinned(const std::string &key) const {
  auto it = map.find(key); if (it == map.end()) return false; return it->second.pinned;
}
void Preferences::setPinned(const std::string &key, bool v) {
  map[key].pinned = v;
}

bool Preferences::isHidden(const std::string &key) const { auto it = map.find(key); if (it == map.end()) return false; return it->second.hidden; }
void Preferences::setHidden(const std::string &key, bool v) { map[key].hidden = v; }

} // namespace prefs
