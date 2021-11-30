#pragma once

#include <iostream>
#include <chrono>

#define time_point_t std::chrono::time_point<std::chrono::system_clock>

namespace utl {

std::string ISO_8601(time_point_t tp)
{
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());

    std::chrono::seconds s = std::chrono::duration_cast<std::chrono::seconds>(ms);
    std::time_t t = s.count();
    std::size_t fractional_seconds = ms.count() % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&t), "%Y-%m-%d %T") << "." << std::setw(3) << std::setfill('0')
       << fractional_seconds;
    return ss.str();
}

}  // namespace utl