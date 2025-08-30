
#include "utils.h"
#include <algorithm>

std::vector<std::string> toStringArray(const std::string &content,
                                       const std::string &delimiter) {
  std::vector<std::string> result;
  size_t start = 0, end;

  if (delimiter.empty())
    return {content};
  if (content.empty())
    return {};

  while ((end = content.find(delimiter, start)) != std::string::npos) {
    if (end != start)
      result.push_back(content.substr(start, end - start));
    start = end + delimiter.size();
  }

  if (start < content.size())
    result.push_back(content.substr(start));

  return result;
}

size_t contains(const std::string &str, const std::string &search,
                bool caseSensitive) {
  if (search.empty())
    return std::string::npos;

  if (caseSensitive) {
    auto pos = str.find(search);
    return (pos != std::string::npos) ? static_cast<unsigned int>(pos)
                                      : std::string::npos;
  } else {
    auto it =
        std::search(str.begin(), str.end(), search.begin(), search.end(),
                    [](char a, char b) {
                      return std::tolower(static_cast<unsigned char>(a)) ==
                             std::tolower(static_cast<unsigned char>(b));
                    });
    if (it != str.end()) {
      return static_cast<unsigned int>(it - str.begin());
    } else {
      return std::string::npos;
    }
  }
}

std::string toLower(const std::string &str) {
  std::string result = str;
  std::transform(result.begin(), result.end(), result.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return result;
}

std::string ltrim(const std::string &s) {
  std::string copy = s;
  copy.erase(copy.begin(),
             std::find_if(copy.begin(), copy.end(),
                          [](unsigned char ch) { return !std::isspace(ch); }));
  return copy;
}

std::string rtrim(const std::string &s) {
  std::string copy = s;
  copy.erase(std::find_if(copy.rbegin(), copy.rend(),
                          [](unsigned char ch) { return !std::isspace(ch); })
                 .base(),
             copy.end());
  return copy;
}

std::string trim(const std::string &s) { return ltrim(rtrim(s)); }

bool starts_with(const std::string &s, const std::string &prefix) {
  if (prefix.size() > s.size())
    return false;
  return std::equal(prefix.begin(), prefix.end(), s.begin());
}

int damerauLevenshtein(const std::string &s1, const std::string &s2) {
  size_t m = s1.size();
  size_t n = s2.size();
  std::vector<std::vector<int>> d(m + 1, std::vector<int>(n + 1));

  for (size_t i = 0; i <= m; ++i)
    d[i][0] = i;
  for (size_t j = 0; j <= n; ++j)
    d[0][j] = j;

  for (size_t i = 1; i <= m; ++i) {
    for (size_t j = 1; j <= n; ++j) {
      int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;

      d[i][j] =
          std::min({d[i - 1][j] + 1, d[i][j - 1] + 1, d[i - 1][j - 1] + cost});

      if (i > 1 && j > 1 && s1[i - 1] == s2[j - 2] && s1[i - 2] == s2[j - 1]) {
        d[i][j] = std::min(d[i][j], d[i - 2][j - 2] + cost);
      }
    }
  }

  return d[m][n];
}

double similarity(const std::string &a, const std::string &b) {
  int dist = damerauLevenshtein(toLower(a), toLower(b));
  int maxLen = std::max(a.size(), b.size());
  if (maxLen == 0)
    return 1.0;
  return 1.0 - double(dist) / maxLen;
}
