#include "preferences.h"
#include "xdg.h"
#include <fstream>
#include <sstream>

namespace prefs {

Preferences::Preferences(const std::filesystem::path &path) : file(path) { load(); }
Preferences::~Preferences() { }

void Preferences::load() {
  map.clear();
  try {
    if (!std::filesystem::exists(file)) return;
    std::ifstream f(file);
    if (!f.is_open()) return;
    std::string line;
    while (std::getline(f, line)) {
      if (line.empty()) continue;
      // format: key\t<pinned 0|1>\t<hidden 0|1>
      std::istringstream iss(line);
      std::string key; int pinned = 0; int hidden = 0;
      if (!std::getline(iss, key, '\t')) continue;
      std::string s1; if (!std::getline(iss, s1, '\t')) continue; pinned = std::stoi(s1);
      std::string s2; if (!std::getline(iss, s2, '\t')) continue; hidden = std::stoi(s2);
      Entry e; e.pinned = (pinned != 0); e.hidden = (hidden != 0);
      map[key] = e;
    }
  } catch(...) {}
}

void Preferences::save() {
  try {
    std::filesystem::create_directories(file.parent_path());
    std::ofstream f(file, std::ios::trunc);
    if (!f.is_open()) return;
    for (auto &p : map) {
      f << p.first << '\t' << (p.second.pinned ? 1 : 0) << '\t' << (p.second.hidden ? 1 : 0) << '\n';
    }
  } catch(...) {}
}

bool Preferences::isPinned(const std::string &key) const { auto it = map.find(key); if (it == map.end()) return false; return it->second.pinned; }
void Preferences::setPinned(const std::string &key, bool v) { map[key].pinned = v; }

bool Preferences::isHidden(const std::string &key) const { auto it = map.find(key); if (it == map.end()) return false; return it->second.hidden; }
void Preferences::setHidden(const std::string &key, bool v) { map[key].hidden = v; }

} // namespace prefs
