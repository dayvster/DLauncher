// Simple integration test: create a temp directory with a .desktop file and
// verify AppReader discovers it when scanning.
#include <filesystem>
#include <fstream>
#include <cassert>
#include "../src/core/apps/readApps.h"
#include "../src/core/intern.h"

int main() {
  namespace fs = std::filesystem;
  fs::path tmp = fs::temp_directory_path() / "dlauncher_test_apps";
  fs::create_directories(tmp);
  fs::path desktop = tmp / "TestDmenu.desktop";
  std::ofstream f(desktop);
  f << "[Desktop Entry]\n";
  f << "Name=TestDmenu\n";
  f << "Exec=/bin/echo hello\n";
  f << "NoDisplay=true\n"; // typical dmenu helper might set this
  f.close();

  AppReader r;
  // point scanner at our temp dir
  r.SetDesktopAppPaths({ tmp.string() });
  r.SetCacheEnabled(false);
  r.LoadApps(true); // include hidden by default to accept NoDisplay
  auto apps = r.GetAllApps();
  bool found = false;
  for (const auto &a : apps) {
    if (a.name == "TestDmenu" && a.exec == "/bin/echo hello") { found = true; break; }
  }
  assert(found && "Test desktop app should be discovered");

  // cleanup
  fs::remove(desktop);
  fs::remove(tmp);
  return 0;
}
