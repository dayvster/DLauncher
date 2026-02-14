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

/*
  This was a fun little addition for generating random debug strings.
  It combines a random adjective and noun from predefined lists.
  Example output: "quick_fox", "lazy_dog", "happy_cat"

  the purpose is to have a unique identifier for each debug run,
  making it easier to trace logs from different executions.
*/
const std::vector<std::string> Debug::adjectives = {
    "quick", "lazy", "sleepy", "noisy", "hungry", "happy", "sad", "angry", "brave",
    "clever", "wild", "gentle", "fierce", "shy", "bold", "calm", "bright", "dark",
    "silent", "loud", "tiny", "giant", "swift", "slow", "curious", "funny", "serious",
    "mysterious", "friendly", "grumpy", "eager", "proud", "timid", "silly", "wise"};

const std::vector<std::string> Debug::nouns = {
    "fox", "dog", "head", "leg", "tail", "cat", "mouse", "house", "car",
    "wolf", "bear", "lion", "tiger", "eagle", "owl", "fish", "tree", "river",
    "mountain", "cloud", "star", "moon", "sun", "rock", "leaf", "flower", "book",
    "door", "window", "road", "field", "lake", "rain", "wind", "fire"};

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
