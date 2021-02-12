#include "library.h"


#include <iostream>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <optional>
#include <cstddef>
#include <iomanip>

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

    auto flag = std::byte{1u} << (7u - _nibble);
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
      _value |= std::byte{1} << (7u - _nibble);
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


struct RandomBitReader {
  RandomBitReader(byte_string const& ref) : ref(ref) {}

  auto getBit(unsigned index) -> Bit {
    auto byte = index / 8;
    auto nibble = index % 8;

    if (byte >= ref.size()) {
      return Bit::ZERO;
    }

    auto b = ref[byte] & (1_b << (7 - nibble));
    return b != 0_b ? Bit::ONE : Bit::ZERO;
  }

 private:
  byte_string const& ref;
};

struct RandomBitManipulator {
  RandomBitManipulator(byte_string& ref) : ref(ref) {}

  auto getBit(unsigned index) -> Bit {
    auto byte = index / 8;
    auto nibble = index % 8;

    if (byte >= ref.size()) {
      return Bit::ZERO;
    }

    auto b = ref[byte] & (1_b << (7 - nibble));
    return b != 0_b ? Bit::ONE : Bit::ZERO;
  }

  auto setBit(unsigned index, Bit value) -> void {
    auto byte = index / 8;
    auto nibble = index % 8;

    if (byte >= ref.size()) {
      ref.resize(byte+1);
    }
    auto bit = 1_b << (7 - nibble);
    ref[byte] = (ref[byte] & ~bit) | (value == Bit::ONE ? bit : 0_b);
  }

 private:
  byte_string& ref;
};


auto interleave(std::vector<byte_string> const &vec) -> byte_string {
  std::size_t max_size = 0;
  std::vector<BitReader> reader;
  reader.reserve(vec.size());

  for (auto &str : vec) {
    if (str.size() > max_size) {
      max_size = str.size();
    }
    reader.emplace_back(str.cbegin(), str.cend());
  }

  BitWriter bitWriter;
  bitWriter.reserve(vec.size() * max_size);

  for (size_t i = 0; i < 8 * max_size; i++) {
    for (auto &it : reader) {
      auto b = it.next();
      bitWriter.append(b.value_or(Bit::ZERO));
    }
  }

  return std::move(bitWriter).str();
}

auto transpose(byte_string const &bs, std::size_t dimensions) -> std::vector<byte_string> {
  assert(dimensions > 0);
  BitReader reader(bs.cbegin(), bs.cend());
  std::vector<BitWriter> writer;
  writer.resize(dimensions);

  while (true) {
    for (auto &w : writer) {
      auto b = reader.next();
      if (!b.has_value()) {
        goto fuckoff_cxx;
      }
      w.append(b.value());
    }
  }
fuckoff_cxx:

  std::vector<byte_string> result;
  std::transform(writer.begin(), writer.end(), std::back_inserter(result), [](auto &bs) {
    return std::move(bs).str();
  });
  return result;
}

auto compareWithBox(byte_string const &cur, byte_string const &min, byte_string const &max, std::size_t dimensions)
  -> std::vector<CompareResult> {
  // TODO Don't crash with illegal dimensions
  assert(dimensions != 0);
  std::vector<CompareResult> result;
  result.resize(dimensions);

  std::size_t max_size = std::max(std::max(cur.size(), min.size()), max.size());

  BitReader cur_reader(cur.cbegin(), cur.cend());
  BitReader min_reader(min.cbegin(), min.cend());
  BitReader max_reader(max.cbegin(), max.cend());

  auto isLargerThanMin = std::vector<bool>{};
  auto isLowerThanMax = std::vector<bool>{};
  isLargerThanMin.resize(dimensions);
  isLowerThanMax.resize(dimensions);

  for (std::size_t i = 0; i < 8 * max_size; i++) {
    unsigned step = i / dimensions;
    unsigned dim = i % dimensions;

    auto cur_bit = cur_reader.next().value_or(Bit::ZERO);
    auto min_bit = min_reader.next().value_or(Bit::ZERO);
    auto max_bit = max_reader.next().value_or(Bit::ZERO);

    if (result[dim].flag != 0) {
      continue;
    }

    if (!isLargerThanMin[dim]) {
      if (cur_bit == Bit::ZERO && min_bit == Bit::ONE) {
        result[dim].outStep = step;
        result[dim].flag = -1;
      } else if (cur_bit == Bit::ONE && min_bit == Bit::ZERO) {
        isLargerThanMin[dim] = true;
        result[dim].saveMin = step;
      }
    }

    if (!isLowerThanMax[dim]) {
      if (cur_bit == Bit::ONE && max_bit == Bit::ZERO) {
        result[dim].outStep = step;
        result[dim].flag = 1;
      } else if (cur_bit == Bit::ZERO && max_bit == Bit::ONE) {
        isLowerThanMax[dim] = true;
        result[dim].saveMax = step;
      }
    }
  }

  return result;
}

auto getNextZValue(byte_string const &cur, byte_string const &min, byte_string const &max, std::vector<CompareResult> &cmpResult)
  -> std::optional<byte_string> {

  auto result = cur;

  auto const dims = cmpResult.size();

  auto minOutstepIter = std::min_element(cmpResult.begin(), cmpResult.end(), [&](auto const &a, auto const &b) {
    if (a.flag == 0) {
      return false;
    }
    if (b.flag == 0) {
      return true;
    }
    return a.outStep < b.outStep;
  });
  assert(minOutstepIter->flag != 0);
  auto const d = std::distance(cmpResult.begin(), minOutstepIter);

  RandomBitReader nisp(cur);

  std::size_t changeBP = dims * minOutstepIter->outStep + d;;
  if (minOutstepIter->flag > 0)  {
    while (changeBP != 0) {
      --changeBP;
      if (nisp.getBit(changeBP) == Bit::ZERO) {
        auto dim = changeBP % dims;
        auto step = changeBP / dims;
        if (cmpResult[dim].saveMax <= step) {
          cmpResult[dim].saveMin = step;
          cmpResult[dim].flag = 0;
          goto update_dims;
        }
      }

    }

    return std::nullopt;
  }

update_dims:
  // TODO implement
  assert(false);
  for (unsigned dim = 0; dim < dims; dim++) {
    auto& cmpRes = cmpResult[dim];
    if (cmpRes.flag >= 0) {
      auto bp = dims * cmpRes.saveMin + dim;
      if(changeBP > bp) {
        // “set all bits of dim with bit positions > changeBP to 0”
      }else {
        // “set all bits of dim with bit positions >  changeBP  to  the  minimum  of  the  query  box  in  this dim”
      }
    } else {
      // load the minimum for that dimension

    }
  }

  return result;
}

template<typename T>
auto to_byte_string_fixed_length(T v) -> byte_string {
  byte_string result;
  static_assert(std::is_integral_v<T>);
  if constexpr (std::is_unsigned_v<T>) {
    result.reserve(sizeof(T));
    for (size_t i = 0; i < sizeof(T); i++) {
      uint8_t b = 0xff & (v >> (56 - 8 * i));
      result.push_back(std::byte{b});
    }
  } else {
    // we have to add a <positive?> byte
    result.reserve(sizeof(T) + 1);
    if (v < 0) {
      result.push_back(0_b);
    } else {
      result.push_back(0xff_b);
    }
    for (size_t i = 0; i < sizeof(T); i++) {
      uint8_t b = 0xff & (v >> (56 - 8 * i));
      result.push_back(std::byte{b});
    }
  }
  return result;
}

template auto to_byte_string_fixed_length<uint64_t >(uint64_t) -> byte_string;
template auto to_byte_string_fixed_length<int64_t>(int64_t) -> byte_string;


std::ostream& operator<<(std::ostream& ostream, byte_string const& string) {
  ostream << "[";
  bool first = true;
  for (auto const& it : string) {
    if (!first) {
      ostream << " ";
    }
    first = false;
    ostream << std::hex << std::setfill('0') << std::setw(2) << std::to_integer<unsigned>(it);
  }
  ostream << "]";
  return ostream;
}
