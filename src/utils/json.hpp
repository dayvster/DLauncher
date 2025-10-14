// Single-header nlohmann/json for frequency persistence
// You can replace this with the official header if desired
// Minimal version for map<string, int> serialization
#pragma once
#include <map>
#include <string>
#include <fstream>
#include <sstream>

namespace json_util
{
  inline std::map<std::string, int> load_freq(const std::string &path)
  {
    std::map<std::string, int> freq;
    std::ifstream f(path);
    std::string line;
    while (std::getline(f, line))
    {
      auto pos = line.find(':');
      if (pos != std::string::npos)
      {
        std::string key = line.substr(0, pos);
        try
        {
          int val = std::stoi(line.substr(pos + 1));
          freq[key] = val;
        }
        catch (const std::invalid_argument &)
        {
          // skip malformed line
        }
        catch (const std::out_of_range &)
        {
          // skip malformed line
        }
      }
    }
    return freq;
  }
  inline void save_freq(const std::string &path, const std::map<std::string, int> &freq)
  {
    std::ofstream f(path);
    for (const auto &[key, val] : freq)
    {
      f << key << ":" << val << "\n";
    }
  }
} // namespace json_util
