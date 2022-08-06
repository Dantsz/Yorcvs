#pragma once
#include <iostream>
namespace yorcvs {
namespace MSGSEVERITY {
    [[maybe_unused]] constexpr size_t INFO = 0;
    [[maybe_unused]] constexpr size_t WARNING = 1;
    [[maybe_unused]] constexpr size_t ERROR = 2;
} // namespace MSGSEVERITY

/**
 * @brief Simple function to print a message
 *
 * @param message - message to log
 * @param level - level of severity
 */
template <typename T = std::string>
void log(const T& message, size_t level = yorcvs::MSGSEVERITY::INFO) noexcept
{
    // TODO: std::format
    std::cout << "Severity  " << level << "  Message: " << message << '\n';
}
}
