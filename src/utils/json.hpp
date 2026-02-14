// Single-header nlohmann/json for frequency persistence
// You can replace this with the official header if desired
// Minimal version for map<string, int> serialization
#pragma once
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>

// Lightweight utilities for persistent frequency storage.
// Keys are percent-encoded to safely store arbitrary exec strings on line-based files.
namespace json_util
{
inline std::string percent_encode(const std::string &s)
{
  std::ostringstream out;
  out << std::hex << std::uppercase;
  for (unsigned char c : s)
  {
    // Unreserved characters per RFC3986: ALPHA / DIGIT / '-' / '.' / '_' / '~'
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '.' || c == '_' || c == '~')
    {
      out << c;
    }
    else
    {
      out << '%' << std::setw(2) << std::setfill('0') << int(c);
    }
  }
  return out.str();
}

inline std::string percent_decode(const std::string &s)
{
  std::ostringstream out;
  for (size_t i = 0; i < s.size(); ++i)
  {
    if (s[i] == '%' && i + 2 < s.size())
    {
      std::istringstream iss(s.substr(i + 1, 2));
      int value;
      if (iss >> std::hex >> value)
      {
        out << static_cast<char>(value);
        i += 2;
        continue;
      }
    }
    out << s[i];
  }
  return out.str();
}

inline std::map<std::string, int> load_freq(const std::string &path)
{
  std::map<std::string, int> freq;
  std::ifstream f(path);
  if (!f.is_open())
    return freq;
  std::string line;
  while (std::getline(f, line))
  {
    auto pos = line.find(':');
    if (pos == std::string::npos)
      continue;
    std::string key_enc = line.substr(0, pos);
    std::string valstr = line.substr(pos + 1);
    try
    {
      int val = std::stoi(valstr);
      std::string key = percent_decode(key_enc);
      freq[key] = val;
    }
    catch (...) {
      // skip malformed line
    }
  }
  return freq;
}

inline void save_freq(const std::string &path, const std::map<std::string, int> &freq)
{
  // Ensure parent directory exists
  std::filesystem::path p(path);
  std::filesystem::path parent = p.parent_path();
  if (!parent.empty())
    std::filesystem::create_directories(parent);

  // Write atomically: write to temp file then rename
  std::filesystem::path tmp = parent / (p.filename().string() + ".tmp");
  std::ofstream f(tmp, std::ios::trunc);
  for (const auto &p : freq)
  {
    if (p.second <= 0) continue;
    f << percent_encode(p.first) << ":" << p.second << "\n";
  }
  f.close();
  std::error_code ec;
  std::filesystem::rename(tmp, path, ec);
  if (ec)
  {
    // best-effort fallback: try overwrite
    std::filesystem::create_directories(parent);
    std::ofstream f2(path, std::ios::trunc);
    for (const auto &p : freq)
    {
      if (p.second <= 0) continue;
      f2 << percent_encode(p.first) << ":" << p.second << "\n";
    }
  }
}

} // namespace json_util
