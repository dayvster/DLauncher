#pragma once
#include <string>
#include <unordered_map>

class FrequencyStore
{
public:
  FrequencyStore(const std::string &path);
  int get(const std::string &key) const;
  void inc(const std::string &key, int by = 1);
  void save();
  bool dirty() const { return dirtyFlag; }

private:
  std::string path;
  std::unordered_map<std::string, int> store;
  bool dirtyFlag = false;
};
