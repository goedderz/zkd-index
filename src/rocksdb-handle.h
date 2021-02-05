#ifndef ZKD_TREE_ROCKSDB_HANDLE_H
#define ZKD_TREE_ROCKSDB_HANDLE_H
#include <memory>
#include <rocksdb/db.h>

struct RocksDBHandle {
  RocksDBHandle(std::unique_ptr<rocksdb::DB> db,
    std::unique_ptr<rocksdb::ColumnFamilyHandle> def)
    : db(std::move(db)), default_(std::move(def)) {}

  std::unique_ptr<rocksdb::DB> db;
  std::unique_ptr<rocksdb::ColumnFamilyHandle> default_;
};

std::shared_ptr<RocksDBHandle> OpenRocksDB(std::string const& dbname);

#endif //ZKD_TREE_ROCKSDB_HANDLE_H
