#include <QString>
#include <QStringList>

#pragma once
#include <string>
#include <vector>

<<<<<<< HEAD
// Returns: program, arguments, and a list of env assignments (e.g. "KEY=VAL") to apply.
// Optional parameters allow expanding .desktop placeholders:
// - fileArgs: files/URLs to expand for %f/%F/%u/%U
// - appName: expanded for %c
// - iconName: expanded for %i
// - desktopFilePath: expanded for %k
std::tuple<QString, QStringList, QStringList> parseExecCommand(
    const QString &exec,
    const QStringList &fileArgs = QStringList(),
    const QString &appName = QString(),
    const QString &iconName = QString(),
    const QString &desktopFilePath = QString());
=======
std::pair<QString, QStringList> parseExecCommand(const QString &exec);
>>>>>>> origin/main
std::vector<std::string> toStringArray(const std::string &content,
                                       const std::string &delimiter = "\n");

size_t contains(const std::string &str, const std::string &search,
                const bool caseSensetive = true);

std::string toLower(const std::string &str);
std::string toLower(const std::string &s);
std::string ltrim(const std::string &s);
std::string rtrim(const std::string &s);
std::string trim(const std::string &s);
bool starts_with(const std::string &s, const std::string &prefix);
std::vector<std::string> toStringArray(const std::string &s,
                                       const std::string &delimiter);

int damerauLevenshtein(const std::string &s1, const std::string &s2);
double similarity(const std::string &a, const std::string &b);

enum class PathType
{
  NotAPath = 0,
  File = 1,
  Directory = 2
};

<<<<<<< HEAD
PathType detectPathType(const std::string &search);
=======
PathType detectPathType(const std::string &search);
>>>>>>> origin/main
