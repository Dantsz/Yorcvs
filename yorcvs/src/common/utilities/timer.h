#pragma once
#include <chrono>
namespace yorcvs {
/**
 * @brief Class for measuring time intervals
 *
 */
class Timer {
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
        if (mStopped) {
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
}
