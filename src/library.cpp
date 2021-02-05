#include "library.h"

#include <iostream>
#include <algorithm>
#include <optional>
#include <assert.h>

enum class Bit {
  ZERO = 0,
  ONE = 1
};

class BitReader {
 public:
  using iterator = typename byte_string::const_iterator;

  explicit BitReader(iterator begin, iterator end)
      : _current(begin), _end(end) {}

  auto next() -> std::optional<Bit> {
    if (_nibble >= 8) {
      if (_current == _end) {
        return std::nullopt;
      }
      _value = *_current;
      _nibble = 0;
      ++_current;
    }

    auto flag = std::byte{1u} << (7 - _nibble);
    auto bit = (_value & flag) != std::byte{0} ? Bit::ONE : Bit::ZERO;
    _nibble += 1;
    return bit;
  }

 private:
  iterator _current;
  iterator _end;
  std::byte _value{};
  std::size_t _nibble = 8;
};

class BitWriter {
 public:
  void append(Bit bit) {
    if (bit == Bit::ONE) {
      _value |= std::byte{1} << (7 - _nibble);
    }
    _nibble += 1;
    if (_nibble == 8) {
      _buffer.push_back(_value);
      _value = std::byte{0};
      _nibble = 0;
    }
  }

  auto str() && -> byte_string {
    if (_nibble > 0) {
      _buffer.push_back(_value);
    }
    _nibble = 0;
    _value = std::byte{0};
    return std::move(_buffer);
  }

  void reserve(std::size_t amount) {
    _buffer.reserve(amount);
  }

 private:
  std::size_t _nibble = 0;
  std::byte _value = std::byte{0};
  byte_string _buffer;
};


auto interleave(std::vector<byte_string> vec) -> byte_string {
  std::size_t max_size = 0;
  std::vector<BitReader> reader;
  reader.reserve(vec.size());

  for (auto& str : vec) {
    if (str.size() > max_size) {
      max_size = str.size();
    }
    reader.emplace_back(str.cbegin(), str.cend());
  }

  BitWriter bitWriter;
  bitWriter.reserve(vec.size() * max_size);

  for(size_t i = 0; i < 8*max_size; i++) {
    for (auto& it : reader) {
      auto b = it.next();
      bitWriter.append(b.value_or(Bit::ZERO));
    }
  }

  return std::move(bitWriter).str();
}

auto transpose(byte_string const& bs, std::size_t dimensions) -> std::vector<byte_string> {
  assert(dimensions > 0);
  BitReader reader(bs.cbegin(), bs.cend());
  std::vector<BitWriter> writer;
  writer.resize(dimensions);

  while (true) {
    for (auto& w : writer) {
      auto b = reader.next();
      if (!b.has_value()) {
        break;
      }
      w.append(b.value());
    }
  }

  std::vector<byte_string> result;
  std::transform(writer.begin(), writer.end(), std::back_inserter(result), [](auto & bs) {
    return std::move(bs).str();
  });
  return result;
}
