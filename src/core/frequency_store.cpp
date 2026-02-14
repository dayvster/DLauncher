#include "frequency_store.h"
#include "../utils/json.hpp"
#include "xdg.h"
#include <filesystem>

FrequencyStore::FrequencyStore(const std::string &p)
    : path(p)
{
  // load into temporary ordered map then populate unordered_map
  try {
    std::map<std::string,int> tmp = json_util::load_freq(path);
    store.reserve(tmp.size() + 64);
    for (const auto &p : tmp) store.emplace(p.first, p.second);
  } catch(...) {}
  dirtyFlag = false;
}

int FrequencyStore::get(const std::string &key) const
{
  auto it = store.find(key);
  if (it == store.end()) return 0;
  return it->second;
}

void FrequencyStore::inc(const std::string &key, int by)
{
  store[key] += by;
  dirtyFlag = true;
}

void FrequencyStore::save()
{
  if (!dirtyFlag) return;
  // ensure parent dir exists
  std::filesystem::path p(path);
  if (!p.parent_path().empty()) xdg::ensureDir(p.parent_path());
  // json_util expects a std::map; build one from unordered_map
  std::map<std::string,int> out;
  out.insert(store.begin(), store.end());
  json_util::save_freq(path, out);
  dirtyFlag = false;
}
