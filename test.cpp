#include <cstdlib>
#include <iostream>
#include <string>

#include "src/library.h"
#include "src/rocksdb-handle.h"

#include <random>


void fillRocksdb(std::shared_ptr<RocksDBHandle> const& rocks) {
  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> distrib(0, 600000);

  for (std::size_t i = 0; i < 1000000; i++) {
    std::vector<byte_string> coords;

    for (std::size_t j = 0; j < 4; j++) {
      coords.push_back(to_byte_string_fixed_length<uint64_t>(distrib(gen)));
    }

    if (i % 10000 == 9999) {
      std::cout << "wrote 10000 entries" << std::endl;
    }

    auto key = interleave(coords);
    auto value = to_byte_string_fixed_length(i);
    rocks->db->Put({},
                   rocksdb::Slice(reinterpret_cast<const char *>(key.data())),
                   rocksdb::Slice(reinterpret_cast<const char *>(value.data())));
  }
}



using namespace std::string_view_literals;

int main(int argc, char *argv[]) {

  if (argc != 3) {
    std::cerr << "bad parameter, expecting" << argv[0] << " path " << "(fill|find)" << std::endl;
    return EXIT_FAILURE;
  }

  auto db = OpenRocksDB(argv[1]);



  if (argv[2] == "fill"sv) {
    fillRocksdb(db);
  } else if(argv[2] == "find"sv) {

  } else {
    std::cerr << "invalid verb: " << argv[2] << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
