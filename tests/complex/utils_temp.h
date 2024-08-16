#pragma once

#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>

#ifndef SIZEOF_ARRAY
    #define SIZEOF_ARRAY(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif // SIZEOF_ARRAY

namespace xutils_temp {

// ~Uniform [0...1.0]
inline double rand_d() { return (double)rand() / RAND_MAX; }

// If _dblMaxVal negative -> [-_nMaxValEnd, +_nMaxValEnd]
// If _dblMaxVal positive -> [0, +_nMaxValEnd]
inline int32_t rand_int(double _dblMaxVal)
{
    auto unMaxVal = (uint32_t)(std::abs(_dblMaxVal) + 0.5);
    if (_dblMaxVal < 0)
        return (int32_t)(rand_d() * unMaxVal * 2.0 - unMaxVal + 0.5);

    return (int32_t)(rand_d() * unMaxVal + 0.5);
}

inline int32_t rand_int(int32_t _nMinVal, int32_t _nMaxVal)
{
    // assert(_nMaxVal >= _nMinVal);
    _nMaxVal = std::max(_nMinVal, _nMaxVal);
    if (_nMaxVal == _nMinVal)
        return _nMinVal;

    return _nMinVal + (uint32_t)((_nMaxVal - _nMinVal) * rand_d() + 0.5);
}

inline bool rand_bool(double _dblProbability)
{
    return _dblProbability <= 0.0 ? false : 
           _dblProbability >= 1.0 ? true : // NOLINT(readability-avoid-nested-conditional-operator)
                                    rand_d() < _dblProbability;
}

inline std::string str_format(const char* _pszFormat, ...)
{
    va_list vl;
    va_start(vl, _pszFormat);
    auto nBufferSize = vsnprintf(nullptr, 0, _pszFormat, vl) + 1;
    va_end(vl);

    va_start(vl, _pszFormat);
    std::vector<char> buffer((size_t)nBufferSize + 1); // Add position for terminating null char
    vsnprintf(buffer.data(), buffer.size(), _pszFormat, vl);
    va_end(vl);

    return {buffer.data()};
}

} // namespace xutils_temp