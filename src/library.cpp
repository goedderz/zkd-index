#include "library.h"

#include <iostream>
#include <optional>

enum class Bit {
  ZERO = 0,
  ONE = 1
};

class BitStream {
 public:
  explicit BitStream(byte_string str)
      : _str(std::move(str)) {}

  auto next() -> std::optional<Bit> {
    if (_str.length() * 8 >= _nextIdx) {
      return std::nullopt;
    }

    auto byte = _str.at(_nextIdx / 8);

    auto bit = Bit{byte & (1 << (_nextIdx % 8))};

    ++_nextIdx;

    return bit;
  }

 private:
  byte_string _str;
  std::size_t _nextIdx = 0;
};

auto interleave(std::vector<byte_string> vec) -> byte_string {

  return byte_string();
}
