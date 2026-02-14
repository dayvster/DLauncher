#include "menu_parser.h"
#include <fstream>
#include <sstream>

std::pair<std::string,std::string> parseMenuItemString(const std::string &line) {
  auto trimmed = line;
  // trim
  while (!trimmed.empty() && (trimmed.back()=='\n' || trimmed.back()=='\r')) trimmed.pop_back();
  if (trimmed.empty()) return {"",""};
  size_t pos = trimmed.find("::");
  if (pos!=std::string::npos) return {trimmed.substr(0,pos), trimmed.substr(pos+2)};
  pos = trimmed.find('\t');
  if (pos!=std::string::npos) return {trimmed.substr(0,pos), trimmed.substr(pos+1)};
  pos = trimmed.find('|');
  if (pos!=std::string::npos) return {trimmed.substr(0,pos), trimmed.substr(pos+1)};
  return {trimmed, trimmed};
}

std::vector<std::pair<std::string,std::string>> parseMenuFile(const std::string &path) {
  std::vector<std::pair<std::string,std::string>> out;
  std::ifstream f(path);
  if (!f.is_open()) return out;
  std::string line;
  while (std::getline(f, line)) {
    auto p = parseMenuItemString(line);
    if (!p.first.empty() && !p.second.empty()) out.push_back(p);
  }
  return out;
}
