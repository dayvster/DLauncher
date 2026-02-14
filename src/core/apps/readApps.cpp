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
#include "core/intern.h"

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

void AppReader::LoadApps(bool includeHidden, bool showSystem)
{
    // Ensure cache directory exists before attempting to read cache
    ensureCacheDir();

    const char* home = getenv("HOME");
    std::string homeStr = home ? std::string(home) : std::string();

  // Build scan directories from configured desktopAppPaths, expanding '~'
  std::vector<std::string> priorityDirs;
  for (const auto &p : desktopAppPaths) {
    if (!p.empty() && p[0] == '~') {
      if (!homeStr.empty()) priorityDirs.push_back(homeStr + p.substr(1));
    } else {
      priorityDirs.push_back(p);
    }
  }
  // Deduplicate while preserving order
  {
    std::set<std::string> seen;
    std::vector<std::string> dedup;
    for (auto &d : priorityDirs) {
      if (seen.insert(d).second) dedup.push_back(d);
    }
    priorityDirs.swap(dedup);
  }

  // Attempt to load from cache if enabled and present. But only accept cache
  // when it is newer than the scanned desktop files; otherwise force a rescan
  // so recently installed apps (snap/flatpak) aren't hidden by stale cache.
  if (useCache)
  {
    try
    {
      auto p = cachePath();
      if (std::filesystem::exists(p))
      {
        // compute newest mtime across desktop dirs
        std::filesystem::file_time_type newest = std::filesystem::file_time_type::min();
        for (const auto &d : priorityDirs) {
          try {
            if (!d.empty() && std::filesystem::exists(d)) {
              for (const auto &entry : std::filesystem::recursive_directory_iterator(d)) {
                try {
                  if (entry.path().extension() == ".desktop") {
                    auto fm = std::filesystem::last_write_time(entry.path());
                    if (fm > newest) newest = fm;
                  }
                } catch(...) {}
              }
            }
          } catch(...) {}
        }

        bool cache_is_fresh = true;
        try {
          auto cache_mtime = std::filesystem::last_write_time(p);
          if (newest > cache_mtime) cache_is_fresh = false;
        } catch(...) { cache_is_fresh = false; }

        if (cache_is_fresh) {
          std::ifstream f(p);
          if (f.is_open())
          {
            std::string line;
            size_t loaded = 0;
            while (std::getline(f, line))
            {
              if (line.empty()) continue;
              // Format (backwards compatible):
              // older: name\texec\ticon\tcomment\tnoDisplay\thidden\tcategories
              // newer: name\texec\ticon\tcomment\tnoDisplay\thidden\tcategories\tonlyshowin\tnotshowin
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
              if (parts.size() >= 9) {
                std::string onlysi = json_util::percent_decode(parts[7]);
                if (!onlysi.empty()) app.onlyShowIn = toStringArray(onlysi, ";");
                std::string notsi = json_util::percent_decode(parts[8]);
                if (!notsi.empty()) app.notShowIn = toStringArray(notsi, ";");
              }
              // Precompute lower-case and exec-normalized values
              app.name_lc = toLower(app.name);
              app.exec_lc = toLower(app.exec);
              size_t pct = app.exec_lc.find('%');
              if (pct != std::string::npos) app.exec_lc = app.exec_lc.substr(0, pct);
              // trim spaces
              app.exec_lc.erase(remove_if(app.exec_lc.begin(), app.exec_lc.end(), ::isspace), app.exec_lc.end());
              // intern exec string
              app.exec_intern = intern::intern_string(app.exec);
              // Optionally hide obvious system/config utilities unless showSystem is true
              std::string nm = toLower(app.name);
              bool likely_system = (nm.find("settings") != std::string::npos || nm.find("configuration") != std::string::npos || nm.find("system") != std::string::npos || nm.find("bluetooth") != std::string::npos || nm.find("printer") != std::string::npos || nm.find("authentication") != std::string::npos || nm.find("kcm_") != std::string::npos || nm.find("kcm ") != std::string::npos || nm.find("app permissions") != std::string::npos);
              if (!showSystem && likely_system) continue;
              allApps.push_back(std::move(app));
              ++loaded;
            }
            // Only use cache if it actually contained apps; otherwise fall through to scanning
            if (loaded > 0) {
              // Quick sanity check: if important desktop files exist on disk but
              // are missing from the cache, treat the cache as stale and fall
              // back to a full rescan. This handles cases where snap/flatpak
              // apps were installed after the cache was written.
              std::vector<std::string> critical = {"spotify"};
              bool critical_missing = false;
              for (const auto &name : critical) {
                bool found_on_disk = false;
                for (const auto &d : priorityDirs) {
                  try {
                    if (!d.empty() && std::filesystem::exists(d)) {
                      for (const auto &entry : std::filesystem::directory_iterator(d)) {
                        try {
                          if (entry.path().extension() == ".desktop") {
                            std::ifstream tf(entry.path());
                            if (!tf.is_open()) continue;
                            std::string contents((std::istreambuf_iterator<char>(tf)), std::istreambuf_iterator<char>());
                            if (toLower(contents).find(name) != std::string::npos) {
                              found_on_disk = true;
                              break;
                            }
                          }
                        } catch(...) {}
                      }
                    }
                  } catch(...) {}
                  if (found_on_disk) break;
                }
                if (found_on_disk) {
                  // ensure the cache also contains it
                  bool in_cache = false;
                  for (const auto &app : allApps) {
                    if (toLower(app.name).find(name) != std::string::npos) { in_cache = true; break; }
                  }
                  if (!in_cache) { critical_missing = true; break; }
                }
              }
              if (!critical_missing) return;
              // fall through to scanning: clear any partially loaded apps
              allApps.clear();
            }
          }
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
      // Allow symlinked desktop files and follow them
      std::filesystem::path p = entry.path();
      try { p = std::filesystem::canonical(entry.path()); } catch(...) {}
      DesktopApp app = parseDesktopApp(p);
        // Skip entries marked NoDisplay or Hidden unless requested
        if (!includeHidden && (app.noDisplay || app.hidden))
          continue;
        // Precompute normalized keys
        app.name_lc = toLower(app.name);
        app.exec_lc = toLower(app.exec);
        size_t pct = app.exec_lc.find('%');
        if (pct != std::string::npos) app.exec_lc = app.exec_lc.substr(0, pct);
        app.exec_lc.erase(remove_if(app.exec_lc.begin(), app.exec_lc.end(), ::isspace), app.exec_lc.end());
        app.exec_intern = intern::intern_string(app.exec);

        // Optionally hide system/config utilities by name
        std::string nm = toLower(app.name);
        bool likely_system = (nm.find("settings") != std::string::npos || nm.find("configuration") != std::string::npos || nm.find("system") != std::string::npos || nm.find("bluetooth") != std::string::npos || nm.find("printer") != std::string::npos || nm.find("authentication") != std::string::npos || nm.find("kcm_") != std::string::npos || nm.find("kcm ") != std::string::npos || nm.find("app permissions") != std::string::npos);
        if (!showSystem && likely_system) continue;

        std::string key_name = app.name_lc;
        key_name.erase(remove_if(key_name.begin(), key_name.end(), ::isspace), key_name.end());
        if (key_name.empty())
          continue;
        if (seenNames.find(key_name) == seenNames.end())
        {
          allApps.push_back(std::move(app));
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

void AppReader::DumpAndPrint(bool includeHidden, bool showSystem, const std::string &pattern)
{
  const char* home = getenv("HOME");
  std::string homeStr = home ? std::string(home) : std::string();
  std::vector<std::string> priorityDirs = {
      homeStr + "/.local/share/applications",
      "/usr/local/share/applications",
      "/usr/share/applications"};

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
        std::filesystem::path p = entry.path();
        try { p = std::filesystem::canonical(entry.path()); } catch(...) {}
        DesktopApp app = parseDesktopApp(p);
        bool skipped = false;
        std::string reason;
        if (!includeHidden && app.noDisplay)
        {
          skipped = true;
          reason = "NoDisplay";
        }
        else if (!includeHidden && app.hidden)
        {
          skipped = true;
          reason = "Hidden";
        }
        else if (!includeHidden && !app.onlyShowIn.empty())
        {
          skipped = true;
          reason = "OnlyShowIn";
        }
        else if (!includeHidden && !app.notShowIn.empty())
        {
          skipped = true;
          reason = "NotShowIn";
        }
        // system/config heuristics
        std::string nm = toLower(app.name);
        bool likely_system = (nm.find("settings") != std::string::npos || nm.find("configuration") != std::string::npos || nm.find("system") != std::string::npos || nm.find("bluetooth") != std::string::npos || nm.find("printer") != std::string::npos || nm.find("authentication") != std::string::npos || nm.find("kcm_") != std::string::npos || nm.find("kcm ") != std::string::npos || nm.find("app permissions") != std::string::npos);
        if (!showSystem && likely_system)
        {
          skipped = true;
          reason = "System";
        }
        std::string out;
        if (skipped)
        {
          out = entry.path().string() + "\tSKIPPED\t" + reason;
        }
        else
        {
          out = entry.path().string() + "\tINCLUDED\t" + app.name;
        }
        if (/* debug output controlled by flag */ true) {
          if (pattern.empty()) {
            // use std::cout for diagnostic dump mode (this function is a CLI
            // diagnostic helper; keep output on stdout regardless of -v so
            // scripts can consume it). Use std::cout here instead of Debug::log.
            std::cout << out << std::endl;
          } else {
            // case-insensitive match
            std::string low = toLower(out);
            std::string pat = toLower(pattern);
            if (low.find(pat) != std::string::npos) std::cout << out << std::endl;
          }
        }
      }
    }
    catch (const std::filesystem::filesystem_error &e)
    {
      std::cerr << "Filesystem error: " << e.what() << std::endl;
      continue;
    }
  }
  // After scanning from disk, persist to cache for faster startup next time
    if (useCache)
    {
      try {
      SaveCache();
      } catch(...) {}
    }
}

void AppReader::SaveCache()
{
  try {
    std::filesystem::path p = cachePath();
    std::filesystem::create_directories(p.parent_path());
    std::ofstream f(p, std::ios::trunc);
      for (const auto &app : allApps)
      {
        std::string name = json_util::percent_encode(app.name);
        std::string exec = json_util::percent_encode(app.exec);
        std::string icon = app.icon ? json_util::percent_encode(*app.icon) : std::string();
        std::string comment = app.comment ? json_util::percent_encode(*app.comment) : std::string();
        std::string noDisplay = app.noDisplay ? "1" : "0";
        std::string hidden = app.hidden ? "1" : "0";
        std::string cats;
        if (!app.categories.empty()) {
          std::ostringstream oss;
          for (size_t i = 0; i < app.categories.size(); ++i) {
            if (i) oss << ";";
            oss << app.categories[i];
          }
          cats = json_util::percent_encode(oss.str());
        }
        std::string onlysi;
        if (!app.onlyShowIn.empty()) {
          std::ostringstream oss2;
          for (size_t i = 0; i < app.onlyShowIn.size(); ++i) {
            if (i) oss2 << ";";
            oss2 << app.onlyShowIn[i];
          }
          onlysi = json_util::percent_encode(oss2.str());
        }
        std::string notsi;
        if (!app.notShowIn.empty()) {
          std::ostringstream oss3;
          for (size_t i = 0; i < app.notShowIn.size(); ++i) {
            if (i) oss3 << ";";
            oss3 << app.notShowIn[i];
          }
          notsi = json_util::percent_encode(oss3.str());
        }
        f << name << "\t" << exec << "\t" << icon << "\t" << comment << "\t" << noDisplay << "\t" << hidden << "\t" << cats << "\t" << onlysi << "\t" << notsi << "\n";
    }
    f.close();
  } catch(...) {}
}

std::vector<DesktopApp>
AppReader::ReadDesktopApps(int limit, const std::string &searchTerm)
{
  std::vector<DesktopApp> filtered;
  std::string searchLower = toLower(searchTerm);

  for (const auto &app : allApps)
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

const std::vector<DesktopApp> &AppReader::GetAllApps() const { return allApps; }

std::vector<DesktopApp> AppReader::SearchApps(std::string searchTerm, int limit,
                                              bool isFuzzy)
{
  std::vector<DesktopApp> results;
  std::string searchLower = toLower(searchTerm);

  if (isFuzzy)
  {
    std::set<std::pair<std::string, std::string>> seen;
    for (const auto &app : allApps)
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
    else if (argLower == "onlyshowin")
    {
      result.onlyShowIn = toStringArray(value, ";");
    }
    else if (argLower == "notshowin")
    {
      result.notShowIn = toStringArray(value, ";");
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
  // If name is empty, fall back to desktop filename (without extension)
  if (result.name.empty())
  {
    try {
      std::string fname = path.filename().string();
      if (!fname.empty()) {
        auto pos = fname.rfind('.');
        if (pos != std::string::npos) fname = fname.substr(0, pos);
        result.name = fname;
      }
    } catch (...) {}
  }

  return result;
}
