#pragma once
#include <string>
#include <unordered_map>
#include <filesystem>

namespace prefs {
class Preferences {
public:
  explicit Preferences(const std::filesystem::path &path);
  ~Preferences();

  bool isPinned(const std::string &key) const;
  void setPinned(const std::string &key, bool v);

  bool isHidden(const std::string &key) const;
  void setHidden(const std::string &key, bool v);

  void save();
  void load();

private:
  std::filesystem::path file;
  struct Entry { bool pinned = false; bool hidden = false; };
  std::unordered_map<std::string, Entry> map;
};
} // namespace prefs
