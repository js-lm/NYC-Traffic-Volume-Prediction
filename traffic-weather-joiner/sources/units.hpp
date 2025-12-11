#pragma once

#include <string>
#include <vector>
#include <ctime>

namespace units{

    struct Timestamp{
        int year;
        int month;
        int day;
        int hour;
        int minute;

        bool operator<(const Timestamp &other) const{
            if(year != other.year)      return year < other.year;
            if(month != other.month)    return month < other.month;
            if(day != other.day)        return day < other.day;
            if(hour != other.hour)      return hour < other.hour;
            return minute < other.minute;
        }

        // convert to YYYYMMDD
        int toPackedDate() const{ return year * 10000 + month * 100 + day;}

        int absoluteDifferenceInMinutes(const Timestamp &other) const{
            std::tm time1{
                .tm_sec     = 0,
                .tm_min     = minute,
                .tm_hour    = hour,
                .tm_mday    = day,
                .tm_mon     = month - 1,
                .tm_year    = year - 1900,
                .tm_isdst   = -1
            };

            std::tm time2{
                .tm_sec     = 0,
                .tm_min     = other.minute,
                .tm_hour    = other.hour,
                .tm_mday    = other.day,
                .tm_mon     = other.month - 1,
                .tm_year    = other.year - 1900,
                .tm_isdst   = -1
            };

            std::time_t timestamp1{std::mktime(&time1)};
            std::time_t timestamp2{std::mktime(&time2)};

            double differenceSeconds{std::difftime(timestamp1, timestamp2)};
            int differenceMinutes{static_cast<int>(differenceSeconds / 60.0)};

            return differenceMinutes < 0 ? -differenceMinutes : differenceMinutes;
        }
    };

    struct TimeRowData{
        Timestamp timestamp;
        std::vector<std::string> fullRow;
        
        bool operator<(const TimeRowData &other) const{
            return timestamp < other.timestamp;
        }
    };

} // namespace units