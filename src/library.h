#ifndef ZKD_TREE_LIBRARY_H
#define ZKD_TREE_LIBRARY_H
#include <cstddef>
#include <string>
#include <vector>
#include <limits>
#include <optional>

static std::byte operator"" _b(unsigned long long b) {
  return std::byte{(unsigned char) b};
}

using byte_string = std::basic_string<std::byte>;
using byte_string_view = std::basic_string_view<std::byte>;

std::ostream& operator<<(std::ostream& ostream, byte_string const& string);

auto interleave(std::vector<byte_string> const& vec) -> byte_string;
auto transpose(byte_string const& bs, std::size_t dimensions) -> std::vector<byte_string>;

struct CompareResult {
  signed flag = 0;
  unsigned outStep = std::numeric_limits<unsigned>::max();
  unsigned saveMin = std::numeric_limits<unsigned>::max();
  unsigned saveMax = std::numeric_limits<unsigned>::max();
};

auto compareWithBox(byte_string const& cur, byte_string const& min, byte_string const& max, std::size_t dimensions)
-> std::vector<CompareResult>;

auto getNextZValue(byte_string const& cur, byte_string const& min, byte_string const& max, std::vector<CompareResult> & cmpResult)
  -> std::optional<byte_string>;

template<typename T>
auto to_byte_string_fixed_length(T) -> byte_string;


#endif //ZKD_TREE_LIBRARY_H
