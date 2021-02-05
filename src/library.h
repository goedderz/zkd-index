#ifndef ZKD_TREE_LIBRARY_H
#define ZKD_TREE_LIBRARY_H

#include <string>
#include <vector>

using byte_string = std::basic_string<std::byte>;

auto interleave(std::vector<byte_string> vec) -> byte_string;
auto transpose(byte_string const& bs, std::size_t dimensions) -> std::vector<byte_string>;

#endif //ZKD_TREE_LIBRARY_H
