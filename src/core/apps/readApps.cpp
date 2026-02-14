#include "readApps.h"
#include "../../utils/utils.h"
#include <cstdlib>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include "../../utils/json.hpp"
#include <fstream>
#include <chrono>

AppReader::AppReader() = default;
AppReader::~AppReader() = default;

static std::filesystem::path cachePath()
{
  const char* xdg = getenv("XDG_CACHE_HOME");
  std::string base = xdg ? std::string(xdg) : (std::string(getenv("HOME")) + "/.cache");
  return std::filesystem::path(base) / "dlauncher" / "apps.cache";
}

static void ensureCacheDir()
{
  std::filesystem::create_directories(cachePath().parent_path());
}

void AppReader::LoadApps(bool includeHidden)
{

    const char* home = getenv("HOME");
    std::string homeStr = home ? std::string(home) : std::string();
  std::vector<std::string> priorityDirs = {
      homeStr + "/.local/share/applications",
      "/usr/local/share/applications",
      "/usr/share/applications"};

  // Attempt to load from cache if enabled and present
  if (useCache)
  {
    try
    {
      auto p = cachePath();
      if (std::filesystem::exists(p))
      {
        std::ifstream f(p);
        if (f.is_open())
        {
          std::string line;
          while (std::getline(f, line))
          {
            if (line.empty()) continue;
            // Format: name\texec\ticon\tcomment\tnoDisplay\thidden\tcategories
            std::vector<std::string> parts = toStringArray(line, "\t");
            if (parts.size() < 7) continue;
            DesktopApp app;
            app.name = json_util::percent_decode(parts[0]);
            app.exec = json_util::percent_decode(parts[1]);
            std::string iconStr = json_util::percent_decode(parts[2]);
            if (!iconStr.empty()) app.icon = iconStr;
            std::string commentStr = json_util::percent_decode(parts[3]);
            if (!commentStr.empty()) app.comment = commentStr;
            app.noDisplay = (parts[4] == "1");
            app.hidden = (parts[5] == "1");
            std::string cats = json_util::percent_decode(parts[6]);
            if (!cats.empty()) app.categories = toStringArray(cats, ";");
            allAps.push_back(app);
          }
          return;
        }
      }
    }
    catch (...) {}
  }
  std::set<std::string> seenNames;
    for (const auto &appDir : priorityDirs)
    {
    if (!std::filesystem::exists(appDir))
      continue;
    try
    {
        for (const auto &entry : std::filesystem::directory_iterator(appDir))
        {
          if (entry.path().extension() != ".desktop")
            continue;
        DesktopApp app = parseDesktopApp(entry.path());
        // Skip entries marked NoDisplay or Hidden unless requested
        if (!includeHidden && (app.noDisplay || app.hidden))
          continue;
        std::string key_name = toLower(app.name);
        key_name.erase(remove_if(key_name.begin(), key_name.end(), ::isspace), key_name.end());
        if (key_name.empty())
          continue;
        if (seenNames.find(key_name) == seenNames.end())
        {
          allAps.push_back(app);
          seenNames.insert(key_name);
        }
      }
    }
    catch (const std::filesystem::filesystem_error &e)
    {
      std::cerr << "Filesystem error: " << e.what() << std::endl;
      continue;
    }
  }
}

std::vector<DesktopApp>
AppReader::ReadDesktopApps(int limit, const std::string &searchTerm)
{
  std::vector<DesktopApp> filtered;
  std::string searchLower = toLower(searchTerm);

  for (const auto &app : allAps)
  {
    if (searchTerm.empty() ||
        toLower(app.name).find(searchLower) != std::string::npos)
    {
      filtered.push_back(app);
    }
  }

  if (limit > 0 && filtered.size() > static_cast<size_t>(limit))
  {
    filtered.resize(limit);
  }

  return filtered;
}

std::vector<DesktopApp> AppReader::GetAllApps() { return allAps; }

std::vector<DesktopApp> AppReader::SearchApps(std::string searchTerm, int limit,
                                              bool isFuzzy)
{
  std::vector<DesktopApp> results;
  std::string searchLower = toLower(searchTerm);

  if (isFuzzy)
  {
    std::set<std::pair<std::string, std::string>> seen;
    for (const auto &app : allAps)
    {
      std::string appNameLower = toLower(app.name);
      appNameLower.erase(remove_if(appNameLower.begin(), appNameLower.end(), ::isspace), appNameLower.end());
      std::string exec = toLower(app.exec);
      size_t percent = exec.find('%');
      if (percent != std::string::npos)
        exec = exec.substr(0, percent);
      exec.erase(remove_if(exec.begin(), exec.end(), ::isspace), exec.end());
      if (similarity(appNameLower, searchLower) > 0.4 ||
          contains(exec, searchLower, false) != std::string::npos)
      {
        auto key = std::make_pair(appNameLower, exec);
        if (seen.find(key) == seen.end())
        {
          results.push_back(app);
          seen.insert(key);
        }
      }
    }
    if (limit > 0 && results.size() > static_cast<size_t>(limit))
    {
      results.resize(limit);
    }
  }
  return results;
}

DesktopApp AppReader::parseDesktopApp(const std::filesystem::path &path)
{
  DesktopApp result;

  std::ifstream f(path);
  if (!f.is_open())
    return result;

  std::stringstream buffer;
  buffer << f.rdbuf();
  std::string contents = buffer.str();

  auto lines = toStringArray(contents, "\n");

  bool inDesktopEntry = false;

  for (const auto &line : lines)
  {
    std::string trimmed = trim(line);

    if (trimmed.empty() || trimmed[0] == '#')
      continue;

    if (trimmed.front() == '[' && trimmed.back() == ']')
    {
      std::string section = toLower(trimmed);
      if (section == "[desktop entry]")
      {
        inDesktopEntry = true;
        continue;
      }
      else
      {

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

    if (argLower == "name")
    {
      result.name = value;
    }
    else if (argLower.rfind("name[", 0) == 0 && result.name.empty())
    {
      result.name = value;
    }
    else if (argLower == "exec")
    {
      result.exec = value;
    }
    else if (argLower == "icon")
    {
      result.icon = value;
    }
    else if (argLower == "comment")
    {
      result.comment = value;
    }
    else if (argLower == "categories")
    {
      result.categories = toStringArray(value, ";");
    }
    else if (argLower == "nodisplay")
    {
      std::string v = toLower(value);
      result.noDisplay = (v == "true" || v == "1");
    }
    else if (argLower == "hidden")
    {
      std::string v = toLower(value);
      result.hidden = (v == "true" || v == "1");
    }
  }

  return result;
}
