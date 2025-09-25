// Parses a .desktop Exec string, removes field codes, splits into program and args.
// Returns a pair: (program, args)
#include <QString>
#include <QStringList>
std::pair<QString, QStringList> parseExecCommand(const QString &exec);

#pragma once
#include <string>
#include <vector>

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
