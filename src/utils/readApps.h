
#pragma once
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

struct DesktopApp {
  std::string name;
  std::string exec;
  std::optional<std::string> icon;
  std::optional<std::string> comment;
  std::vector<std::string> categories;
};

class AppReader {
public:
  AppReader();
  ~AppReader();

  void LoadApps();

  std::vector<DesktopApp> ReadDesktopApps(int limit = -1,
                                          const std::string &searchTerm = "");
  std::vector<DesktopApp> GetAllApps();
  std::vector<DesktopApp> SearchApps(std::string searchTerm, int limit = -1,
                                     bool isFuzzy = true);

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

  std::vector<DesktopApp> allAps;
};
