/**
 * @file log.h
 * @author Dansz
 * @brief Simple logging functions
 * @version 0.1
 * @date 2021-07-29
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once
#include <iostream>
#include <string>

namespace yorcvs
{


namespace MSGSEVERITY
{
    constexpr size_t INFO = 0;
    constexpr size_t WARNING = 1;
    constexpr size_t ERROR = 2;
}

/**
 * @brief Simple function to print a message
 *
 * @param message - message to log
 * @param level - level of severity
 */
template <typename T = std::string> void log(const T &message, size_t level = yorcvs::MSGSEVERITY::INFO) noexcept
{
    // TODO: std::format
    std::cout << "Severity  " << level << ": Message " << message << '\n';
}

} // namespace yorcvs
