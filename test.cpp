#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>

#include "src/library.h"
#include "src/rocksdb-handle.h"

#include <random>


static auto sliceFromString(byte_string const& str) -> rocksdb::Slice {
  return rocksdb::Slice(reinterpret_cast<char const*>(str.c_str()), str.size());
}

void fillRocksdb(std::shared_ptr<RocksDBHandle> const& rocks) {
  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> distrib(0, 600);

  for (std::size_t i = 0; i < 1000000; i++) {
    std::vector<byte_string> coords;

    for (std::size_t j = 0; j < 4; j++) {
      coords.push_back(to_byte_string_fixed_length<uint64_t>(distrib(gen)));
    }


    auto key = interleave(coords);
    if (i % 10000 == 9999) {
      std::cout << "wrote 10000 entries" << std::endl;
      std::cout << key << std::endl;
    }

    auto value = to_byte_string_fixed_length(i);
    auto s = rocks->db->Put({},
                            sliceFromString(key),
                            sliceFromString(value));
    if (!s.ok()) {
      std::cerr << "insert failed: " << s.ToString() << std::endl;
      return;
    }
  }

  auto s = rocks->db->SyncWAL();
  if (!s.ok()) {
    std::cerr << "sync failed: " << s.ToString() << std::endl;
  }
}




static auto viewFromSlice(rocksdb::Slice slice) -> byte_string_view {
    return byte_string_view{reinterpret_cast<std::byte const*>(slice.data()), slice.size()};
}

void findAllInBox(std::shared_ptr<RocksDBHandle> const& rocks, std::vector<byte_string> const& min, std::vector<byte_string> const& max) {
    
    auto min_s = interleave(min);
    auto max_s = interleave(max);
    
    auto iter = std::unique_ptr<rocksdb::Iterator>{rocks->db->NewIterator(rocksdb::ReadOptions{})};

    byte_string cur = min_s;
    
    while (true) {
        std::cout << "Seeking to " << cur << std::endl;
        iter->Seek(sliceFromString(cur));
        auto s = iter->status();
        if (!s.ok()) {
            std::cerr << s.ToString() << std::endl;
            return;
        }
        if (!iter->Valid()) {
            break;
        }
        
        while (true) {
            auto key = viewFromSlice(iter->key());
            if (key < min_s || key > max_s) {
                cur = key;
                break;
            }
            
            auto value = transpose(byte_string{key}, 4);

            std::cout << value[0] << " " << value[1] << " " << value[2] << " " << value[3] << std::endl;
            iter->Next();
        }
        
        auto cmp = compareWithBox(cur, min_s, max_s, 4);
        
        auto next = getNextZValue(cur, min_s, max_s, cmp);
        if (!next) {
            std::cout << "No next value generated";
            break;
        }
        
        cur = next.value();
    }


}


using namespace std::string_view_literals;

int main(int argc, char *argv[]) {

  if (argc < 3) {
    std::cerr << "bad parameter, expecting" << argv[0] << " path " << "(fill|find)" << std::endl;
    return EXIT_FAILURE;
  }

  auto db = OpenRocksDB(argv[1]);

  std::string value;
  if (auto s = db->db->Get({}, "Hello", &value); s.IsNotFound()) {
    db->db->Put({}, "Hello", "World");
    std::cout << "set hello = world" << std::endl;
  } else {
    std::cout << "read hello = " << value << std::endl;
  }

  if (argv[2] == "fill"sv) {
    fillRocksdb(db);
  } else if(argv[2] == "find"sv) {
    if (argc != 5) {
      std::cerr << "missing min and max in from \"a b c d\" " << std::endl;
      return EXIT_FAILURE;
    }

    std::vector<byte_string> min, max;

    {
      std::stringstream ss(argv[3]);
      for (size_t i = 0; i < 4; i++) {
        uint64_t v;
        ss >> v;
        min.emplace_back(to_byte_string_fixed_length(v));
      }
    }
    {
      std::stringstream ss(argv[4]);
      for (size_t i = 0; i < 4; i++) {
        uint64_t v;
        ss >> v;
        max.emplace_back(to_byte_string_fixed_length(v));
      }
    }

    findAllInBox(db, min, max);
    
  } else {
    std::cerr << "invalid verb: " << argv[2] << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
