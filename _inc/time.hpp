#pragma once

#include "common.h"
#include <ctime>

#define OFF_START_HOUR ( 0 )
#define OFF_END_HOUR ( 6 )

template <typename T>
bool in_bound(const T min, const T current, const T max)
{
    return ((min <= current) && (current <= max));
}

std::tuple<int, int, int> getTime()
{
    std::time_t now = std::time(nullptr);  // czas w sekundach od epoki
    std::tm *ltm = std::localtime(&now);   // struktura z lokalnym czasem

    return { ltm->tm_hour, ltm->tm_min, ltm->tm_sec };
}
