#pragma once

#include <atomic>
#include <chrono>
#include <limits>
#include <utility>

namespace xsdk {

/**
 * @brief The minimum value representable as an int64_t, used to initialize an absent timestamp.
 */
static constexpr int64_t kAbsentRT = std::numeric_limits<int64_t>::min();

/**
 * @brief A templated class XTimed that extends TValue and stores a timestamp using TClock.
 * @template TValue The data type of the value stored in XTimed.
 * @template TClock The clock type used to get the current timestamp.
 */
template <class TValue, class TClock>
class XTimed: public TValue {
    int64_t timestamp_ = TClock::Timestamp();

public:
    /**
     * @brief A type alias for the TValue stored in XTimed.
     */
    using timed_value = TValue;

    using TValue::TValue;
    using TValue::operator=;

    ///@name Constructors
    ///@{
    /**
     * @brief Default constructor initializing the timestamp with the kAbsentRT value.
     */
    XTimed() : timestamp_(kAbsentRT) {}
    /// @brief Move constructor
    XTimed(XTimed&&) noexcept = default;
    /// @brief Copy constructor
    XTimed(const XTimed&)     = default;

    /**
     * @brief Move constructor initializing the XTimed with a given TValue and current timestamp.
     * @param _val The value to be stored in XTimed.
     */
    XTimed(TValue&& _val) noexcept : TValue(std::move(_val)), timestamp_(TClock::Timestamp()) {}
    /**
     * @brief Copy constructor initializing the XTimed with a given TValue and current timestamp.
     * @param _val The value to be stored in XTimed.
     */
    XTimed(const TValue& _val) : TValue(_val), timestamp_(TClock::Timestamp()) {}

    /**
     * @brief Move constructor initializing the XTimed with a given TValue and timestamp.
     * @param _val The value to be stored in XTimed.
     * @param _timestamp The timestamp to be stored in XTimed.
     */
    XTimed(TValue&& _val, int64_t _timespamp) : TValue(std::move(_val)), timestamp_(_timespamp) {}
    /**
     * @brief Copy constructor initializing the XTimed with a given TValue and timestamp.
     * @param _val The value to be stored in XTimed.
     * @param _timestamp The timestamp to be stored in XTimed.
     */
    XTimed(const TValue& _val, int64_t _timespamp) : TValue(_val), timestamp_(_timespamp) {}
    ///@}

    XTimed& operator=(const XTimed& _val)     = default;
    XTimed& operator=(XTimed&& _val) noexcept = default;

  
    ///@name TClock helper methods
    ///@{
    /**
     * @brief Static helper function returning the current timestamp in ticks.
     * @return The current timestamp in ticks.
     */
    static int64_t ClockTimestamp() { return TClock::Timestamp(); }
    /**
     * @brief Static helper function returning the number of ticks per second.
     * @return The number of ticks per second.
     */
    static int64_t TicksPerSecond() { return TClock::TicksPerSecond(); }
    /**
     * @brief Static helper function converting milliseconds to ticks.
     * @param _msec The time in milliseconds.
     * @return The equivalent time in ticks.
     */
    static int64_t MsecToTicks(double _msec)
    {
        return static_cast<int64_t>(_msec * TClock::TicksPerSecond() / 1'000);
    }
    /**
     * @brief Static helper function converting ticks to milliseconds.
     * @param _timestamp The time in ticks.
     * @return The equivalent time in milliseconds.
     */
    static double TicksToMsec(int64_t _timestamp)
    {
        return static_cast<double>(_timestamp) / static_cast<double>(TClock::TicksPerSecond()) * 1000.0;
    }
    ///@}

public:
    /**
     * @brief Getter for the timestamp stored in XTimed.
     * @return The timestamp stored in XTimed.
     */
    int64_t Timestamp() const { return timestamp_; }
    /**
     * @brief Checks if the timestamp is absent (initialization value).
     * @return true if the timestamp is absent, false otherwise.
     */
    bool    TimeIsAbsent() const { return timestamp_ == kAbsentRT; }
    /**
     * @brief Calculates the elapsed time in ticks between the current timestamp and the stored one.
     * @return The elapsed time in ticks.
     */
    int64_t TimeElapsed() const { return TimeIsAbsent() ? 0 : TClock::Timestamp() - timestamp_; }
    /**
     * @brief Resets the timestamp with a given elapsed time (in ticks).
     * @param _elapsed The elapsed time (in ticks) to be added to the current timestamp.
     * @return The new timestamp if the elapsed time is not absent, the current timestamp otherwise.
     */
    int64_t TimeReset(int64_t _elapsed = 0)
    {
        auto t = std::exchange(timestamp_, _elapsed == kAbsentRT ? kAbsentRT : (TClock::Timestamp() + _elapsed));
        return t == kAbsentRT ? t : TClock::Timestamp() - t;
    }

    /**
     * @brief Static constructor for creating an empty XTimed instance with an initial timestamp.
     * @return An empty XTimed instance with an initial timestamp.
     */
    static XTimed EmptyWithTime() { return XTimed(TValue()); }
};

} // namespace xsdk