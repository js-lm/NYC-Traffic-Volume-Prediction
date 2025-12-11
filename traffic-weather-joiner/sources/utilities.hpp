#pragma once

#include <cstdio>
#include <string>
#include <vector>

#include "units.hpp"

namespace utilities{

    // accpet 2006-01-01T00:00 or 2006-01-01 00:00
    inline units::Timestamp parseTimestamp(const std::string &timeString){
        units::Timestamp timestamp{};
        // YYYY-MM-DDTHH:MM
        if(std::sscanf(timeString.c_str(), "%d-%d-%dT%d:%d", &timestamp.year, &timestamp.month, &timestamp.day, &timestamp.hour, &timestamp.minute) == 5){
            return timestamp;
        }
        // YYYY-MM-DD HH:MM
        if(std::sscanf(timeString.c_str(), "%d-%d-%d %d:%d", &timestamp.year, &timestamp.month, &timestamp.day, &timestamp.hour, &timestamp.minute) == 5){
            return timestamp;
        }
        return timestamp;
    }

    // find column index by name, returns 0 if not found
    inline size_t findColumn(const std::vector<std::string> &header, const std::string &name){
        for(size_t i{0}; i < header.size(); i++){
            if(header[i] == name){
                return i;
            }
        }
        return 0;
    }

} // namespace utilities
