#pragma once
#include <tuple>
#include <unordered_map>
#include "utilities/log.h"
#include "utilities/timer.h"
#include "utilities/ulamspiral.h"
namespace yorcvs
{
template<typename K, typename V>
std::unordered_map<V, K> build_reverse_unordered_map(const std::unordered_map<K, V>& map)
{
    std::unordered_map<V, K> reversed{};
    for (const auto& [k, v] : map)
    {
        reversed.insert(std::make_pair(v, k));
    }
    return reversed;
}
} // namespace yorcvs
