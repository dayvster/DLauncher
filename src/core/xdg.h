#pragma once
#include <filesystem>
#include <string>

namespace xdg {
// Returns XDG config home (XDG_CONFIG_HOME or $HOME/.config)
std::filesystem::path configHome();
// Returns XDG cache home (XDG_CACHE_HOME or $HOME/.cache)
std::filesystem::path cacheHome();
// Return $HOME directory path (or '.' fallback)
std::filesystem::path home();
// Ensure a directory exists (creates parents as needed)
void ensureDir(const std::filesystem::path &p);
// Helper to build a config path under XDG config
std::filesystem::path configPath(const std::string &relPath);
// Helper to build a cache path under XDG cache
std::filesystem::path cachePath(const std::string &relPath);
} // namespace xdg
