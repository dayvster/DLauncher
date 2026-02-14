#include "frequency_store.h"
#include "../utils/json.hpp"
#include "xdg.h"
#include <filesystem>

FrequencyStore::FrequencyStore(const std::string &p)
    : path(p)
{
  store = json_util::load_freq(path);
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
  json_util::save_freq(path, store);
  dirtyFlag = false;
}
