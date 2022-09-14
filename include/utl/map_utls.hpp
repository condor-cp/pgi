#pragma once
#include <string>
#include <map>
#include "string_utls.hpp"

namespace utl {

template <typename T, typename... Ts>
std::map<std::string, std::string> merge_maps(std::map<std::string, T> first_map, Ts... others)
{
    std::map<std::string, std::string> final_map;

    for (auto const& [key, val] : first_map)
        final_map[key] = utl::convert_to_string<T>(val);

    if constexpr (sizeof...(others) > 0)
    {
        std::map<std::string, std::string> rest_map = merge_maps(others...);
        final_map.insert(rest_map.begin(), rest_map.end());
    }
    return final_map;
}

template <typename T, typename... Ts>
std::map<std::string, std::vector<std::string>> merge_maps(std::map<std::string, std::vector<T>> first_map,
    Ts... others)
{
    std::map<std::string, std::vector<std::string>> final_map;

    for (auto const& [key, val] : first_map)
        final_map[key] = utl::convert_to_string<T>(val);

    if constexpr (sizeof...(others) > 0)
    {
        std::map<std::string, std::vector<std::string>> rest_map = merge_maps(others...);
        final_map.insert(rest_map.begin(), rest_map.end());
    }
    return final_map;
}

}  // namespace utl