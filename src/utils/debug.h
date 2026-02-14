#pragma once

#include <iostream>
#include <string>
#include <vector>

class Debug
{
public:
  static void log(const char *message);
  static void log(const std::string &message);
  static void setVerbosity(int v);
  static int verbosity();

  static const std::vector<std::string> adjectives;
  static const std::vector<std::string> nouns;

  static std::string generateRandomString();
};
