#ifndef ZKD_TREE_LIBRARY_H
#define ZKD_TREE_LIBRARY_H

#include <string>
#include <vector>

using byte_string = std::basic_string<std::byte>;
using byte_string_view = std::basic_string_view<std::byte>;

auto interleave(std::vector<byte_string> const& vec) -> byte_string;
auto transpose(byte_string const& bs, std::size_t dimensions) -> std::vector<byte_string>;

struct CompareResult {
  signed flag = 0;
  bool isLargerThanMin = false;
  bool isLowerThanMax = false;
  unsigned outStep = 0;
  unsigned saveMin = unsigned(-1);
  unsigned saveMax = unsigned(-1);
};

auto compareWithBox(byte_string const& cur, byte_string const& min, byte_string const& max, std::size_t dimensions)
-> std::vector<CompareResult>;

#endif //ZKD_TREE_LIBRARY_H
