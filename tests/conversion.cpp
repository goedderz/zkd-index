#include <gtest.h>

#include "library.h"

TEST(byte_string_conversion, uint64) {
  auto tests = {
    std::pair{12ull, byte_string{0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 12_b}},
    std::pair{0xAABBCCDDull, byte_string{0_b, 0_b, 0_b, 0_b, 0xAA_b, 0xBB_b, 0xCC_b, 0xDD_b}},
    std::pair{0x0123456789ABCDEFull, byte_string{0x01_b, 0x23_b, 0x45_b, 0x67_b, 0x89_b, 0xAB_b, 0xCD_b, 0xEF_b}}};

  for (auto &&[v, bs] : tests) {
    auto result = to_byte_string_fixed_length(v);
    EXPECT_EQ(result, bs);
  }
}

TEST(byte_string_conversion, uint64_compare) {
  auto tests = {
    std::pair{12ul, 7ul},
    std::pair{4567ul, 768735456ul},
    std::pair{4567ul, 4567ul},
  };

  for (auto &&[a, b] : tests) {
    auto a_bs = to_byte_string_fixed_length(a);
    auto b_bs = to_byte_string_fixed_length(b);

    EXPECT_EQ(a < b, a_bs < b_bs) << "byte string of " << a << " and " << b << " does not compare equally: " << a_bs << " " << b_bs;
  }
}

TEST(byte_string_conversion, int64) {
  auto tests = {
    std::pair{12ll, byte_string{0xff_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 12_b}},
    std::pair{0xAABBCCDDll, byte_string{0xFF_b, 0_b, 0_b, 0_b, 0_b, 0xAA_b, 0xBB_b, 0xCC_b, 0xDD_b}},
    std::pair{-0x0123456789ABCDEFll, byte_string{0x00_b, 0xFE_b, 0xDC_b, 0xBA_b, 0x98_b, 0x76_b, 0x54_b, 0x32_b, 0x11_b}}};

  for (auto &&[v, bs] : tests) {
    auto result = to_byte_string_fixed_length(v);
    EXPECT_EQ(result, bs);
  }
}

TEST(byte_string_conversion, int64_compare) {
  auto tests = {
    std::pair{12ll, 453ll},
    std::pair{-12ll, 453ll},
    std::pair{-1458792ll, 453ll},
    std::pair{17819835131ll, -894564ll},
    std::pair{-12ll, -8ll},
    std::pair{-5646872ll, -5985646871ll},
    std::pair{-5985646871ll, -5985646871ll},
  };

  for (auto &&[a, b] : tests) {
    auto a_bs = to_byte_string_fixed_length(a);
    auto b_bs = to_byte_string_fixed_length(b);

    EXPECT_EQ(a < b, a_bs < b_bs) << "byte string of " << a << " and " << b << " does not compare equally: " << a_bs << " " << b_bs;
  }
}


TEST(byte_string_conversion, double_float) {
    auto tests = {
        std::pair{0.0, byte_string{0b10111111_b, 0b11110000_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b}}
    };
        
        for (auto &&[v, bs] : tests) {
            auto result = to_byte_string_fixed_length(v);
            EXPECT_EQ(result, bs);
        }
}

TEST(byte_string_conversion, double_float_cmp) {
    auto tests = {
        std::pair{1.0, 0.0},
        std::pair{-1.0, 1.0},
        std::pair{1.0, 1.2},
        std::pair{10.0, 1.2},
        std::pair{-10.0, 1.2},
        std::pair{1.0, 1.0},
    };
    
    for (auto &&[a, b] : tests) {
        auto a_bs = to_byte_string_fixed_length(a);
        auto b_bs = to_byte_string_fixed_length(b);
        
        EXPECT_EQ(a < b, a_bs < b_bs) << "byte string of " << a << " and " << b << " does not compare equally: " << a_bs << " " << b_bs;
    }
}
