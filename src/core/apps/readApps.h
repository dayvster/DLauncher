#pragma once
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

struct DesktopApp
{
  std::string name;
  std::string exec;
  // Precomputed lower-cased name for fast search
  std::string name_lc;
  // Precomputed normalized exec string (lower-cased, trimmed, up to '%' placeholder)
  std::string exec_lc;
  // Interned pointer to canonical exec string (stored in intern pool)
  const std::string *exec_intern = nullptr;
  std::optional<std::string> icon;
  std::optional<std::string> comment;
  bool noDisplay = false;
  bool hidden = false;
  std::vector<std::string> categories;
};

class AppReader
{
public:
  AppReader();
  ~AppReader();

  // includeHidden: if true, do not skip entries marked NoDisplay or Hidden
  void LoadApps(bool includeHidden = false);
  // Dump scan diagnostics to stdout: for each .desktop file print path and
  // whether it was included or skipped (with reason). Useful for --dump CLI.
  void DumpAndPrint(bool includeHidden = false);
  // Save currently loaded apps to the on-disk cache (creates directories as needed)
  void SaveCache();

  std::vector<DesktopApp> ReadDesktopApps(int limit = -1,
                                          const std::string &searchTerm = "");
  const std::vector<DesktopApp> &GetAllApps() const;
  std::vector<DesktopApp> SearchApps(std::string searchTerm, int limit = -1,
                                     bool isFuzzy = true);

  // Cache management: Load apps from cache when possible to speed startup.
  // Cache is stored in XDG cache dir (or ~/.cache) as 'dlauncher/apps.cache'.
  void SetCacheEnabled(bool enabled) { useCache = enabled; }

private:
  DesktopApp parseDesktopApp(const std::filesystem::path &path);

  const std::vector<std::string> desktopAppPaths = {
      "/usr/share/applications",
      "/usr/local/share/applications",
      "~/.local/share/applications",
      "~/.local/share/flatpak/exports/share/applications",
      "/var/lib/flatpak/exports/share/applications",
      "/var/lib/snapd/desktop/applications",
      "/var/lib/flatpak/applications",
      "/var/lib/flatpak/exports/share/applications"};

  std::vector<DesktopApp> allApps;
  bool useCache = true;
};
