#pragma once
#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <tuple>
#include <unordered_map>
namespace yorcvs
{
namespace MSGSEVERITY
{
constexpr size_t INFO = 0;
constexpr size_t WARNING = 1;
constexpr size_t ERROR = 2;
} // namespace MSGSEVERITY

/**
 * @brief Simple function to print a message
 *
 * @param message - message to log
 * @param level - level of severity
 */
template <typename T = std::string> void log(const T &message, size_t level = yorcvs::MSGSEVERITY::INFO) noexcept
{
    // TODO: std::format
    std::cout << "Severity  " << level << "  Message: " << message << '\n';
}

/**
 * @brief Class for measuring time intervals
 *
 */
class Timer
{
  public:
    using time_type = std::chrono::time_point<std::chrono::steady_clock>;

    /**
     * @brief Sarts the timer, the timer is stopped by deault
     *
     */
    void start()
    {
        mStarted = true;
        mStopped = false;
        stop_point = {};
        begin_point = std::chrono::steady_clock::now();
    }

    /**
     * @brief Stops the timer
     *
     */
    void stop()
    {
        // Stop the timer
        mStarted = false;
        // Unpause the timer
        mStopped = true;
        stop_point = std::chrono::steady_clock::now();
    }

    /**
     * @brief Checks if the timer has been stopped
     *
     * @return true the timer is paused
     * @return false it's not paused
     */
    [[nodiscard]] bool is_paused() const
    {
        return mStopped;
    }

    /**
     * @brief Checks if the timer is counting time
     *
     * @return true
     * @return false
     */
    [[nodiscard]] bool is_started() const
    {
        return mStarted;
    }

    /**
     * @brief Returns the time passed since timer was started
     *
     * @tparam return_type - what type should be returned
     * @tparam cast_format - what unit of measure should be used(nanosecond,milisecond,second) , by default it's
     * millisecond
     */
    template <typename return_type = size_t, typename cast_format = std::chrono::milliseconds>
    [[nodiscard]] return_type get_ticks() const
    {
        if (mStopped)
        {
            return static_cast<return_type>(std::chrono::duration_cast<cast_format>(stop_point - begin_point).count());
        }
        return static_cast<return_type>(
            std::chrono::duration_cast<cast_format>(std::chrono::steady_clock::now() - begin_point).count());
    }

    time_type begin_point = {};
    time_type stop_point = {};

    bool mStopped = false;
    bool mStarted = false;
};
/**
 * @brief Transform between coordinates on the spiral and their index
 * Example Spiral:
  20--21--22--23--24--25
   |                   |
  19   6---7---8---9  26
   |   |           |   |
  18   5   0---1  10  27
   |   |       |   |   |
  17   4---3---2  11  28
   |               |   |
  16--15--14--13--12  29
                       |
  35--34--33--32--31--30

 * Coordinates:
 (-1,1)---(0,1) ---  (1,1)
    |
    |
 (-1,0)    (0,0) --- (1,0)
    |                  |
    |                  |
 (-1,-1)---( 0,1 )--- (1,-1)
 *
 *
 *
 */
namespace spiral
{
/**
 * @brief Transform the index of an element in the spiral in its coordinates
 *
 * @param n the index
 * @return constexpr std::tuple<intmax_t,intmax_t> - first element is x and the second y
 */
inline std::tuple<intmax_t, intmax_t> wrap(const size_t n)
{
    // no constexppr array
    constexpr intmax_t c[4][6] = {
        {-1, 0, 0, -1, 1, 0}, {-1, 1, 1, 1, 0, 0}, {1, 0, 1, 1, -1, -1}, {1, -1, 0, -1, 0, -1}};

    const intmax_t square = static_cast<intmax_t>(std::floor(std::sqrt(n / 4)));

    const intmax_t index = (static_cast<intmax_t>(n) - 4 * square * square) % (2 * square + 1);
    const intmax_t side = (static_cast<intmax_t>(n) - 4 * square * square) / (2 * square + 1);

    return std::make_tuple<intmax_t, intmax_t>(c[side][0] * square + c[side][1] * index + c[side][2],
                                               c[side][3] * square + c[side][4] * index + c[side][5]);
}
/**
 * @brief Transform the coordinates of the element in its index in the spiral
 *
 * @param x x coordinate
 * @param y y coordinate
 * @return constexpr size_t the index in the spiral
 */
inline size_t unwrap(const intmax_t x, const intmax_t y)
{
    const intmax_t ax = std::abs(x);
    const intmax_t ay = std::abs(y);
    // find loop number in spiral
    const intmax_t loop = std::max(ax, ay);
    // one less than the edge length of the current loop
    const intmax_t edgelen = 2 * loop;
    // the numbers in the inner loops
    const intmax_t prev = (edgelen - 1) * (edgelen - 1);
    if (x == loop && y < loop)
    {
        // right edge
        return prev - y + (loop - 1);
    }
    if (y == -loop)
    {
        // top edge
        return prev + loop - x + edgelen - 1;
    }
    if (x == -1 * loop)
    {
        // left edge
        return prev + loop + y + 2 * edgelen - 1;
    }
    if (y == loop)
    {
        // bottom edge
        return prev + x + loop + 3 * edgelen - 1;
    }
    return 0;
}
} // namespace spiral
/*
* @brief reverses the key with the value in a unordered_map
*/
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