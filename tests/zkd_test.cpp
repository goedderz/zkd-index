
#include <gtest.h>

#include "library.h"

TEST(foo, bar) {
  auto res = interleave({});
  ASSERT_EQ(res, byte_string{});
}
