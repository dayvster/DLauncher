// Test prefs persistence: set pinned/hidden flags, save, reload, verify
#include <filesystem>
#include <cassert>
#include <fstream>
#include "../src/core/preferences.h"

int main() {
  namespace fs = std::filesystem;
  fs::path tmp = fs::temp_directory_path() / "dlauncher_prefs_test";
  fs::create_directories(tmp.parent_path());
  fs::path prefsFile = tmp;

  // Ensure clean
  if (fs::exists(prefsFile)) fs::remove(prefsFile);

  prefs::Preferences p(prefsFile);
  // initially false
  assert(!p.isPinned("/bin/testapp"));
  assert(!p.isHidden("/bin/testapp"));

  p.setPinned("/bin/testapp", true);
  p.setHidden("/bin/testapp", true);
  p.save();

  prefs::Preferences p2(prefsFile);
  assert(p2.isPinned("/bin/testapp"));
  assert(p2.isHidden("/bin/testapp"));

  // cleanup
  fs::remove(prefsFile);
  return 0;
}
