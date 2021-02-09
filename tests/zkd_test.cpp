#include <array>
#include <utility>
#include <vector>

#include <gtest.h>

#include "library.h"
#include "rocksdb-handle.h"

std::byte operator"" _b(unsigned long long b) {
  return std::byte{(unsigned char) b};
}

// byte_string operator"" _bs(const char* p) {
//   // TODO
// }

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

TEST(transpose, d3_empty) {
  auto res = transpose({}, 3);
  ASSERT_EQ(res, (std::vector<byte_string>{{}, {}, {}}));
}

TEST(transpose, d3_multi) {
  auto const testees = std::array{
    std::pair{byte_string{0b00011100_b}, std::vector<byte_string>{{0b01000000_b}, {0b01000000_b}, {0b01000000_b}}},
    std::pair{byte_string{0b00011110_b}, std::vector<byte_string>{{0b01100000_b}, {0b01000000_b}, {0b01000000_b}}},
    std::pair{byte_string{0b10101010_b}, std::vector<byte_string>{{0b10100000_b}, {0b01000000_b}, {0b10000000_b}}},
  };
  for (auto const &it : testees) {
    auto const &[testee, expected] = it;
    auto res = transpose(testee, 3);
    ASSERT_EQ(res, expected);
  }
}

TEST(compareBox, d2_eq) {

  auto d1 = std::make_pair(5_b, 35_b);   // (00000101, 00100011)
  auto d2 = std::make_pair(77_b, 121_b); // (01001101, 01111001)

  auto min_v = interleave({byte_string{d1.first}, byte_string{d2.first}});   // 0001000001110011
  auto max_v = interleave({byte_string{d1.second}, byte_string{d2.second}}); // 0001110101001011

  //ASSERT_TRUE(strcmp(min_v.data(), max_v.data()) < 0);

  auto v = interleave({byte_string{15_b}, byte_string{86_b}}); // (00001111, 01010110) // 0001000110111110
  auto res = compareWithBox(v, min_v, max_v, 2);

  // 0001000001110011 -- min (5, 77)
  // 0001000110111110 -- cur (15, 86)
  // 0001110101001011 -- max (35, 121)

  EXPECT_EQ(res[0].flag, 0);
  EXPECT_EQ(res[1].flag, 0);
  // TODO
}

TEST(compareBox, d2_eq2) {

  auto d1 = std::make_pair(2_b, 6_b); // (00000010, 00000110)
  auto d2 = std::make_pair(3_b, 5_b); // (00000011, 00000101)

  auto min_v = interleave({byte_string{d1.first}, byte_string{d2.first}});   // 00 00 00 00 00 00 11 01
  auto max_v = interleave({byte_string{d1.second}, byte_string{d2.second}}); // 00 00 00 00 00 11 10 01

  // ASSERT_TRUE(strcmp(min_v.data(), max_v.data()) < 0);

  auto v = interleave({byte_string{3_b}, byte_string{3_b}}); // (00000011, 00000011) -- 00 00 00 00 00 00 11 11
  auto res = compareWithBox(v, min_v, max_v, 2);

  EXPECT_EQ(res[0].flag, 0);
  EXPECT_EQ(res[0].saveMin, 7);
  EXPECT_EQ(res[0].saveMax, 5);

  EXPECT_EQ(res[1].flag, 0);
  // TODO
}

TEST(compareBox, d2_less) {
  auto d1 = std::make_pair(5_b, 35_b);
  auto d2 = std::make_pair(77_b, 121_b);
  auto p = std::make_pair(3_b, 86_b);

  auto min_v = interleave({byte_string{d1.first}, byte_string{d2.first}});
  auto max_v = interleave({byte_string{d1.second}, byte_string{d2.second}});

  //ASSERT_TRUE(strcmp(min_v.data(), max_v.data()) < 0);

  auto v = interleave({byte_string{p.first}, byte_string{p.second}});
  auto res = compareWithBox(v, min_v, max_v, 2);

  EXPECT_EQ(res[0].flag, -1);
  EXPECT_EQ(res[1].flag, 0);
  // TODO
}

TEST(compareBox, d2_x_less_y_greater) {
  auto d1 = std::make_pair(0b100_b, 0b1000_b);
  auto d2 = std::make_pair(0b10_b, 0b110_b);
  auto p = std::make_pair(0b11_b, 0b10000_b);

  auto min_v = interleave({byte_string{d1.first}, byte_string{d2.first}});    // 00 00 00 00 00 10 01 00
  auto max_v = interleave({byte_string{d1.second}, byte_string{d2.second}});  // 00 00 00 00 10 01 01 00
  auto v = interleave({byte_string{p.first}, byte_string{p.second}});         // 00 00 00 01 00 00 10 10

  auto res = compareWithBox(v, min_v, max_v, 2);

  EXPECT_EQ(res[0].flag, -1);
  EXPECT_EQ(res[0].saveMin, -1);
  EXPECT_EQ(res[0].saveMax, 4);
  EXPECT_EQ(res[0].outStep, 5);
  EXPECT_EQ(res[1].flag, 1);
  EXPECT_EQ(res[1].saveMin, 3);
  EXPECT_EQ(res[1].saveMax, -1);
  EXPECT_EQ(res[1].outStep, 3);
}

TEST(compareBox, d3_x_less_y_greater_z_eq) {
  auto d1 = std::make_pair<byte_string, byte_string>({0b100_b}, {0b1000_b});
  auto d2 = std::make_pair<byte_string, byte_string>({0b10_b}, {0b110_b});
  auto d3 = std::make_pair<byte_string, byte_string>({0b0_b}, {0b10_b});
  auto p = std::array{byte_string{0b11_b}, byte_string{0b10000_b}, byte_string{0b10_b}};

  auto min_v = interleave({d1.first, d2.first, d3.first});    // 00 00 00 00 00 10 01 00
  auto max_v = interleave({d1.second, d2.second, d3.second});  // 00 00 00 00 10 01 01 00
  auto v = interleave({p[0], p[1], p[2]});         // 00 00 00 01 00 00 10 10

  auto res = compareWithBox(v, min_v, max_v, 3);

  EXPECT_EQ(res[0].flag, -1);
  EXPECT_EQ(res[0].saveMin, -1);
  EXPECT_EQ(res[0].saveMax, 4);
  EXPECT_EQ(res[0].outStep, 5);
  EXPECT_EQ(res[1].flag, 1);
  EXPECT_EQ(res[1].saveMin, 3);
  EXPECT_EQ(res[1].saveMax, -1);
  EXPECT_EQ(res[1].outStep, 3);
  EXPECT_EQ(res[2].flag, 0);
  EXPECT_EQ(res[2].saveMin, 3);
  EXPECT_EQ(res[2].saveMax, -1);
  EXPECT_EQ(res[2].outStep, -1);
}


#if 0
// class RocksdbTest : public testing::Test {
//  public:
//
//   // static std::shared_ptr<RocksDBHandle> db;
//
//  protected:
//   static void SetUpTestCase() {
//     // db = OpenRocksDB("tmp");
//     //
//     // db->db->Put(rocksdb::WriteOptions(), "foobar", "baz");
//     //
//     // auto value = std::string{};
//     // auto res = db->db->Get(rocksdb::ReadOptions(), "foobar", &value);
//   }
//
//   auto cmp = rocksdb::BytewiseComparator();
//   void SetUp() {
//   }
// };
#endif

TEST(rocksdb, convert_bytestring) {
  auto const data = {
    byte_string{0b00011100_b},
    byte_string{0b11111111_b, 0b01010101_b},
  };

  for (auto const &it : data) {
    auto const slice = rocksdb::Slice(reinterpret_cast<char const *>(it.c_str()), it.size());
    auto const string = byte_string{reinterpret_cast<std::byte const *>(slice.data()), slice.size()};
    EXPECT_EQ(it, string);
    EXPECT_EQ(it.size(), slice.size());
    EXPECT_EQ(it.size(), string.size());
    EXPECT_EQ(0, memcmp(it.data(), slice.data(), it.size()));
  }
}

auto sliceFromString(byte_string const& str) -> rocksdb::Slice {
  return rocksdb::Slice(reinterpret_cast<char const *>(str.c_str()), str.size());
}

auto viewFromSlice(rocksdb::Slice slice) -> byte_string_view {
  return byte_string_view{reinterpret_cast<std::byte const *>(slice.data()), slice.size()};
}

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

TEST(rocksdb, cmp_slice) {
  enum class Cmp : int {
    LT = -1,
    EQ = 0,
    GT = 1
  };
  auto const data = {
    std::pair{Cmp::EQ, std::pair{byte_string{0x42_b}, byte_string{0x42_b}}},
    std::pair{Cmp::EQ, std::pair{byte_string{0x01_b, 0x02_b}, byte_string{0x01_b, 0x02_b}}},
    std::pair{Cmp::LT, std::pair{byte_string{0x01_b, 0x01_b}, byte_string{0x01_b, 0x02_b}}},
    std::pair{Cmp::GT, std::pair{byte_string{0x80_b}, byte_string{0x7f_b, 0xff_b}}},
    // TODO more tests
  };

  auto const* const cmp = rocksdb::BytewiseComparator();

  for (auto const &it : data) {
    auto const& [expected, testee] = it;
    auto const& [left, right] = testee;
    EXPECT_EQ(expected == Cmp::LT, cmp->Compare(sliceFromString(left), sliceFromString(right)) < 0)
      << "left = " << left << ", right = " << right;
    EXPECT_EQ(expected == Cmp::EQ, cmp->Compare(sliceFromString(left), sliceFromString(right)) == 0)
            << "left = " << left << ", right = " << right;
    EXPECT_EQ(expected == Cmp::EQ, cmp->Equal(sliceFromString(left), sliceFromString(right)))
            << "left = " << left << ", right = " << right;
    EXPECT_EQ(expected == Cmp::GT, cmp->Compare(sliceFromString(left), sliceFromString(right)) > 0)
            << "left = " << left << ", right = " << right;
  }
}

