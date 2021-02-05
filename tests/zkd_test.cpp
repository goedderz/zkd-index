#include <array>

#include <gtest.h>

#include "library.h"

std::byte operator"" _b(unsigned long long b) {
  return std::byte{(unsigned char) b};
}

TEST(interleave, d0) {
  auto res = interleave({});
  ASSERT_EQ(byte_string{}, res);
}

TEST(interleave, d1_empty) {
  auto res = interleave({{}});
  ASSERT_EQ(byte_string{}, res);
}

TEST(interleave, d1_multi) {
  auto const testees = std::array{
    byte_string{0x42_b},
    byte_string{{0x42_b, 0x42_b}},
    byte_string{{0x01_b, 0x02_b, 0x03_b}},
  };
  for (auto const &testee : testees) {
    auto const res = interleave({testee});
    EXPECT_EQ(testee, res);
  }
}

TEST(interleave, d2_empty) {
  auto res = interleave({{}, {}});
  ASSERT_EQ(byte_string{}, res);
}

TEST(interleave, d2_multi) {
  auto const testees = std::array{
    std::pair{byte_string{0b01010101_b, 0b10101010_b}, std::tuple{byte_string{0b00001111_b}, byte_string{0b11110000_b}}},
    std::pair{byte_string{0b01010101_b, 0b01010101_b, 0b00110011_b, 0b00110011_b}, std::tuple{byte_string{0b00000000_b, 0b01010101_b}, byte_string{0b11111111_b, 0b01010101_b}}},
    std::pair{byte_string{0b10101010_b, 0b10101010_b, 0b01010101_b, 0b01010101_b}, std::tuple{byte_string{0b11111111_b}, byte_string{0b00000000_b, 0b11111111_b}}},
    std::pair{byte_string{0b01010111_b, 0b01010111_b, 0b00010001_b, 0b00010001_b, 0b01000100_b, 0b01000100_b}, std::tuple{byte_string{0b00010001_b}, byte_string{0b11111111_b, 0b01010101_b, 0b10101010_b}}},
  };
  for (auto const &it : testees) {
    auto const &[expected, testee] = it;
    auto const res = interleave({std::get<0>(testee), std::get<1>(testee)});
    EXPECT_EQ(expected, res);
  }
}
