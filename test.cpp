#include <cstdlib>
#include <iostream>

#include "src/library.h"
#include "src/rocksdb-handle.h"

#include <gtest.h>

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();

    /*


  if (argc != 2) {
    std::cerr << "bad parameter, expecting a single path" << std::endl;
    return EXIT_FAILURE;
  }

  auto db = OpenRocksDB(argv[1]);

  db->db->Put(rocksdb::WriteOptions(), "foobar", "baz");

  auto value = std::string{};
  auto res = db->db->Get(rocksdb::ReadOptions(), "foobar", &value);

  std::cout << value << std::endl;*/
}
