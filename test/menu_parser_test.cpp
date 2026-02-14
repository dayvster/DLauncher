#include <iostream>
#include <cassert>
#include "../src/utils/menu_parser.h"

int main() {
  auto p1 = parseMenuItemString("Label::echo hello");
  assert(p1.first=="Label" && p1.second=="echo hello");

  auto p2 = parseMenuItemString("Just a command");
  assert(p2.first=="Just a command" && p2.second=="Just a command");

  auto p3 = parseMenuItemString("A\tcmd arg");
  assert(p3.first=="A" && p3.second=="cmd arg");

  auto p4 = parseMenuItemString("X|/bin/ls -l");
  assert(p4.first=="X" && p4.second=="/bin/ls -l");

  std::cout<<"menu_parser tests passed\n";
  return 0;
}
