#include <cstdlib>
#include <iostream>

#include "rocksdb-handle.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "bad parameter, expecting a single path" << std::endl;
    return EXIT_FAILURE;
  }

  auto db = OpenRocksDB(argv[1]);

}
