
#include "readApps.h"
#include "utils.h"
#include <cstdlib>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>

AppReader::AppReader() = default;
AppReader::~AppReader() = default;

void AppReader::LoadApps()
{
  // Priority: user dirs first, then system
  std::vector<std::string> priorityDirs = {
      std::string(getenv("HOME")) + "/.local/share/applications",
      "/usr/local/share/applications",
      "/usr/share/applications"};
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

    // detect section headers
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
  }

  return result;
}
