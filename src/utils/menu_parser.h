#pragma once
#include <string>
#include <vector>

// Parse menu file lines into (label, command) pairs.
std::vector<std::pair<std::string,std::string>> parseMenuFile(const std::string &path);
std::pair<std::string,std::string> parseMenuItemString(const std::string &line);
