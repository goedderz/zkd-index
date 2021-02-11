#include <cstdlib>
#include <iostream>
#include <string>

#include "src/library.h"
#include "src/rocksdb-handle.h"



using namespace std::string_view_literals;

int main(int argc, char *argv[]) {

  if (argc != 3) {
    std::cerr << "bad parameter, expecting" << argv[0] << " path " << "(fill|find)" << std::endl;
    return EXIT_FAILURE;
  }

  auto db = OpenRocksDB(argv[1]);



  if (argv[2] == "fill"sv) {

  } else if(argv[2] == "find"sv) {

  } else {
    std::cerr << "invalid verb: " << argv[2] << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
