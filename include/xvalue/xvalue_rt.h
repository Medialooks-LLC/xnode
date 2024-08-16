#pragma once

//#include "xbase.h"
#include "xtimed.h"
#include "xvalue.h"

namespace xsdk {

namespace xnode {
    /**
     * @brief Monotonic increasing sequence generator template class.
     *
     * This template class Monotonic generates monotonically increasing sequence of values.
     * The template parameter T represents the data type of values in the sequence.
     * The second template parameter TIdx defaults to 0, it is used to provide a variant
     * of this template with different instance names.
     *
     * @tparam T Data type of values in the sequence.
     * @tparam TIdx Index of current template instance.
     */
    template <typename T, size_t TIdx = 0>
    class Monotonic {
    public:
        /**
         * @brief Increment the monotonic data value and ensure it remains monotonic.
         * @param _val The current data value.
         * @return The next monotonic data value.
         */
        static int64_t Next(int64_t _val)
        {
            static std::atomic_int64_t prev = std::numeric_limits<int64_t>::min();

            auto next_min = prev.fetch_add(1) + 1;
            auto val      = std::max(_val, next_min);
            while (!prev.compare_exchange_strong(next_min, val))
                val = std::max(next_min + 1, val);

            return val;
        }
    };
    /**
     * @brief UniqueClock template class generating unique nanosecond timestamps.
     *
     * This template class UniqueClock generates a clock that returns unique nanosecond timestamps.
     * The first template parameter ClockT specifies the type of high-resolution clock to use.
     * The second template parameter TicksPerSecond defines the number of ticks per second.
     *
     * @tparam ClockT Type of high-resolution clock to use.
     * @tparam TicksPerSecondT Number of ticks per second.
     */
    template <class ClockT = std::chrono::high_resolution_clock, size_t TicksPerSecondT = 10'000'000>
    class UniqueClock {
    public:
        /**
         * @brief Generates the current unique timestamp in nanoseconds.
         * @return The current unique timestamp in nanoseconds.
         */
        static int64_t Timestamp()
        {
            static_assert(TicksPerSecondT > 0);
            static constexpr int64_t nsec_per_tick = 1'000'000'000 / TicksPerSecondT;
            auto t = std::chrono::time_point_cast<std::chrono::nanoseconds>(ClockT::now()).time_since_epoch().count() /
                     nsec_per_tick;
            return Monotonic<UniqueClock<ClockT, TicksPerSecondT>>::Next(t);
        }

        /**
         * @brief The number of ticks per second in the clock.
         */
        constexpr static uint32_t TicksPerSecond() { return TicksPerSecondT; }
    };

} // namespace xnode

/**
 * @brief XValueRT is a wrapper of XValue class using UniqueClock to generate unique monotonic timestamps.
 * @tparam XValue The data type to store.
 * @tparam XTimed A clock type that generates unique monotonic timestamps.
 */
using XValueRT = XTimed<XValue, xnode::UniqueClock<>>;

} // namespace xsdk