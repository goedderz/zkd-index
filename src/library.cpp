#include "library.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <optional>


byte_string operator"" _bs(const char* const str, std::size_t len) {
  using namespace std::string_literals;

  std::string normalizedInput{};
  normalizedInput.reserve(len);
  for (char const* p = str; *p != '\0'; ++p) {
    switch (*p) {
      case '0':
      case '1':
        normalizedInput += *p;
        break;
      case ' ':
      case '\'':
        // skip whitespace and single quotes
        break;
      default:
        throw std::invalid_argument{"Unexpected character "s + *p + " in byte string: " + str};
    }
  }

  if (normalizedInput.empty()) {
    throw std::invalid_argument{"Empty byte string"};
  }

  auto result = byte_string{};

  char const* p = normalizedInput.c_str();
  for (auto bitIdx = 0; *p != '\0'; bitIdx = 0) {
    result += std::byte{0};
    for (; *p != '\0' && bitIdx < 8; ++bitIdx) {
      switch (*p) {
        case '0':
          break;
        case '1': {
          auto const bitPos = 7 - bitIdx;
          result.back() |= (std::byte{1} << bitPos);
          break;
        }
        default:
          throw std::invalid_argument{"Unexpected character "s + *p + " in byte string: " + str};
      }

      ++p;
      // skip whitespace and single quotes
      while (*p == ' ' || *p == '\'') {
        ++p;
      }
    }
  }

  return result;
}


byte_string operator"" _bss(const char* str, std::size_t len) {
  return byte_string{ reinterpret_cast<const std::byte*>(str), len};
}

BitReader::BitReader(BitReader::iterator begin, BitReader::iterator end)
  : _current(begin), _end(end) {}

auto BitReader::next() -> std::optional<Bit> {
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

ByteReader::ByteReader(iterator begin, iterator end)
  : _current(begin), _end(end) {}

auto ByteReader::next() -> std::optional<std::byte> {

  if (_current == _end) {
    return std::nullopt;
  }
  return *(_current++);
}

void BitWriter::append(Bit bit) {
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

auto BitWriter::str() && -> byte_string {
  if (_nibble > 0) {
    _buffer.push_back(_value);
  }
  _nibble = 0;
  _value = std::byte{0};
  return std::move(_buffer);
}

void BitWriter::reserve(std::size_t amount) {
  _buffer.reserve(amount);
}

RandomBitReader::RandomBitReader(const byte_string& ref) : ref(ref) {}

auto RandomBitReader::getBit(unsigned int index) -> Bit {
  auto byte = index / 8;
  auto nibble = index % 8;

  if (byte >= ref.size()) {
    return Bit::ZERO;
  }

  auto b = ref[byte] & (1_b << (7 - nibble));
  return b != 0_b ? Bit::ONE : Bit::ZERO;
}

RandomBitManipulator::RandomBitManipulator(byte_string& ref) : ref(ref) {}

auto RandomBitManipulator::getBit(unsigned int index) -> Bit {
  auto byte = index / 8;
  auto nibble = index % 8;

  if (byte >= ref.size()) {
    return Bit::ZERO;
  }

  auto b = ref[byte] & (1_b << (7 - nibble));
  return b != 0_b ? Bit::ONE : Bit::ZERO;
}

auto RandomBitManipulator::setBit(unsigned int index, Bit value) -> void {
  auto byte = index / 8;
  auto nibble = index % 8;

  if (byte >= ref.size()) {
    ref.resize(byte + 1);
  }
  auto bit = 1_b << (7 - nibble);
  ref[byte] = (ref[byte] & ~bit) | (value == Bit::ONE ? bit : 0_b);
}

auto interleave(std::vector<byte_string> const& vec) -> byte_string {
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

  for (size_t i = 0; i < 8 * max_size; i++) {
    for (auto& it : reader) {
      auto b = it.next();
      bitWriter.append(b.value_or(Bit::ZERO));
    }
  }

  return std::move(bitWriter).str();
}

auto interleave_bytes(std::vector<byte_string> const& vec) -> byte_string {
  std::size_t max_size = 0;
  std::vector<std::pair<byte_string::const_iterator, byte_string::const_iterator>> reader;
  reader.reserve(vec.size());

  for (auto& str : vec) {
    reader.emplace_back(std::make_pair(str.cbegin(), str.cend()));
    if (str.size() > max_size) {
      max_size = str.size();
    }
  }

  byte_string result;

  for (size_t i = 0; i < max_size; i++) {
    for (auto& [it, end] : reader) {
      if (it == end) {
        result.push_back(0_b);
      } else {
        result.push_back(*it);
        it++;
      }
    }
  }

  return result;
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
        goto fuckoff_cxx;
      }
      w.append(b.value());
    }
  }
fuckoff_cxx:

  std::vector<byte_string> result;
  std::transform(writer.begin(), writer.end(), std::back_inserter(result), [](auto& bs) {
    return std::move(bs).str();
  });
  return result;
}

auto transpose_bytes(byte_string const& bs, std::size_t dimensions) -> std::vector<byte_string> {
  assert(dimensions > 0);
  std::vector<byte_string> result;
  result.resize(dimensions);

  for (size_t i = 0; i < bs.size(); i++) {
    result[i % dimensions].push_back(bs[i]);
  }

  return result;
}

auto compareWithBox(byte_string const& cur, byte_string const& min, byte_string const& max, std::size_t dimensions)
  -> std::vector<CompareResult> {
  if (dimensions == 0) {
    auto msg = std::string{"dimensions argument to "};
    msg += __func__;
    msg += " must be greater than zero.";
    throw std::invalid_argument{msg};
  }
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

auto compareWithBoxBytes(byte_string const& cur, byte_string const& min, byte_string const& max, std::size_t dimensions)
-> std::vector<CompareResult> {
  assert(dimensions != 0);
  std::vector<CompareResult> result;
  result.resize(dimensions);

  std::size_t max_size = std::max(std::max(cur.size(), min.size()), max.size());

  ByteReader cur_reader(cur.cbegin(), cur.cend());
  ByteReader min_reader(min.cbegin(), min.cend());
  ByteReader max_reader(max.cbegin(), max.cend());

  auto isLargerThanMin = std::vector<bool>{};
  auto isLowerThanMax = std::vector<bool>{};
  isLargerThanMin.resize(dimensions);
  isLowerThanMax.resize(dimensions);

  for (std::size_t i = 0; i < 8 * max_size; i++) {
    unsigned step = i / dimensions;
    unsigned dim = i % dimensions;

    auto cur_byte = cur_reader.next().value_or(0_b);
    auto min_byte = min_reader.next().value_or(0_b);
    auto max_byte = max_reader.next().value_or(0_b);

    if (result[dim].flag != 0) {
      continue;
    }

    if (!isLargerThanMin[dim]) {
      if (cur_byte < min_byte) {
        result[dim].outStep = step;
        result[dim].flag = -1;
      } else if (cur_byte > min_byte) {
        isLargerThanMin[dim] = true;
        result[dim].saveMin = step;
      }
    }

    if (!isLowerThanMax[dim]) {
      if (cur_byte > max_byte) {
        result[dim].outStep = step;
        result[dim].flag = 1;
      } else if (cur_byte < max_byte) {
        isLowerThanMax[dim] = true;
        result[dim].saveMax = step;
      }
    }
  }

  return result;
}

auto testInBox(byte_string_view cur, byte_string const& min, byte_string const& max, std::size_t dimensions)
-> bool {
  auto cmp = compareWithBox(byte_string{cur}, min, max, dimensions);

  return std::all_of(cmp.begin(), cmp.end(), [](auto const& r) {
    return r.flag == 0;
  });
}

auto testInBoxBytes(byte_string_view cur, byte_string const& min, byte_string const& max, std::size_t dimensions)
-> bool {
  auto cmp = compareWithBoxBytes(byte_string{cur}, min, max, dimensions);

  return std::all_of(cmp.begin(), cmp.end(), [](auto const& r) {
    return r.flag == 0;
  });
}

auto getNextZValue(byte_string const& cur, byte_string const& min, byte_string const& max, std::vector<CompareResult>& cmpResult)
  -> std::optional<byte_string> {

  auto result = cur;

  auto const dims = cmpResult.size();

  auto minOutstepIter = std::min_element(cmpResult.begin(), cmpResult.end(), [&](auto const& a, auto const& b) {
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

  std::size_t changeBP = dims * minOutstepIter->outStep + d;

  if (minOutstepIter->flag > 0) {
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
  RandomBitManipulator rbm(result);
  assert(rbm.getBit(changeBP) == Bit::ZERO);
  rbm.setBit(changeBP, Bit::ONE);
  assert(rbm.getBit(changeBP) == Bit::ONE);

  auto min_trans = transpose(min, dims);
  auto next_v = transpose(result, dims);

  for (unsigned dim = 0; dim < dims; dim++) {
    auto& cmpRes = cmpResult[dim];
    if (cmpRes.flag >= 0) {
      auto bp = dims * cmpRes.saveMin + dim;
      if (changeBP >= bp) {
        // “set all bits of dim with bit positions > changeBP to 0”
        BitReader br(next_v[dim].begin(), next_v[dim].end());
        BitWriter bw;
        size_t i = 0;
        while (auto bit = br.next()) {
          if (i * dims + dim > changeBP) {
            break;
          }
          bw.append(bit.value());
          i++;
        }
        next_v[dim] = std::move(bw).str();
      } else {
        // “set all bits of dim with bit positions >  changeBP  to  the  minimum  of  the  query  box  in  this dim”
        BitReader br(next_v[dim].begin(), next_v[dim].end());
        BitWriter bw;
        size_t i = 0;
        while (auto bit = br.next()) {
          if (i * dims + dim > changeBP) {
            break;
          }
          bw.append(bit.value());
          i++;
        }
        BitReader br_min(min_trans[dim].begin(), min_trans[dim].end());
        for (auto j = 0; j < i; ++j) {
          br_min.next();
        }
        for (; auto bit = br_min.next(); ++i) {
          bw.append(bit.value());
        }
        next_v[dim] = std::move(bw).str();
      }
    } else {
      // load the minimum for that dimension
      next_v[dim] = min_trans[dim];
    }
  }

  return interleave(next_v);
}

auto getNextZValueBytes(byte_string const& cur, byte_string const& min, byte_string const& max, std::vector<CompareResult>& cmpResult)
  -> std::optional<byte_string> {

  auto result = cur;

  auto const dims = cmpResult.size();

  auto minOutstepIter = std::min_element(cmpResult.begin(), cmpResult.end(), [&](auto const& a, auto const& b) {
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

  std::size_t changeBP = dims * minOutstepIter->outStep + d;

  if (minOutstepIter->flag > 0) {
    while (changeBP != 0) {
      --changeBP;
      if (cur[changeBP] != 0xff_b) {
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
  result[changeBP] = std::byte(std::to_integer<uint8_t>(result[changeBP]) + 1);

  auto min_trans = transpose_bytes(min, dims);
  auto next_v = transpose_bytes(result, dims);

  for (unsigned dim = 0; dim < dims; dim++) {
    auto& cmpRes = cmpResult[dim];
    if (cmpRes.flag >= 0) {
      auto bp = dims * cmpRes.saveMin + dim;
      if (changeBP >= bp) {
        // “set all bits of dim with bit positions > changeBP to 0”
        ByteReader br(next_v[dim].begin(), next_v[dim].end());
        byte_string bw;
        size_t i = 0;
        while (auto bit = br.next()) {
          if (i * dims + dim > changeBP) {
            break;
          }
          bw.push_back(bit.value());
          i++;
        }
        next_v[dim] = std::move(bw);
      } else {
        // “set all bits of dim with bit positions >  changeBP  to  the  minimum  of  the  query  box  in  this dim”
        ByteReader br(next_v[dim].begin(), next_v[dim].end());
        byte_string bw;
        size_t i = 0;
        while (auto bit = br.next()) {
          if (i * dims + dim > changeBP) {
            break;
          }
          bw.push_back(bit.value());
          i++;
        }
        ByteReader br_min(min_trans[dim].begin(), min_trans[dim].end());
        for (auto j = 0; j < i; ++j) {
          br_min.next();
        }
        for (; auto bit = br_min.next(); ++i) {
          bw.push_back(bit.value());
        }
        next_v[dim] = std::move(bw);
      }
    } else {
      // load the minimum for that dimension
      next_v[dim] = min_trans[dim];
    }
  }

  return interleave_bytes(next_v);
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

template auto to_byte_string_fixed_length<uint64_t>(uint64_t) -> byte_string;
template auto to_byte_string_fixed_length<int64_t>(int64_t) -> byte_string;
template auto to_byte_string_fixed_length<long>(long) -> byte_string;
template auto to_byte_string_fixed_length<long long>(long long) -> byte_string;
template auto to_byte_string_fixed_length<unsigned long>(unsigned long) -> byte_string;
template auto to_byte_string_fixed_length<unsigned long long>(unsigned long long) -> byte_string;
template auto to_byte_string_fixed_length<uint32_t>(uint32_t) -> byte_string;
template auto to_byte_string_fixed_length<int32_t>(int32_t) -> byte_string;


template<typename T>
auto from_byte_string_fixed_length(byte_string const& bs) -> T {
  T result = 0;
  static_assert(std::is_integral_v<T>);
  if constexpr (std::is_unsigned_v<T>) {
    for (size_t i = 0; i < sizeof(T); i++) {
      result |= std::to_integer<uint64_t>(bs[i]) << (56 - 8 * i);
    }
  } else {
    abort();
  }

  return result;
}


template auto from_byte_string_fixed_length<uint64_t>(byte_string const&) -> uint64_t;

std::ostream& operator<<(std::ostream& ostream, byte_string const& string) {
  return operator<<(ostream, byte_string_view{string});
}

std::ostream& operator<<(std::ostream& ostream, byte_string_view const& string) {
  ostream << "[0x ";
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

std::ostream& operator<<(std::ostream& ostream, CompareResult const& cr) {
  ostream << "CR{";
  ostream << "flag=" << cr.flag;
  ostream << ", saveMin=" << cr.saveMin;
  ostream << ", saveMax=" << cr.saveMax;
  ostream << ", outStep=" << cr.outStep;
  ostream << "}";
  return ostream;
}
