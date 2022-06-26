#pragma once
#include "utilities/log.h"
#include "utilities/timer.h"
#include "utilities/ulamspiral.h"
#include <tuple>
#include <unordered_map>

namespace std {
template <>
struct hash<std::tuple<intmax_t, intmax_t>> {
    size_t operator()(const std::tuple<intmax_t, intmax_t>& p) const
    {
        intmax_t x = std::get<0>(p);
        intmax_t y = std::get<1>(p);
        return yorcvs::spiral::unwrap(x, y);
    }
};
} // namespace std
namespace yorcvs {
template <typename K, typename V>
std::unordered_map<V, K> build_reverse_unordered_map(const std::unordered_map<K, V>& map)
{
    std::unordered_map<V, K> reversed {};
    for (const auto& [k, v] : map) {
        reversed.insert(std::make_pair(v, k));
    }
    return reversed;
}

} // namespace yorcvs
