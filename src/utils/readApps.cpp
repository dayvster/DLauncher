
#include "readApps.h"
#include "utils.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

AppReader::AppReader() = default;
AppReader::~AppReader() = default;

void AppReader::LoadApps() {
  for (const auto &path : desktopAppPaths) {
    std::string expandedPath = path;
    if (starts_with(expandedPath, "~")) {
      const char *home = getenv("HOME");
      if (home)
        expandedPath.replace(0, 1, home);
    }

    if (!std::filesystem::exists(expandedPath))
      continue;

    try {
      for (const auto &entry :
           std::filesystem::directory_iterator(expandedPath)) {
        if (entry.path().extension() != ".desktop")
          continue;
        allAps.push_back(parseDesktopApp(entry.path()));
      }
    } catch (const std::filesystem::filesystem_error &e) {
      std::cerr << "Filesystem error: " << e.what() << std::endl;
      continue;
    }
  }
}

std::vector<DesktopApp>
AppReader::ReadDesktopApps(int limit, const std::string &searchTerm) {
  std::vector<DesktopApp> filtered;
  std::string searchLower = toLower(searchTerm);

  for (const auto &app : allAps) {
    if (searchTerm.empty() ||
        toLower(app.name).find(searchLower) != std::string::npos) {
      filtered.push_back(app);
    }
  }

  if (limit > 0 && filtered.size() > static_cast<size_t>(limit)) {
    filtered.resize(limit);
  }

  return filtered;
}

std::vector<DesktopApp> AppReader::GetAllApps() { return allAps; }

std::vector<DesktopApp> AppReader::SearchApps(std::string searchTerm, int limit,
                                              bool isFuzzy) {
  std::vector<DesktopApp> results;
  std::string searchLower = toLower(searchTerm);

  if (isFuzzy) {
    for (const auto &app : allAps) {
      std::string appNameLower = toLower(app.name);
      std::string exec = toLower(app.exec);
      if (similarity(appNameLower, searchLower) > 0.4) {
        results.push_back(app);
      }
      if (contains(exec, searchLower, false) != std::string::npos) {
        results.push_back(app);
      }
    }

    if (limit > 0 && results.size() > static_cast<size_t>(limit)) {
      results.resize(limit);
    }
  }
  return results;
}

DesktopApp AppReader::parseDesktopApp(const std::filesystem::path &path) {
  DesktopApp result;

  std::ifstream f(path);
  if (!f.is_open())
    return result;

  std::stringstream buffer;
  buffer << f.rdbuf();
  std::string contents = buffer.str();

  auto lines = toStringArray(contents, "\n");

  bool inDesktopEntry = false;

  for (const auto &line : lines) {
    std::string trimmed = trim(line);

    if (trimmed.empty() || trimmed[0] == '#')
      continue;

    // detect section headers
    if (trimmed.front() == '[' && trimmed.back() == ']') {
      std::string section = toLower(trimmed);
      if (section == "[desktop entry]") {
        inDesktopEntry = true;
        continue;
      } else {
        // stop parsing when we hit any other section
        break;
      }
    }

    if (!inDesktopEntry)
      continue;

    auto equalsIndex = line.find('=');
    if (equalsIndex == std::string::npos)
      continue;

    std::string arg = trim(line.substr(0, equalsIndex));
    std::string value = trim(line.substr(equalsIndex + 1));
    std::string argLower = toLower(arg);

    if (argLower == "name") {
      result.name = value;
    } else if (argLower.rfind("name[", 0) == 0 && result.name.empty()) {
      result.name = value;
    } else if (argLower == "exec") {
      result.exec = value;
    } else if (argLower == "icon") {
      result.icon = value;
    } else if (argLower == "comment") {
      result.comment = value;
    } else if (argLower == "categories") {
      result.categories = toStringArray(value, ";");
    }
  }

  return result;
}
