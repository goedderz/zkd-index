#ifndef ZKD_TREE_LIBRARY_H
#define ZKD_TREE_LIBRARY_H

#include <cstddef>
#include <limits>
#include <optional>
#include <string>
#include <vector>

static std::byte operator"" _b(unsigned long long b) {
  return std::byte{(unsigned char) b};
}

using byte_string = std::basic_string<std::byte>;
using byte_string_view = std::basic_string_view<std::byte>;

byte_string operator"" _bs(const char* str, std::size_t len);

std::ostream& operator<<(std::ostream& ostream, byte_string const& string);
std::ostream& operator<<(std::ostream& ostream, byte_string_view const& string);

auto interleave(std::vector<byte_string> const& vec) -> byte_string;
auto transpose(byte_string const& bs, std::size_t dimensions) -> std::vector<byte_string>;

struct CompareResult {
  signed flag = 0;
  unsigned outStep = std::numeric_limits<unsigned>::max();
  unsigned saveMin = std::numeric_limits<unsigned>::max();
  unsigned saveMax = std::numeric_limits<unsigned>::max();
};

std::ostream& operator<<(std::ostream& ostream, CompareResult const& string);

auto compareWithBox(byte_string const& cur, byte_string const& min, byte_string const& max, std::size_t dimensions)
  -> std::vector<CompareResult>;

auto getNextZValue(byte_string const& cur, byte_string const& min, byte_string const& max, std::vector<CompareResult>& cmpResult)
  -> std::optional<byte_string>;

template<typename T>
auto to_byte_string_fixed_length(T) -> byte_string;


enum class Bit {
  ZERO = 0,
  ONE = 1
};

class BitReader {
 public:
  using iterator = typename byte_string::const_iterator;

  explicit BitReader(iterator begin, iterator end);

  auto next() -> std::optional<Bit>;

 private:
  iterator _current;
  iterator _end;
  std::byte _value{};
  std::size_t _nibble = 8;
};

class BitWriter {
 public:
  void append(Bit bit);

  auto str() && -> byte_string;

  void reserve(std::size_t amount);

 private:
  std::size_t _nibble = 0;
  std::byte _value = std::byte{0};
  byte_string _buffer;
};


struct RandomBitReader {
  explicit RandomBitReader(byte_string const& ref);

  auto getBit(unsigned index) -> Bit;

 private:
  byte_string const& ref;
};

struct RandomBitManipulator {
  RandomBitManipulator(byte_string& ref);

  auto getBit(unsigned index) -> Bit;

  auto setBit(unsigned index, Bit value) -> void;

 private:
  byte_string& ref;
};

#endif //ZKD_TREE_LIBRARY_H
