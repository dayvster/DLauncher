#include "xdg.h"
#include <cstdlib>

namespace xdg {

std::filesystem::path configHome()
{
  const char *xdg = getenv("XDG_CONFIG_HOME");
  if (xdg && xdg[0] != '\0') return std::filesystem::path(xdg);
  const char *home = getenv("HOME");
  if (home && home[0] != '\0') return std::filesystem::path(home) / ".config";
  return std::filesystem::path(".");
}

std::filesystem::path cacheHome()
{
  const char *xdg = getenv("XDG_CACHE_HOME");
  if (xdg && xdg[0] != '\0') return std::filesystem::path(xdg);
  const char *home = getenv("HOME");
  if (home && home[0] != '\0') return std::filesystem::path(home) / ".cache";
  return std::filesystem::path(".");
}

std::filesystem::path home()
{
  const char *h = getenv("HOME");
  if (h && h[0] != '\0') return std::filesystem::path(h);
  return std::filesystem::path(".");
}

void ensureDir(const std::filesystem::path &p)
{
  try {
    if (p.empty()) return;
    std::filesystem::create_directories(p);
  } catch(...) {}
}

std::filesystem::path configPath(const std::string &relPath)
{
  return configHome() / relPath;
}

std::filesystem::path cachePath(const std::string &relPath)
{
  return cacheHome() / relPath;
}

} // namespace xdg
