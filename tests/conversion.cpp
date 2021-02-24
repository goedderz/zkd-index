#include <gtest.h>

#include "library.h"

using namespace zkd;

TEST(byte_string_conversion, uint64) {
  auto tests = {
    std::pair{uint64_t{12}, byte_string{0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 12_b}},
    std::pair{uint64_t{0xAABBCCDD}, byte_string{0_b, 0_b, 0_b, 0_b, 0xAA_b, 0xBB_b, 0xCC_b, 0xDD_b}},
    std::pair{uint64_t{0x0123456789ABCDEF}, byte_string{0x01_b, 0x23_b, 0x45_b, 0x67_b, 0x89_b, 0xAB_b, 0xCD_b, 0xEF_b}}};

  for (auto &&[v, bs] : tests) {
    auto result = to_byte_string_fixed_length(v);
    EXPECT_EQ(result, bs);
  }
}

TEST(byte_string_conversion, uint64_compare) {
  auto tests = {
    std::pair{uint64_t{12}, uint64_t{7}},
    std::pair{uint64_t{4567}, uint64_t{768735456}},
    std::pair{uint64_t{4567}, uint64_t{4567}},
  };

  for (auto &&[a, b] : tests) {
    auto a_bs = to_byte_string_fixed_length(a);
    auto b_bs = to_byte_string_fixed_length(b);

    EXPECT_EQ(a < b, a_bs < b_bs) << "byte string of " << a << " and " << b << " does not compare equally: " << a_bs << " " << b_bs;
  }
}

TEST(byte_string_conversion, int64) {
  auto tests = {
    std::pair{int64_t{12}, byte_string{0xff_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 0_b, 12_b}},
    std::pair{int64_t{0xAABBCCDD}, byte_string{0xFF_b, 0_b, 0_b, 0_b, 0_b, 0xAA_b, 0xBB_b, 0xCC_b, 0xDD_b}},
    std::pair{int64_t{-0x0123456789ABCDEF}, byte_string{0x00_b, 0xFE_b, 0xDC_b, 0xBA_b, 0x98_b, 0x76_b, 0x54_b, 0x32_b, 0x11_b}}};

  for (auto &&[v, bs] : tests) {
    auto result = to_byte_string_fixed_length(v);
    EXPECT_EQ(result, bs);
  }
}

TEST(byte_string_conversion, int64_compare) {
  auto tests = {
    std::pair{int64_t{12}, int64_t{453}},
    std::pair{int64_t{-12}, int64_t{453}},
    std::pair{int64_t{-1458792}, int64_t{453}},
    std::pair{int64_t{17819835131}, int64_t{-894564}},
    std::pair{int64_t{-12}, int64_t{-8}},
    std::pair{int64_t{-5646872}, int64_t{-5985646871}},
    std::pair{int64_t{-5985646871}, int64_t{-5985646871}},
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
        std::pair{1000.0, 100000.0},
        std::pair{-1000.0, 100000.0},
        std::pair{.0001, .001},
    };
    
    for (auto &&[a, b] : tests) {
        auto a_bs = to_byte_string_fixed_length(a);
        auto b_bs = to_byte_string_fixed_length(b);
        
        EXPECT_EQ(a < b, a_bs < b_bs) << "byte string of " << a << " and " << b << " does not compare equally: " << a_bs << " " << b_bs;
    }
}
