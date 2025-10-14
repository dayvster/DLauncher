#include "debug.h"
#include <iostream>
#include <string>

void Debug::log(const char *message)
{
  std::cout << message << std::endl;
}

void Debug::log(const std::string &message)
{
  std::cout << message << std::endl;
}

const std::vector<std::string> Debug::adjectives = {"quick", "lazy", "sleepy",
                                                    "noisy", "hungry", "happy",
                                                    "sad", "angry", "brave"};

const std::vector<std::string> Debug::nouns = {"fox", "dog", "head", "leg", "tail",
                                               "cat", "mouse", "house", "car"};

std::string Debug::generateRandomString()
{
  static bool seeded = false;
  if (!seeded)
  {
    srand(time(nullptr));
    seeded = true;
  }
  std::string adjective = Debug::adjectives[rand() % Debug::adjectives.size()];
  std::string noun = Debug::nouns[rand() % Debug::nouns.size()];
  return adjective + "_" + noun;
}
