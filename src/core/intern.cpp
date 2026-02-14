#include "intern.h"
#include <string>
#include <unordered_set>
#include <mutex>

namespace intern {
static std::unordered_set<std::string> pool;
static std::mutex pool_mtx;

const std::string *intern_string(const std::string &s)
{
  std::lock_guard<std::mutex> lk(pool_mtx);
  auto it = pool.find(s);
  if (it != pool.end()) return &*it;
  // insert and return pointer
  auto res = pool.insert(s);
  return &*res.first;
}

}
